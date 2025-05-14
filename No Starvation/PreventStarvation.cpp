// zapobieganie zaglodzeniu

#include "PreventStarvation.h"
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <random>
#include <iostream>
#include <cstring>

#define PHILOSOPHERS 5
#define NO_EATING_TIME 5

sem_t chopsticks[PHILOSOPHERS];
sem_t screen_lock;

bool running = true;
bool paused = false;
bool colors_enabled = false;

enum StatusColor {
    THINKING = 1,
    EATING,
    HUNGRY,
    STARVING,
    HOLDING,
    CHOPSTICK_FREE,
    CHOPSTICK_USED
};

// tablica okreslajaca aktualny stan danego filozofa
int philosopherState[PHILOSOPHERS];
// tablica przechowujaca czas kiedy filozof ostatnio jadl
time_t last_ate[PHILOSOPHERS];
sem_t general_lock;

char state[PHILOSOPHERS][4];
// tablica zjedzonych posilkow
int meals[PHILOSOPHERS];
// tablica dostepnych paleczek
bool chopstick_available[PHILOSOPHERS];

void draw_interface();
void pause_barrier();

int main() {
    pthread_t philosophers[PHILOSOPHERS];
    int ids[PHILOSOPHERS];

    initscr();
    cbreak();
    noecho();
    curs_set(FALSE);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

    if (has_colors()) {
        start_color();
        init_pair(THINKING, COLOR_CYAN, COLOR_BLACK);
        init_pair(EATING, COLOR_GREEN, COLOR_BLACK);
        init_pair(STARVING, COLOR_RED, COLOR_BLACK);
        init_pair(HOLDING, COLOR_YELLOW, COLOR_BLACK);
        init_pair(CHOPSTICK_FREE, COLOR_WHITE, COLOR_BLACK);
        init_pair(CHOPSTICK_USED, COLOR_MAGENTA, COLOR_BLACK);
        colors_enabled = true;
    } else {
        colors_enabled = false;
    }

    printw("Starvation Prevention Simulation\n\n");
    printw("Press any key to start Simulation (q or Q to exit)...\n\n");
    int choice = getch();
    if(choice == 'q' or choice == 'Q')
        running = false;


    sem_init(&screen_lock, 0, 1);
    sem_init(&general_lock, 0, 1);
    for (int i = 0; i < PHILOSOPHERS; ++i) {
        sem_init(&chopsticks[i], 0, 1);
        philosopherState[i] = THINKING;
        chopstick_available[i] = true;
        meals[i] = 0;
        strcpy(state[i], "T");
    }

    for (int i = 0; i < PHILOSOPHERS; ++i) {
        ids[i] = i;
        pthread_create(&philosophers[i], NULL, prevent_starvation, &ids[i]);
    }

    while (running) {
        draw_interface();

        int ch = getch();
        if (ch == 'q' || ch == 'Q') running = false;
        if (ch == ' ') paused = !paused;

        usleep(100000); // 0.1s
    }

    for (int i = 0; i < PHILOSOPHERS; ++i) {
        pthread_join(philosophers[i], NULL);
    }


    for (int i = 0; i < PHILOSOPHERS; ++i) {
        sem_destroy(&chopsticks[i]);
    }

    sem_destroy(&screen_lock);
    sem_destroy(&general_lock);

    endwin();
    return 0;
}

// Funkcja generujaca losowa liczbe calkowita z przedzialu [a, b]
int random_between(int a, int b) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(a, b);
    return dist(gen);
}

// Funkcja filozofa z zapobieganiem zaglodzeniu
void* prevent_starvation(void* arg) {
    int id = *(int*)arg;
    int left = (id + 1) % PHILOSOPHERS;
    int right = id;

    time_t now = -1;;
    last_ate[id] = -1;

    while (running) {
        pause_barrier();

        now = time(NULL);

        if (last_ate[id] > 0 && difftime(now, last_ate[id]) >= NO_EATING_TIME)
            strcpy(state[id], "S");
        else {
            // mysli
            strcpy(state[id], "T");
            sleep(random_between(2, 5));


            strcpy(state[id], "H");
        }

        // czekanie na lewa paleczke
        sem_wait(&chopsticks[left]);
        chopstick_available[left] = false;
        strcpy(state[id], "L.");

        pause_barrier();

        // czekanie na prawa paleczke
        sem_wait(&chopsticks[right]);
        chopstick_available[right] = false;
        strcpy(state[id], "LR");


        pause_barrier();

        // jedzenie
        strcpy(state[id], "E");
        meals[id]++;
        sleep(random_between(1, 4));
        last_ate[id] = time(NULL);

        // zwalnianie paleczek
        sem_post(&chopsticks[left]);
        chopstick_available[left] = true;
        strcpy(state[id], ".R");

        pause_barrier();

        sem_post(&chopsticks[right]);
        chopstick_available[right] = true;
        strcpy(state[id], "..");

        pause_barrier();
    }

    return NULL;
}


void draw_interface() {
    sem_wait(&screen_lock);
    erase();

    mvprintw(0, 0, "Dining Philosophers Simulation");
    mvprintw(1, 0, "Press SPACE to pause/resume, Q to quit.\n");

    for (int i = 0; i < PHILOSOPHERS; i++) {
        mvprintw(3, i * 18, "Philosopher %d", i);
        mvprintw(4, i * 18, "Meals: %d", meals[i]);

        if (strcmp(state[i], "E") == 0) attron(COLOR_PAIR(EATING));
        else if (strcmp(state[i], "T") == 0) attron(COLOR_PAIR(THINKING));
        else if (strcmp(state[i], "S") == 0) attron(COLOR_PAIR(STARVING));
        else if (strcmp(state[i], "LR") == 0) attron(COLOR_PAIR(HOLDING));
        else attron(COLOR_PAIR(HOLDING));

        mvprintw(5, i * 18, "State: %s", state[i]);
        attroff(COLOR_PAIR(EATING));
        attroff(COLOR_PAIR(THINKING));
        attroff(COLOR_PAIR(STARVING));
        attroff(COLOR_PAIR(HOLDING));
    }

    mvprintw(8, 0, "Available Chopsticks:");
    for (int i = 0; i < PHILOSOPHERS; i++) {
        if (chopstick_available[i])
            attron(COLOR_PAIR(CHOPSTICK_FREE));
        else
            attron(COLOR_PAIR(CHOPSTICK_USED));

        mvprintw(9, i * 12, "[%d] %-3s", i, chopstick_available[i] ? "YES" : "NO");
        attroff(COLOR_PAIR(CHOPSTICK_FREE));
        attroff(COLOR_PAIR(CHOPSTICK_USED));
    }

    mvprintw(11, 0, "Legend:\n");
    attron(COLOR_PAIR(THINKING)); printw(" T "); attroff(COLOR_PAIR(THINKING)); printw(" - Thinking\n");
    attron(COLOR_PAIR(EATING)); printw(" E "); attroff(COLOR_PAIR(EATING)); printw(" - Eating\n");
    attron(COLOR_PAIR(STARVING)); printw(" S "); attroff(COLOR_PAIR(STARVING)); printw(" - Starving\n");
    attron(COLOR_PAIR(HOLDING)); printw(" L/R "); attroff(COLOR_PAIR(HOLDING)); printw(" - Holding chopstick\n");

    if (paused) mvprintw(17, 0, "-- PAUSED --");

    refresh();
    sem_post(&screen_lock);
}


void pause_barrier() {
    while (paused && running) {
        sleep(2);
    }
}

