#include "PreventStarvationDeadLock.h"

#include <ncurses.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <random>
#include <ctime>
#include <cstring>
#include <deque>
#include <algorithm>

#define PHILOSOPHERS 5
#define STARTING_POINTS 50
#define GAIN_POINTS 10
#define LOOSE_POINTS 10
#define MAX_POINTS 100
#define MIN_POINTS 0
#define NO_EATING_TIME 5

enum StatusColor {
    THINKING = 1,
    EATING,
    STARVING,
    HOLDING,
    CHOPSTICK_FREE,
    CHOPSTICK_USED
};

bool running = true;
bool paused = false;
sem_t chopsticks[PHILOSOPHERS];
int points[PHILOSOPHERS];
std::deque<int> hunger_queue;
bool is_hungry[PHILOSOPHERS] = {false};
sem_t points_lock;
sem_t pause_lock;
sem_t screen_lock;

char state[PHILOSOPHERS][4];
// tablica zjedzonych posilkow
int meals[PHILOSOPHERS];
// tablica dostepnych paleczek
bool chopstick_available[PHILOSOPHERS];

// tablica przechowujaca czas kiedy filozof ostatnio jadl
time_t last_ate[PHILOSOPHERS];

void* philosopher_thread(void* arg);
void* hunger_monitor(void* arg);
void draw_interface();
void pause_barrier();
int random_between(int a, int b);

int main() {
    pthread_t philosophers[PHILOSOPHERS];
    pthread_t monitor;
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
        init_pair(STARVING, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(HOLDING, COLOR_YELLOW, COLOR_BLACK);
        init_pair(CHOPSTICK_FREE, COLOR_WHITE, COLOR_BLACK);
        init_pair(CHOPSTICK_USED, COLOR_RED, COLOR_BLACK);
    }

    sem_init(&screen_lock, 0, 1);
    sem_init(&pause_lock, 0, 1);
    sem_init(&points_lock, 0, 1);
    for (int i = 0; i < PHILOSOPHERS; i++) {
        sem_init(&chopsticks[i], 0, 1);
        strcpy(state[i], "T");
        chopstick_available[i] = true;
        meals[i] = 0;
        last_ate[i] = time(NULL);
        points[i] = STARTING_POINTS;
    }

    for (int i = 0; i < PHILOSOPHERS; i++) {
        ids[i] = i;
        pthread_create(&philosophers[i], NULL, philosopher_thread, &ids[i]);
    }

    pthread_create(&monitor, NULL, hunger_monitor, NULL);

    while (running) {
        draw_interface();

        int ch = getch();
        if (ch == 'q' || ch == 'Q') running = false;
        if (ch == ' ') paused = !paused;

        usleep(100000); // 0.1s
    }

    for (int i = 0; i < PHILOSOPHERS; i++) pthread_join(philosophers[i], NULL);
    pthread_join(monitor, NULL);

    for (int i = 0; i < PHILOSOPHERS; i++) sem_destroy(&chopsticks[i]);
    sem_destroy(&points_lock);
    sem_destroy(&pause_lock);
    sem_destroy(&screen_lock);

    endwin();
    return 0;
}

