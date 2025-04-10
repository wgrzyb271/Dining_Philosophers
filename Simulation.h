//
// Created by Glitch on 07/04/2025.
//

#ifndef DININGPHILOSOPHERS_SIMULATION_H
#define DININGPHILOSOPHERS_SIMULATION_H

#include <semaphore.h>

extern sem_t* chopsticks;
extern int no_philosophers;

// funkcja realizujaca zadanie filozofow (wywoluje zadany typ sumulajci)
void *philosophy(void *arg);
// funkcja inicalizujaca filozofow i paleczki
void init();
//funkcja zwalniajaca filozofow i paleczki
void destroy();
// funkcja wyswietlajaca TUI - Text User Interface
void printMenu();
// funkcja zwalniajaca zasoby przeznaczone na TUI
void exitMenu();

#endif //DININGPHILOSOPHERS_SIMULATION_H
