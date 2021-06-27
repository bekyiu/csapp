cmake_minimum_required(VERSION 3.13)
project(csapp C)

set(CMAKE_C_STANDARD 11)

#include_directories("ass/*.h")

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
foreach (file ${files})
    add_executable(csapp ${file})
endforeach ()
