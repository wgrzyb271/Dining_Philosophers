//
// Created by Glitch on 07/04/2025.
//

#include "PreventDeadLock.h"
#include "Simulation.h"

#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>



// rowiazanie z hierarchia zasobow
// filozof podnosi najpierw paleczke z najniszym dostepnym numerem - zapobiega zakleszczeniu
void prevent_deadlock(int id, int leftChopstick, int rightChopstick){
    while (true){

        /*
         * ==================
         *  Pick up ChopSticks
         */

        printw("Philosopher with id: %i and thread_id = %ld is thinking\n", id, (long) pthread_self());
        sleep(1);

        if(leftChopstick < rightChopstick){
            printw("Philosopher with id: %i and thread_id = %ld is acquiring left chopstick\n", id, (long) pthread_self());
            sem_wait(&chopsticks[leftChopstick]);

            printw("Philosopher with id: %i and thread_id = %ld is acquiring right chopstick\n", id, (long) pthread_self());
            sem_wait(&chopsticks[rightChopstick]);
        } else {
            printw("Philosopher with id: %i and thread_id = %ld is acquiring right chopstick\n", id, (long) pthread_self());
            sem_wait(&chopsticks[rightChopstick]);

            printw("Philosopher with id: %i and thread_id = %ld is acquiring left chopstick\n", id, (long) pthread_self());
            sem_wait(&chopsticks[leftChopstick]);
        }

        printw("Philosopher with id: %i and thread_id = %ld acquired both chopsticks\n", id, (long) pthread_self());


        /*
         * ==================
         *  Eat
         */

        printw("Philosopher with id: %i and thread_id = %ld is eating\n", id, (long) pthread_self());
        sleep(2);




        /*
         * ==================
         *  Release ChopSticks
         */

        if(leftChopstick < rightChopstick){

            printw("Philosopher with id: %i and thread_id = %ld is releasing right chopstick\n", id, (long) pthread_self());
            sem_post(&chopsticks[rightChopstick]);

            printw("Philosopher with id: %i and thread_id = %ld is releasing left chopstick\n", id, (long) pthread_self());
            sem_post(&chopsticks[leftChopstick]);

        } else {
            printw("Philosopher with id: %i and thread_id = %ld is releasing left chopstick\n", id, (long) pthread_self());
            sem_post(&chopsticks[leftChopstick]);

            printw("Philosopher with id: %i and thread_id = %ld is releasing right chopstick\n", id, (long) pthread_self());
            sem_post(&chopsticks[rightChopstick]);
        }

        printw("Philosopher with id: %i and thread_id = %ld released both chopsticks\n", id, (long) pthread_self());
    }
}