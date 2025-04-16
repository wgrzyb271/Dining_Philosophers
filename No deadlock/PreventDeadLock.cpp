// zapobieganie zakleszczeniu


#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdarg.h>
#include <random>

#define PHILOSOPHERS 5

sem_t chopsticks[PHILOSOPHERS];
sem_t screen_lock;

bool running = true;
bool colors_enabled = false;

enum StatusColor {
    THINKING,
    EATING,
    RELEASING,
    DEFAULT_COLOR
};

// Funkcja generujaca losowa liczbe calkowita z przedzialu [a, b]
int random_between(int a, int b) {
    static std::random_device rd;
    static std::mt19937 gen(rd()); // generator Mersenne Twister
    std::uniform_int_distribution<> dist(a, b);
    return dist(gen);
}

// Funkcja pomocnicza do bezpiecznego wypisywania
void safe_print(int color, const char* format, ...) {
    va_list args;
    sem_wait(&screen_lock);

    if (colors_enabled) {
        attron(COLOR_PAIR(color));
    }

    va_start(args, format);
    vw_printw(stdscr, format, args);
    va_end(args);

    if (colors_enabled) {
        attroff(COLOR_PAIR(color));
    }

    refresh();
    sem_post(&screen_lock);
}

// Funkcja filozofa z zapobieganiem zakleszczeniu
void* philosopher_thread(void* arg) {
    int id = *(int*)arg;
    int left = (id + 1) % PHILOSOPHERS;
    int right = id;

    while (running) {
        safe_print(THINKING, "Philosopher %d is thinking\n", id);
        sleep(random_between(2, 5));

        if (left < right) {
            safe_print(RELEASING, "Philosopher %d is acquiring left chopstick (%d)\n", id, left);
            sem_wait(&chopsticks[left]);

            safe_print(RELEASING, "Philosopher %d is acquiring right chopstick (%d)\n", id, right);
            sem_wait(&chopsticks[right]);
        } else {
            safe_print(RELEASING, "Philosopher %d is acquiring right chopstick (%d)\n", id, right);
            sem_wait(&chopsticks[right]);

            safe_print(RELEASING, "Philosopher %d is acquiring left chopstick (%d)\n", id, left);
            sem_wait(&chopsticks[left]);
        }

        safe_print(EATING, "Philosopher %d is eating\n", id);
        sleep(random_between(1, 4));

        if (left < right) {
            safe_print(RELEASING, "Philosopher %d is releasing right chopstick (%d)\n", id, right);
            sem_post(&chopsticks[right]);

            safe_print(RELEASING, "Philosopher %d is releasing left chopstick (%d)\n", id, left);
            sem_post(&chopsticks[left]);
        } else {
            safe_print(RELEASING, "Philosopher %d is releasing left chopstick (%d)\n", id, left);
            sem_post(&chopsticks[left]);

            safe_print(RELEASING, "Philosopher %d is releasing right chopstick (%d)\n", id, right);
            sem_post(&chopsticks[right]);
        }

        safe_print(RELEASING, "Philosopher %d released both chopsticks\n", id);
    }

    return NULL;
}

int main() {
    pthread_t philosophers[PHILOSOPHERS];
    int ids[PHILOSOPHERS];

    initscr();
    cbreak();
    scrollok(stdscr, TRUE);
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);

//    timeout(0);

    printw("Deadlock Prevention Simulation\n\n");
    printw("Press any key to start Simulation (q or Q to exit)...\n\n");
    int choice = getch();
    if(choice == 'q' or choice == 'Q')
        running = false;
    clear();
    nodelay(stdscr, TRUE);

    if (has_colors()) {
        start_color();
        init_pair(THINKING, COLOR_CYAN, COLOR_BLACK);
        init_pair(EATING, COLOR_GREEN, COLOR_BLACK);
        init_pair(RELEASING, COLOR_YELLOW, COLOR_BLACK);
        colors_enabled = true;
    } else {
        colors_enabled = false;
        init_pair(DEFAULT_COLOR, COLOR_WHITE, COLOR_BLACK);
    }

    sem_init(&screen_lock, 0, 1);
    for (int i = 0; i < PHILOSOPHERS; ++i) {
        sem_init(&chopsticks[i], 0, 1);
    }

    for (int i = 0; i < PHILOSOPHERS; ++i) {
        ids[i] = i;
        pthread_create(&philosophers[i], NULL, philosopher_thread, &ids[i]);
    }

    while (running) {
        int ch = getch();
        if (ch == 'q' || ch == 'Q') {
            running = false;
        }
        usleep(100000); // 0.1s
    }

    for (int i = 0; i < PHILOSOPHERS; ++i) {
        pthread_join(philosophers[i], NULL);
    }

    sem_destroy(&screen_lock);
    for (int i = 0; i < PHILOSOPHERS; ++i) {
        sem_destroy(&chopsticks[i]);
    }

    endwin();
    return 0;
}
