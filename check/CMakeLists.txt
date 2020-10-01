cmake_minimum_required(VERSION 3.10)
#set_property(TARGET tgt PROPERTY C_STANDARD 11)

set(CMAKE_BUILD_TYPE Debug)

# set the project name
project(WizDumb)

# add the executable
add_executable(chess
    board.c board_hash.c check.c debug.c evaluate.c game.c generate.c material.c move.c 
    move_list.c move_tree.c piece.c play.c search.c sha1.c str.c timer.c
)
