cmake_minimum_required(VERSION 3.28)
project(TheGhostCrossword C)

set(CMAKE_C_STANDARD 11)

add_executable(TheGhostCrossword main.c
        fileUtils.c
        fileUtils.h
        initBoard.c
        initBoard.h
        threadsUtils.c
        threadsUtils.h
        parameters.h
        parameters.h
        gameLogicUtils.c
        gameLogicUtils.h)