void* philosopher_thread(void* arg) {
    int id = *(int*)arg;
    int left = (id + 1) % PHILOSOPHERS;
    int right = id;

    while (running) {

        pause_barrier();

        // sprawedzenie czy filozof jest w kolejce
        bool is_in_queue = false;
        sem_wait(&points_lock);
        if (!hunger_queue.empty() && hunger_queue.front() == id) {
            is_in_queue = true;
            hunger_queue.pop_front();
        }
        sem_post(&points_lock);

        int is_even = id % 2 == 0;

        pause_barrier();
        if(!is_in_queue){
            // myslenie
            strcpy(state[id], "T");
            sleep(random_between(2, 5));
        }

        // podnoszenie paleczek
        if (is_in_queue) {
            sem_wait(&chopsticks[right]);
            strcpy(state[id], ".P");
            chopstick_available[right] = false;

            sem_wait(&chopsticks[left]);
            strcpy(state[id], "LP");
            chopstick_available[left] = false;
        }
        else {
            if (is_even) {
                sem_wait(&chopsticks[right]);
                strcpy(state[id], ".P");
                chopstick_available[right] = false;

                sem_wait(&chopsticks[left]);
                strcpy(state[id], "LP");
                chopstick_available[left] = false;
            } else {
                sem_wait(&chopsticks[left]);
                strcpy(state[id], "L.");
                chopstick_available[left] = false;

                sem_wait(&chopsticks[right]);
                strcpy(state[id], "LP");
                chopstick_available[right] = false;
            }

        }

        // jedzenie posilku
        pause_barrier();

        sem_wait(&points_lock);
        points[id] += GAIN_POINTS;
        if (points[id] > MAX_POINTS)
            points[id] = MAX_POINTS;
        strcpy(state[id], "E");
        meals[id]++;
        last_ate[id] = time(NULL);
        sleep(random_between(1, 4));

        sem_post(&points_lock);

        // zwalnianie paleczek

        if (is_even) {
            sem_post(&chopsticks[right]);
            strcpy(state[id], "L.");
            chopstick_available[right] = true;

            sem_post(&chopsticks[left]);
            strcpy(state[id], "..");
            chopstick_available[left] = true;
        } else {
            sem_post(&chopsticks[left]);
            strcpy(state[id], ".P");
            chopstick_available[left] = true;

            sem_post(&chopsticks[right]);
            strcpy(state[id], "..");
            chopstick_available[right] = true;
        }

    }

    return nullptr;
}

void* hunger_monitor(void* arg) {
    while (running) {
        pause_barrier();
        time_t now = time(NULL);

        sem_wait(&points_lock);
        for (int i = 0; i < PHILOSOPHERS; i++) {
            // sprawdzenie czy filozof nie jadl przez zadany przedzial czasu
            if (last_ate[i] > 0 && difftime(now, last_ate[i]) >= NO_EATING_TIME) {
                points[i] -= LOOSE_POINTS;
                if (points[i] < MIN_POINTS)
                    points[i] = MIN_POINTS;

                // sprawdzenie czy filozof "gloduje" i czy jest juz w kolejce
                if (points[i] == MIN_POINTS && !is_hungry[i] && std::find(hunger_queue.begin(), hunger_queue.end(), i) == hunger_queue.end()) {
                    strcpy(state[i], "S");
                    hunger_queue.push_back(i);
                    is_hungry[i] = true;
                }



            }
        }
        sem_post(&points_lock);
        sleep(1);
    }
    return nullptr;
}

void draw_interface() {
    sem_wait(&screen_lock);
    erase(); // zamiast clear, bo clear powoduje migotanie ekranu

    mvprintw(0, 0, "Dining Philosophers Simulation");
    mvprintw(1, 0, "Press SPACE to pause/resume, Q to quit.\n");

    for (int i = 0; i < PHILOSOPHERS; i++) {
        mvprintw(3, i * 18, "Philosopher %d", i);
        mvprintw(4, i * 18, "Meals: %d", meals[i]);
        mvprintw(5, i * 18, "Points: %d", points[i]);

        if (strcmp(state[i], "E") == 0) attron(COLOR_PAIR(EATING));
        else if (strcmp(state[i], "T") == 0) attron(COLOR_PAIR(THINKING));
        else if (strcmp(state[i], "S") == 0) attron(COLOR_PAIR(STARVING));
        else attron(COLOR_PAIR(HOLDING));

        mvprintw(6, i * 18, "State: %s", state[i]);
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
    attron(COLOR_PAIR(STARVING)); printw(" S "); attroff(COLOR_PAIR(STARVING)); printw(" - Starving (0 points)\n");
    attron(COLOR_PAIR(HOLDING)); printw(" L/P "); attroff(COLOR_PAIR(HOLDING)); printw(" - Holding chopstick\n");

    if (paused) mvprintw(17, 0, "-- PAUSED --");

    refresh();
    sem_post(&screen_lock);
}

void pause_barrier() {
    while (paused && running) {
        sleep(2);
    }
}

int random_between(int a, int b) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(a, b);
    return dis(gen);
}
