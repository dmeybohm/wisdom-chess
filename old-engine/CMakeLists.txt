set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required(VERSION 3.20)

project(Wisdom C CXX)

add_library(chess_lib
board.c
board.h
check.c
check.h
coord.h
debug.c
debug.h
evaluate.c
evaluate.h
generate.c
generate.h
move.c
move.h
move_list.c
move_list.h
move_tree.c
move_tree.h
piece.c
piece.h
search.c
search.h
)

add_executable(chess  play.c)
target_link_libraries(chess PUBLIC chess_lib)

add_subdirectory(benchmark)
