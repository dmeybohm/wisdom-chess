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
        moveParse ("e4 d4"), moveParse ("d2 d1")
    };
    REQUIRE( moves == expected );
}

MoveList copy_moves_and_ptr (const Move **ptr, MoveGenerator& generator)
{
    Board board;
    MoveList moves = generator.generateAllPotentialMoves (board, Color::White);
    return moves;
}

TEST_CASE( "Converting moves to a string" )
{
    MoveList list = { Color::White, { "e2 d4", "a8 a1"} };
    std::string as_string = asString (list);
    REQUIRE( "{ [e2 d4] [a8 a1] }" == as_string );
}

TEST_CASE( "Returning move list moves ptr" )
{
    const Move* ptr;
    MoveGenerator move_generator;
    MoveList result = copy_moves_and_ptr (&ptr, move_generator);
//    std::cout << "Moves first" << &result.get_my_moves()[0] << "\n";

    REQUIRE( result.size() > 0 );
}

TEST_CASE( "Moving move list pointer" )
{
    MoveList initial {Color::Black, {"e4 d4", "d2 d1"}};
    MoveList moved = std::move (initial);

    auto ptr = moved.begin();
    REQUIRE( moved.size() == 2 );
    REQUIRE( *ptr++ == moveParse ("e4 d4", Color::Black) );
    REQUIRE( *ptr == moveParse ("d2 d1", Color::White) );
}

TEST_CASE( "Appending a move" )
{
    MoveList list;

    list.append (moveParse ("e4 e5"));
    list.append (moveParse ("d7 d5"));

    REQUIRE( list.size() == 2 );
}

TEST_CASE( "Moving uncached list" )
{
    SUBCASE( "moving uncached into cached" )
    {
        MoveList uncached_list;

        uncached_list.append (moveParse ("e4 d4"));
        uncached_list.append (moveParse ("d2 d1"));

        MoveList cached;
        cached = std::move (uncached_list);

        auto ptr = cached.begin();
        REQUIRE( cached.size() == 2 );
        REQUIRE( *ptr++ == moveParse ("e4 d4", Color::Black) );
        REQUIRE( *ptr == moveParse ("d2 d1", Color::White) );
    }

    SUBCASE( "moving cached into uncached" )
    {
        MoveList uncached_list;

        uncached_list.append (moveParse ("e4 d4"));
        uncached_list.append (moveParse ("d2 d1"));

        MoveList cached;

        cached.append (moveParse ("e3 d3"));
        cached.append (moveParse ("d3 d1"));

        uncached_list = std::move (cached);

        auto ptr = uncached_list.begin();
        REQUIRE( uncached_list.size() == 2 );
        REQUIRE( *ptr++ == moveParse ("e3 d3", Color::Black) );
        REQUIRE( *ptr == moveParse ("d3 d1", Color::White) );
    }
}

TEST_CASE( "Swapping lists" )
{
    SUBCASE( "Swapping two simple lists" )
    {
        MoveList first = { Color::White, { "e2 d4", "a8 a1"} };
        MoveList second = { Color::Black, { "d7 d5", "f1 c4" } };

        std::swap (first, second);

        auto first_string = asString (first);
        auto second_string = asString (second);

        CHECK( "{ [d7 d5] [f1 c4] }" == first_string );
        CHECK( "{ [e2 d4] [a8 a1] }" == second_string );
    }

    SUBCASE( "Swapping two move lists with different allocators" )
    {
        MoveList first;
        MoveList second = { Color::Black, { "d7 d5", "f1 c4" } };

        first.append (moveParse ("e2 d4", Color::White));
        first.append (moveParse ("a8 a1", Color::Black));

        std::swap (first, second);

        auto first_string = asString (first);
        auto second_string = asString (second);

        CHECK( "{ [d7 d5] [f1 c4] }" == first_string );
        CHECK( "{ [e2 d4] [a8 a1] }" == second_string );
    }
}