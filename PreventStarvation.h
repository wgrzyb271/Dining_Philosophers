//
// Created by Glitch on 07/04/2025.
//

#ifndef DININGPHILOSOPHERS_PREVENTSTARVATION_H
#define DININGPHILOSOPHERS_PREVENTSTARVATION_H

extern int no_philosophers;


void prevent_starvation(int id, int leftChopstick, int rightChopstick);
bool isPhilosopherInQueue(int i);

void printStates(int id = -1);
bool checkExitRequest(int id);
const char* stateToString(int state);
void init_structures();
void destroy_structures();



#endif //DININGPHILOSOPHERS_PREVENTSTARVATION_H
