cmake_minimum_required(VERSION 3.10)
#set_property(TARGET tgt PROPERTY C_STANDARD 11)

# set the project name
project(WizDumb)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

# add the executable
add_executable(chess
    board.c board_hash.c check.c debug.c evaluate.c game.c generate.c material.c move.c 
    move_list.c move_tree.c piece.c play.c search.c sha1.c str.c timer.c
)
