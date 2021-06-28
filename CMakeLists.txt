cmake_minimum_required(VERSION 3.13)
project(csapp C)

set(CMAKE_C_STANDARD 11)


file(GLOB_RECURSE files "${CMAKE_CURRENT_SOURCE_DIR}/bekyiu/*.c")
foreach (file ${files})
    message(${file})
endforeach ()
add_executable(csapp ${files})
