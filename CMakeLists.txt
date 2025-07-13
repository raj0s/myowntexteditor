project(dwerb)

cmake_minimum_required(VERSION 4.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 99)

add_executable(dwerb dwerb.c)

set(SDL3_DIR "/opt/homebrew/opt/sdl3")
target_include_directories(dwerb PRIVATE "${SDL3_DIR}/include")
link_directories("${SDL3_DIR}/lib")
target_compile_options(dwerb PRIVATE -Wall -Wextra -pedantic)
