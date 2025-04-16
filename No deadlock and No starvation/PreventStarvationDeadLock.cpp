// zapobieganie zakleszczeniu i zaglodzeniu

#include <ncurses.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <random>
#include <iostream>
#include <ctime>
#include <cstdarg>

#define PHILOSOPHERS 5
#define STARTING_POINTS 50
#define GAIN_POINTS 10
#define LOOSE_POINTS 5
#define MAX_POINTS 100
#define MIN_POINTS 0
#define NO_EATING_TIME 3 // czas po którym uznajemy, że filozof jest głodny

enum StatusColor {
    THINKING = 1,
    EATING,
    RELEASING,
    REQUEST,
    RETURN,
    POINTS,
    STARVATION,
    DEFAULT_COLOR
};

bool running = true;
bool colors_enabled = false;
sem_t chopsticks[PHILOSOPHERS];
sem_t points_lock;
sem_t screen_lock;
int points[PHILOSOPHERS];
time_t last_ate[PHILOSOPHERS]; // czas ostatniego jedzenia

void* prevent_starvation_deadlock(void* arg);
void* hunger_manager(void* arg);
void think(int id);
void eat(int id);
int random_between(int a, int b);
void safe_print(int color, const char* format, ...);

int main() {
    pthread_t philosophers[PHILOSOPHERS];
    pthread_t manager;
    int ids[PHILOSOPHERS];

    initscr();
    cbreak();
    scrollok(stdscr, TRUE);
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);


    printw("Deadlock and Starvation Prevention Simulation\n\n");
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
        init_pair(RELEASING, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(REQUEST, COLOR_WHITE, COLOR_BLACK);
        init_pair(RETURN, COLOR_RED, COLOR_BLACK);
        init_pair(POINTS, COLOR_YELLOW, COLOR_BLACK);
        init_pair(STARVATION, COLOR_RED, COLOR_BLACK);
        colors_enabled = true;
    }

    sem_init(&screen_lock, 0, 1);
    sem_init(&points_lock, 0, 1);
    for (int i = 0; i < PHILOSOPHERS; ++i) {
        sem_init(&chopsticks[i], 0, 1);
        points[i] = STARTING_POINTS;
        last_ate[i] = time(NULL); // inicjalny czas ostatniego jedzenia
    }

    for (int i = 0; i < PHILOSOPHERS; ++i) {
        ids[i] = i;
        pthread_create(&philosophers[i], NULL, prevent_starvation_deadlock, &ids[i]);
    }

    pthread_create(&manager, NULL, hunger_manager, NULL);

    while (running) {
        int ch = getch();
        if (ch == 'q' || ch == 'Q') {
            running = false;
        }
        usleep(100000); // 0.1 sekundy
    }

    for (int i = 0; i < PHILOSOPHERS; ++i) {
        pthread_join(philosophers[i], NULL);
    }
    pthread_join(manager, NULL);

    sem_destroy(&screen_lock);
    sem_destroy(&points_lock);
    for (int i = 0; i < PHILOSOPHERS; ++i) {
        sem_destroy(&chopsticks[i]);
    }

    endwin();
    return 0;
}

void* prevent_starvation_deadlock(void* arg){
    int id = *(int*)arg;
    int left = (id + 1) % PHILOSOPHERS;
    int right = id;

    while (running) {
        think(id);

        int is_even = id % 2 == 0;
        if (is_even) {
            safe_print(REQUEST, "Philosopher %d is acquiring (right) %d chopstick\n", id, right);
            sem_wait(&chopsticks[right]);
            safe_print(REQUEST, "Philosopher %d is acquiring (left) %d chopstick\n", id, left);
            sem_wait(&chopsticks[left]);
        } else {
            safe_print(REQUEST, "Philosopher %d is acquiring (left) %d chopstick\n", id, left);
            sem_wait(&chopsticks[left]);
            safe_print(REQUEST, "Philosopher %d is acquiring (right) %d chopstick\n", id, right);
            sem_wait(&chopsticks[right]);
        }

        eat(id);

        if (is_even) {
            safe_print(RELEASING, "Philosopher %d is releasing (right) %d chopstick\n", id, right);
            sem_post(&chopsticks[right]);
            safe_print(RELEASING, "Philosopher %d is releasing (left) %d chopstick\n", id, left);
            sem_post(&chopsticks[left]);
        } else {
            safe_print(RELEASING, "Philosopher %d is releasing (left) %d chopstick\n", id, left);
            sem_post(&chopsticks[left]);
            safe_print(RELEASING, "Philosopher %d is releasing (right) %d chopstick\n", id, right);
            sem_post(&chopsticks[right]);
        }

        safe_print(RELEASING, "Philosopher %d released both chopsticks\n", id);
    }

    return nullptr;
}

void* hunger_manager(void* arg) {
    while (running) {
        sleep(NO_EATING_TIME);
        time_t now = time(NULL);

        sem_wait(&points_lock);
        for (int i = 0; i < PHILOSOPHERS; i++) {
            if (difftime(now, last_ate[i]) >= NO_EATING_TIME) {
//                safe_print();
                points[i] -= LOOSE_POINTS;
                if (points[i] < MIN_POINTS)
                    points[i] = MIN_POINTS;

                safe_print(POINTS, "Philosopher %d has %d hunger points\n", i, points[i]);

                if (points[i] == MIN_POINTS)
                    safe_print(STARVATION, "!!! PHILOSOPHER %d IS STARVING !!!\n", i);
            }
        }
        sem_post(&points_lock);
    }
    return nullptr;
}

void think(int id) {
    safe_print(THINKING, "Philosopher %d is thinking...\n", id);
    sleep(random_between(2, 5));
}

void eat(int id) {
    safe_print(EATING, "Philosopher %d is eating...\n", id);

    sem_wait(&points_lock);
    points[id] += GAIN_POINTS;
    if (points[id] > MAX_POINTS)
        points[id] = MAX_POINTS;
    last_ate[id] = time(NULL); // zaktualizuj czas ostatniego jedzenia
    sem_post(&points_lock);

    sleep(random_between(1, 4));
}

int random_between(int a, int b) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(a, b);
    return dist(gen);
}

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
