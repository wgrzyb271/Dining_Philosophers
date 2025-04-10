cmake_minimum_required(VERSION 3.28)
project(DiningPhilosophers)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-lncurses")

add_executable(DiningPhilosophers
        Simulation.h
        Simulation.cpp
        PreventDeadLock.h
        PreventDeadLock.cpp
        PreventStarvation.h
        PreventStarvation.cpp
        PreventStarvationDeadLock.h
        PreventStarvationDeadLock.cpp
)
