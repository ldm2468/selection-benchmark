cmake_minimum_required(VERSION 3.20)
project(deterministic_select C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(deterministic_select main.c select.c select.h array.c array.h util.c util.h stats.c stats.h)

target_compile_options(deterministic_select PUBLIC -Wall -Wextra -pedantic -Werror -O2)

target_link_libraries(deterministic_select m)
