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
    MoveListAllocator allocator;

    SUBCASE( "moving uncached into cached" )
    {
        MoveList uncached_list = MoveList::uncached ();

        uncached_list.push_back (move_parse ("e4 d4"));
        uncached_list.push_back (move_parse ("d2 d1"));

        MoveList cached { &allocator };

        REQUIRE( cached.allocator () != nullptr );
        cached = std::move (uncached_list);
        REQUIRE( cached.allocator () == nullptr );

        REQUIRE( uncached_list.ptr () == nullptr ); // NOLINT(bugprone-use-after-move)
        REQUIRE( cached.ptr () != nullptr );

        auto ptr = cached.begin ();
        REQUIRE( cached.size () == 2 );
        REQUIRE( *ptr++ == move_parse ("e4 d4", Color::Black) );
        REQUIRE( *ptr == move_parse ("d2 d1", Color::White) );
    }

    SUBCASE( "moving cached into uncached" )
    {
        MoveList uncached_list = MoveList::uncached ();

        uncached_list.push_back (move_parse ("e4 d4"));
        uncached_list.push_back (move_parse ("d2 d1"));

        MoveList cached { &allocator };

        cached.push_back (move_parse ("e3 d3"));
        cached.push_back (move_parse ("d3 d1"));

        REQUIRE( uncached_list.allocator () == nullptr );
        uncached_list = std::move (cached);
        REQUIRE( uncached_list.allocator () != nullptr );

        REQUIRE( cached.ptr () == nullptr ); // NOLINT(bugprone-use-after-move)
        REQUIRE( uncached_list.ptr () != nullptr );

        auto ptr = uncached_list.begin ();
        REQUIRE( uncached_list.size () == 2 );
        REQUIRE( *ptr++ == move_parse ("e3 d3", Color::Black) );
        REQUIRE( *ptr == move_parse ("d3 d1", Color::White) );
    }
}

TEST_CASE( "Swapping lists" )
{
    MoveListAllocator allocator;

    SUBCASE( "Swapping two simple lists" )
    {
        MoveList first = { Color::White, { "e2 d4", "a8 a1"} };
        MoveList second = { Color::Black, { "d7 d5", "f1 c4" } };

        std::swap (first, second);

        auto first_string = to_string (first);
        auto second_string = to_string (second);
        std::cout << "first_string: " << first_string << "\n";
        auto second_expected = string { "{ e2 d4, a8 a1 }" };
        auto first_expected = string { "{ d7 d5", "f1 c4 }" };
        bool first_equals =  first_expected == first_string;
        bool second_equals = second_expected == second_string;

        REQUIRE( first_equals );
        REQUIRE( second_equals );
    }
}