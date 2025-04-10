# Dining_Philosophers
Dining Philosophers Problem Simulation in C/C++ (Linux).

This simulation models the classic Dining Philosophers Problem, exploring different scenarios involving deadlock and starvation. The program demonstrates three possible situations:

1. No Deadlock or Starvation: 
            Philosophers can eat without issues, the system runs smoothly without deadlocks or starvation.
2. Deadlock Occurs, No Starvation: 
            Deadlock occurs when each philosopher picks up a chopstick and refuses to put it down, leading to a blocked system, but no philosopher experiences starvation.
3. No Deadlock, Starvation Occurs: 
            Starvation happens when the weakest philosopher never or rarely gets a chopstick, though no deadlock occurs.
