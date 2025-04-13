//
// Created by Glitch on 07/04/2025.
//
#include "Simulation.h"
#include <semaphore.h>
#include <pthread.h>
#include <ncurses.h>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

#include "PreventDeadLock.h"
#include "PreventStarvation.h"

// semafory jako paleczki filozofow
sem_t * chopsticks;
// liczba filozofow
int no_philosophers = 5;
// watki jako filozofowie
pthread_t * philosophers;
// wybor uzytkownika w jakim trybie uruchomic symulacje
int choice = 0;

enum OPTIONS{
    PREVENT_DEADLOCK = 2,
    PREVENT_STARVATION = 1,
    PREVENT_STARVATION_AND_DEADLOCK = 3
};





int main(int argc, char* argv[]){
//    while (true) {
        init();
        printMenu();
        destroy();
        exitMenu();
//    }
	return 0;

}

void *philosophy(void *arg){
    // id watku
    int id = (int) (uintptr_t) arg;
    // prawa paleczka filozowa o numerze id
    int rightChopstick = id;
    // lewa paleczka filozowa o numerze id
    int leftChopstick = (id + 1) % no_philosophers;


    switch (choice) {
        case PREVENT_DEADLOCK: // PREVENT_DEADLOCK
            prevent_deadlock(id, leftChopstick, rightChopstick);
            break;

        case PREVENT_STARVATION: // PREVENT_STARVATION
            prevent_starvation(id, leftChopstick, rightChopstick);
            break;

//        case PREVENT_STARVATION_AND_DEADLOCK: // PREVENT_STARVATION_AND_DEADLOCK
//            prevent_starvation_deadlock(id, leftChopstick, rightChopstick);
//            break;

        default:
            printf("Invalid strategy selected: %d\n", choice);
            break;
    }

    return NULL;


}

void init(){
   	 // alokacja pamieci na paleczek
    chopsticks = (sem_t *) calloc(no_philosophers, sizeof(sem_t));
    if(!chopsticks)
        perror("Chopsticks Memory allocation error");

	// alokacji pamieci na filozofow
	philosophers = (pthread_t *) calloc(no_philosophers, sizeof(pthread_t));
    if(!philosophers)
        perror("Philosophers Memory allocation error");

    // inicjalizacja semaforow jako mutex-y -> zamek (wartosc poczatkowa = 1)
    for (int i = 0; i < no_philosophers; i++) {
        if (sem_init(&chopsticks[i], 0, 1) != 0)
            perror("Semaphores initialization error!");

        // utworzenie watkow
        if (pthread_create(&philosophers[i], NULL, philosophy, (void *) (uintptr_t) i) != 0)
            perror("Threads initialization error!");
    }

    // czekanie na zakonczenie pracy watkow
    for (int i = 0; i < no_philosophers; i++)
        if(pthread_join(philosophers[i], NULL) != 0)
            perror("Thread wait error!");
}


void destroy(){

    free(philosophers);

    // dealokacja semaforow
    for (int i = 0; i < no_philosophers; i++){
        sem_destroy(&chopsticks[i]);

    }

	free(chopsticks);

}

void printMenu(){
    initscr();
   	printw("Dining Philosophers Problem\n\n");
    printw("1) No starvation with deadlock\n");
    printw("2) No deadlock with starvation\n");
    printw("3) No deadlock and starvation\n");
    printw("Choose option [1-3]: ");
    refresh();


    choice = getch() - '0';
    noecho();

    clear();
    printw("Option number %d has been chosen\n", choice);
    refresh();
}

void exitMenu(){
    getch();
	endwin();
}

