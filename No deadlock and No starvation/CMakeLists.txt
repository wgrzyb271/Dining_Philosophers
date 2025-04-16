cmake_minimum_required(VERSION 3.30)
project(NoDeadlock_NoStarvation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-lncurses")

add_executable(NoDeadlock_NoStarvation

        PreventStarvationDeadLock.h
        PreventStarvationDeadLock.cpp
)
