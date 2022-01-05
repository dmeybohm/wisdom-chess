#include "tests.hpp"
#include "move_list.hpp"
#include "board.hpp"
#include "generate.hpp"

using namespace wisdom;

TEST_CASE( "Initializing move list" )
{
    MoveList move_list {Color::Black, {"e4 d4", "d2 d1"}};
    REQUIRE( move_list.size() == 2 );

    std::vector<Move> moves;
    for (auto move : move_list)
        moves.push_back (move);

    std::vector expected = {
        move_parse ("e4 d4"),
        move_parse ("d2 d1")
    };
    REQUIRE( moves == expected );
}

MoveList copy_moves_and_ptr (const Move **ptr)
{
    Board board;
    MoveList moves = generate_moves (board, Color::White);
//    std::cout << "Moves first" << &moves.get_my_moves()[0] << "\n";
    *ptr = moves.data ();
    return moves;
}

TEST_CASE( "Returning move list moves ptr" )
{
    const Move *ptr;
    MoveList result = copy_moves_and_ptr (&ptr);
//    std::cout << "Moves first" << &result.get_my_moves()[0] << "\n";

    REQUIRE( result.data() == ptr );
    REQUIRE( result.size() > 0 );
}

TEST_CASE("Copying move list")
{
    MoveList first_move_list { Color::White, { "e4 e5", "d7 d6", "a8 b8" } };
    MoveList copy = first_move_list;

    REQUIRE( first_move_list.size() == copy.size() );
    REQUIRE( first_move_list == copy );
    REQUIRE( to_string (first_move_list) == to_string (copy) );
}

TEST_CASE( "Appending a move" )
{
    MoveList list;

    list.push_back (move_parse ("e4 e5"));
    list.push_back (move_parse ("d7 d5"));

    REQUIRE( list.size () == 2 );
}