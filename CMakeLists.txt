cmake_minimum_required(VERSION 4.1)
project(Year1_Project C)

set(CMAKE_C_STANDARD 99)

add_library(Year1_Project STATIC main.c
        main.c)

add_executable(power_quality_analyser
        main.c
        waveform.c
        io.c
        io.c
        waveform.h
        waveform.c
        main.c
        io.h
)