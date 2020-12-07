#include "catch.hpp"

#include <iostream>

#include "move_list.hpp"
#include "board.h"
#include "generate.h"

TEST_CASE( "Initializing move list", "[move-list]")
{
    move_list_t move_list { Color::Black, {"e4 d4", "d2 d1"}};
    REQUIRE( move_list.size() == 2 );

    std::vector<move_t> moves;
    for (auto move : move_list)
    {
        moves.push_back(move);
    }
    std::vector expected = { parse_move("e4 d4"), parse_move("d2 d1")};
    REQUIRE( moves == expected );
}

move_list_t copy_moves_and_ptr (const move_t **ptr)
{
    struct board board;
    move_list_t moves = generate_moves (board, Color::White);
//    std::cout << "Moves first" << &moves.get_my_moves()[0] << "\n";
    *ptr = &moves.get_my_moves()[0];
    return moves;
}

TEST_CASE( "Returning move list moves ptr", "[move-list]")
{
    const move_t *ptr;
    move_list_t result = copy_moves_and_ptr (&ptr);
//    std::cout << "Moves first" << &result.get_my_moves()[0] << "\n";

    REQUIRE( &result.get_my_moves()[0] == ptr );
    REQUIRE( result.size() > 0 );
}