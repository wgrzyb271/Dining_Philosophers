cmake_minimum_required(VERSION 3.30)
project(NoDeadlock)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-lncurses")

add_executable(NoDeadlock

        PreventDeadLock.h
        PreventDeadLock.cpp
)
