//
// Created by Glitch on 07/04/2025.
//

#include "PreventStarvation.h"
#include "Simulation.h"

#include <queue>
#include <ncurses.h>
#include <iostream>
#include <unistd.h>


// rozwiazanie z kolejka. Stan filozofa okresla czy jest glodny, je lub mysli
// jezeli filozof jest glodny jest dodawany do kolejki, ktora pozwala mu w trakcie jego
// ruchu na podniesienie paleczek

enum STATE_VALUES{
    THINKING = 2,
    HUNGRY = 1,
    EATING = 0
};

// tablica okreslajaca aktualny stan danego filozofa
int* philosopherState = nullptr;
// kolejka zawierajaca glodnych filozofow (w stanie HUNGRY)
std::deque<int> hungerQueue;
// mutex dotyczacy kolejki
sem_t queueLock;


void prevent_starvation(int id, int leftChopstick, int rightChopstick) {
    init_structures();
    srand(time(NULL));

    while (!checkExitRequest(id)) {
        switch (philosopherState[id]) {
            case THINKING:
                // filozof mysli - czas wylosowany w sekunach [1, 5] sekund
                sleep(rand() % 5 + 1);

                // filozof staje sie glodny
                philosopherState[id] = HUNGRY;

                // dodanie glodnego filozofa do kolejki
                sem_wait(&queueLock);
                if (!isPhilosopherInQueue(id))
                    hungerQueue.push_back(id);
                sem_post(&queueLock);

                printStates();
                break;

            case HUNGRY:
                // filozof jest glodny - sprawdzamy kolejke

                // zablokowanie "kolejki" zeby miec pewnosc ze nie ulega ona zmianie podczas operowaniu na niej
                sem_wait(&queueLock);

                // sprawdzenie czy kolejka nie jest pusta i czy jest kolej filozofa o numerze id
                if (!hungerQueue.empty() && hungerQueue.front() == id) {
                    // sprawdzenie czy obie paleczki sa wolne
                    if (sem_trywait(&chopsticks[leftChopstick]) == 0 && sem_trywait(&chopsticks[rightChopstick]) == 0) {
                        // filozof zaczyna jesc
                        philosopherState[id] = EATING;

                        // usuniecie filozofa z kolejki
                        hungerQueue.pop_front();
                        printStates();
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
                printStates(id);
                sleep(rand() % 5 + 1);
                philosopherState[id] = THINKING;
                printStates();

                // filozof odklada paleczki
                sem_post(&chopsticks[leftChopstick]);
                sem_post(&chopsticks[rightChopstick]);
                break;
        }
    }

}


bool isPhilosopherInQueue(int i){
    for (int current: hungerQueue)
        if (current == i) return true;

    return false;
}

bool checkExitRequest(int id) {
    int ch = getch();
    if (ch == 'q' || ch == 'Q') {
        destroy_structures();
        endwin();
        std::cout << "\n[Philosopher " << id << "] Simulation aborted by user.\n";
        return true;
    }
    return false;
}


void init_structures(){
    try {
        philosopherState = new int[no_philosophers];
    } catch (std::bad_alloc& e) {
        std::cerr << "Memory allocation error: " << e.what() << std::endl;
    }

    sem_init(&queueLock, 0, 1);

    if (has_colors() == FALSE) {
        std::cerr << "Colors are not supported!\n";
        return;
    }

    start_color();

    // inicjalizacja par kolorow
    init_pair(1, COLOR_BLUE, COLOR_BLACK);   // Myslący - niebieski
    init_pair(2, COLOR_YELLOW, COLOR_BLACK); // Glodny - zolty
    init_pair(3, COLOR_GREEN, COLOR_BLACK);  // Jedzacy - zielony

    for(int i = 0; i < no_philosophers; i++)
        philosopherState[i] = THINKING;

}



void destroy_structures(){
    delete[] philosopherState;
    philosopherState = nullptr;
    sem_destroy(&queueLock);
}


void printStates(int currentId) {
    clear();  // wyczysc ekran
    printw("Philosophers States:\n\n");

    if (has_colors()) {
        // jezeli kolory sa dostepne, uzywamy kolorow
        for (int i = 0; i < no_philosophers; ++i) {
            if (i == currentId) attron(A_BOLD | A_STANDOUT);  // podswietlenie filozofa, ktory je

            // ustawienie koloru na podstawie stanu filozofa
            switch (philosopherState[i]) {
                case THINKING:
                    attron(COLOR_PAIR(1));  // Myslacy - niebieski
                    break;
                case HUNGRY:
                    attron(COLOR_PAIR(2));  // Glodny - zolty
                    break;
                case EATING:
                    attron(COLOR_PAIR(3));  // Jedzacy - zielony
                    break;
                default:
                    attron(COLOR_PAIR(0));  // Domyslny kolor
                    break;
            }

            printw("Philosopher %d: %s\n", i, stateToString(philosopherState[i]));
            attroff(COLOR_PAIR(1) | COLOR_PAIR(2) | COLOR_PAIR(3));  // wylacz kolory po uzyciu

            if (i == currentId) attroff(A_BOLD | A_STANDOUT);  // wylacz podswietlenie
            napms(500);  // Opóźnienie w milisekundach (500 ms)
        }
    } else {
        // jezeli kolory nie sa dostepne, wyswietl stan bez kolorow
        for (int i = 0; i < no_philosophers; ++i) {
            printw("Philosopher %d: %s\n", i, stateToString(philosopherState[i]));
            napms(500);
        }
    }

    printw("\nHungry Philosophers Queue:\n");
    for (int id : hungerQueue)
        printw("%d ", id);

    refresh();
}


const char* stateToString(int state) {
    switch(state) {
        case THINKING: return "THINKING";
        case HUNGRY: return "HUNGRY";
        case EATING: return "EATING";
        default: return "UNKNOWN";
    }
}
