//
// Created by Glitch on 07/04/2025.
//
#include "Simulation.h"
#include <semaphore.h>
#include <pthread.h>
#include <ncurses.h>
#include <cstdlib>

// semafory jako paleczki filozofow
sem_t * chopsticks; 
// liczba filozofow
int no_philosophers;
// watki jako filozofowie
pthread_t * philosophers;


// funkcja inicalizujaca filozofow i paleczki
void init();
//funkcja zwalniajaca filozofow i paleczki
void destroy();
// funkcja wyswietlajaca TUI - Text User Interface
void printMenu();
// funkcja zwalniajaca zasoby przeznaczone na TUI
void exitMenu();


int main(int argc, char* argv[]){
	init();
	printMenu();
	exitMenu();
	destroy();
	return 0;

}

void init(){
    no_philosophers = 5;
   	 // alokacja pamieci na paleczek
    	chopsticks = (sem_t *) calloc(no_philosophers, sizeof(sem_t));
	// alokacji pamieci na filozofow
	philosophers = (pthread_t *) calloc(no_philosophers, sizeof(pthread_t));

    // inicjalizacja semaforow jako mutex-y -> zamek (wartosc poczatkowa = 1)
    for (int i = 0; i < no_philosophers; i++)
        sem_init(&chopsticks[i], 0, 1);

}


void destroy(){

    // dealokacja semaforow
    for (int i = 0; i < no_philosophers; i++){
        sem_destroy(&chopsticks[i]);

    }

	free(chopsticks);
	free(philosophers);
}

void printMenu(){
	initscr();
   	printw("Hello\n");
   	getch();
}

void exitMenu(){
	endwin();
}

