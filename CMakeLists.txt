cmake_minimum_required(VERSION 3.14)
project(kyou)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -ggdb")

add_executable(kyou interpret_main.c file.c interpret.c ast.c tokens.c utf8.c hash.c list.c)
add_executable(kyouc compiler.c file.c ast.c tokens.c utf8.c hash.c list.c)