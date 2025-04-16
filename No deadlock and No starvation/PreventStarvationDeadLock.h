//
// Created by Glitch on 07/04/2025.
//

#ifndef DININGPHILOSOPHERS_PREVENTSTARVATIONDEADLOCK_H
#define DININGPHILOSOPHERS_PREVENTSTARVATIONDEADLOCK_H

//extern int no_philosophers;

void* prevent_starvation_deadlock(void* arg);
void* hunger_manager(void* arg);
int random_between(int a, int b);
void safe_print(int color, const char* format, ...);
//void prevent_starvation_deadlock(int id, int leftChopstick, int rightChopstick);
void structures_init();
void structures_destroy();


#endif //DININGPHILOSOPHERS_PREVENTSTARVATIONDEADLOCK_H
