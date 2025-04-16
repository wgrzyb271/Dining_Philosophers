// zapobieganie zaglodzeniu

#include "PreventStarvation.h"
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <cstdarg>
#include <random>
#include <deque>
#include <iostream>

#define PHILOSOPHERS 5

sem_t chopsticks[PHILOSOPHERS];
sem_t screen_lock;

bool running = true;
bool colors_enabled = false;

enum StatusColor {
    THINKING,
    HUNGRY,
    EATING,
    RELEASING,
    DEFAULT_COLOR
};

// tablica okreslajaca aktualny stan danego filozofa
int philosopherState[PHILOSOPHERS];
// kolejka zawierajaca glodnych filozofow (w stanie HUNGRY)
std::deque<int> hungerQueue;
// mutex dotyczacy kolejki
sem_t queueLock;



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
void* prevent_starvation(void* arg) {
    int id = *(int*)arg;
    int left = (id + 1) % PHILOSOPHERS;
    int right = id;

    while (running) {
        switch (philosopherState[id]) {
            case THINKING:
                safe_print(THINKING, "Philosopher %d is thinking\n", id);
                // filozof mysli - czas wylosowany w sekunach [1, 5] sekund
                sleep(random_between(2, 5));

                // filozof staje sie glodny
                philosopherState[id] = HUNGRY;

                // dodanie glodnego filozofa do kolejki
                sem_wait(&queueLock);
                if (!isPhilosopherInQueue(id))
                    hungerQueue.push_back(id);
                sem_post(&queueLock);
                break;

            case HUNGRY:
                // filozof jest glodny - sprawdzamy kolejke

                // zablokowanie "kolejki" zeby miec pewnosc ze nie ulega ona zmianie podczas operowaniu na niej
                sem_wait(&queueLock);

                // sprawdzenie czy kolejka nie jest pusta i czy jest kolej filozofa o numerze id
                if (!hungerQueue.empty() && hungerQueue.front() == id) {
                    // sprawdzenie czy obie paleczki sa wolne
                    if (sem_trywait(&chopsticks[left]) == 0 && sem_trywait(&chopsticks[right]) == 0) {
                        safe_print(RELEASING, "Philosopher %d is acquiring left chopstick (%d)\n", id, left);
                        safe_print(RELEASING, "Philosopher %d is acquiring right chopstick (%d)\n", id, right);
                        // filozof zaczyna jesc
                        philosopherState[id] = EATING;

                        // usuniecie filozofa z kolejki
                        hungerQueue.pop_front();
                    } else {
                        // jezeli paleczki nie sa wolne, filozof pozostaje w kolejce
                        sem_post(&queueLock);
                        break;
                    }
                }

                // odblokowanie kolejki
                sem_post(&queueLock);

                break;

            case EATING:
                // filozof je - czas wylosowany w sekunach: [1, 5] sekund
                safe_print(EATING, "Philosopher %d is eating\n", id);
                sleep(random_between(1, 4));
                philosopherState[id] = THINKING;

                // filozof odklada paleczki
                safe_print(RELEASING, "Philosopher %d is releasing left chopstick (%d)\n", id, left);
                sem_post(&chopsticks[left]);
                safe_print(RELEASING, "Philosopher %d is releasing right chopstick (%d)\n", id, right);
                sem_post(&chopsticks[right]);
                safe_print(RELEASING, "Philosopher %d released both chopsticks\n", id);
                break;
        }
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

    printw("Starvation Prevention Simulation\n\n");
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
    sem_init(&queueLock, 0, 1);
    for (int i = 0; i < PHILOSOPHERS; ++i) {
        sem_init(&chopsticks[i], 0, 1);
        philosopherState[i] = THINKING;
    }

    for (int i = 0; i < PHILOSOPHERS; ++i) {
        ids[i] = i;
        pthread_create(&philosophers[i], NULL, prevent_starvation, &ids[i]);
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
    sem_destroy(&queueLock);
    for (int i = 0; i < PHILOSOPHERS; ++i) {
        sem_destroy(&chopsticks[i]);
    }

    endwin();
    return 0;
}




bool isPhilosopherInQueue(int i){
    for (int current: hungerQueue)
        if (current == i) return true;

    return false;
}
