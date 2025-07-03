project(dwerb)


cmake_minimum_required(VERSION 4.0)
set(CMAKE_C_STANDARD 99)

add_executable(dwerb dwerb.c)

target_compile_options(dwerb PRIVATE -Wall -Wextra -pedantic)
