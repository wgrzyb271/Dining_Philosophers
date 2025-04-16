cmake_minimum_required(VERSION 3.30)
project(NoStarvation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-lncurses")

add_executable(NoStarvation

        PreventStarvation.h
        PreventStarvation.cpp

)
