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

MoveList copy_moves_and_ptr (const Move **ptr, MoveGenerator& generator)
{
    Board board;
    MoveList moves = generator.generate_all_potential_moves (board, Color::White);
//    std::cout << "Moves first" << &moves.get_my_moves()[0] << "\n";
    *ptr = moves.data ();
    return moves;
}

TEST_CASE( "Returning move list moves ptr" )
{
    const Move* ptr;
    MoveGenerator move_generator;
    MoveList result = copy_moves_and_ptr (&ptr, move_generator);
//    std::cout << "Moves first" << &result.get_my_moves()[0] << "\n";

    REQUIRE( result.data() == ptr );
    REQUIRE( result.size() > 0 );
}

TEST_CASE( "Moving move list pointer" )
{
    MoveList initial {Color::Black, {"e4 d4", "d2 d1"}};
    MoveList moved = std::move (initial);

    REQUIRE( initial.ptr () == nullptr ); // NOLINT(bugprone-use-after-move)
    REQUIRE( moved.ptr () != nullptr );

    auto ptr = moved.begin ();
    REQUIRE( moved.size () == 2 );
    REQUIRE( *ptr++ == move_parse ("e4 d4", Color::Black) );
    REQUIRE( *ptr == move_parse ("d2 d1", Color::White) );
}

TEST_CASE( "Appending a move" )
{
    MoveList list = MoveList::uncached ();

    list.push_back (move_parse ("e4 e5"));
    list.push_back (move_parse ("d7 d5"));

    REQUIRE( list.size () == 2 );
}

TEST_CASE( "Moving uncached list" )
{
    MoveList uncached_list = MoveList::uncached ();

    uncached_list.push_back (move_parse ("e4 d4"));
    uncached_list.push_back (move_parse ("d2 d1"));

    MoveList moved = std::move (uncached_list);

    REQUIRE( uncached_list.ptr () == nullptr ); // NOLINT(bugprone-use-after-move)
    REQUIRE( moved.ptr () != nullptr );

    auto ptr = moved.begin ();
    REQUIRE( moved.size () == 2 );
    REQUIRE( *ptr++ == move_parse ("e4 d4", Color::Black) );
    REQUIRE( *ptr == move_parse ("d2 d1", Color::White) );
}
