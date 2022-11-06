cmake_minimum_required(VERSION 3.20)
project(selection_benchmark C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

add_executable(selection_benchmark main.c select.c select.h array.c array.h util.c util.h stats.c stats.h select_cpp.cpp select_cpp.h)

target_compile_options(selection_benchmark PUBLIC -Wall -Wextra -pedantic -Werror -O3)

target_link_libraries(selection_benchmark m)
