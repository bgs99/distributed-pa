cmake_minimum_required(VERSION 3.13)
project(distributedomputing_pa1 C)

set(CMAKE_C_STANDARD 99)
add_compile_options(-Wall -pedantic)
add_executable(distributedomputing_pa1 main.c ipc.h common.h pa1.h distributed.h ipc.c logger.h)