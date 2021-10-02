#include "tests.hpp"
#include "perft.hpp"
#include "board_builder.hpp"
#include "board.hpp"
#include "str.hpp"

#include <iostream>

using std::string;
using std::vector;
using wisdom::BoardBuilder;
using wisdom::Color;
using wisdom::Piece;

TEST_CASE( "move list")
{
    BoardBuilder builder;

    builder.add_piece ("a7", Color::White, Piece::Pawn);
    builder.add_piece ("e8", Color::Black, Piece::King);
    builder.add_piece ("h8", Color::Black, Piece::Rook);
    builder.add_piece ("e1", Color::White, Piece::King);
    builder.add_piece ("a1", Color::White, Piece::Rook);

    builder.add_piece ("e5", Color::White, Piece::Pawn);
    builder.add_piece ("d7", Color::Black, Piece::Pawn);

    auto board = builder.build();

    string perft_move_list = wisdom::join({
        "e1c1", // white queen-side castle
        "d7d5",
        "e5d6", // white en-passant
        "e8g8", // black king-side castle
        "a7a8Q", // white promotion
    }, "\n");

    auto wisdom_move_list = wisdom::perft::to_move_list (*board, Color::White, perft_move_list);
    auto converted = wisdom_move_list.to_string ();

    REQUIRE( converted == "{ [O-O-O] [d7 d5] [e5 d6 ep] [O-O] [a7 a8(Q)] }" );
}

TEST_CASE( "to_perft_move" )
{
    SUBCASE( "Castling move" )
    {
        auto white_castle = move_parse ("o-o", Color::White);
        auto black_castle = move_parse ("o-o-o", Color::Black);
        auto white_result = wisdom::perft::to_perft_move (white_castle, Color::White);
        auto black_result = wisdom::perft::to_perft_move (black_castle, Color::Black);

        CHECK( white_result == "e1g1" );
        CHECK( black_result == "e8c8" );
    }

    SUBCASE( "Promoted move" )
    {
        auto promote_bishop = wisdom::move_parse ("e7e8(B)");
        auto promote_result = wisdom::perft::to_perft_move (promote_bishop, Color::White);

        CHECK( promote_result == "e7e8B" );
    }

    SUBCASE( "En-passant" )
    {
        auto en_passant = wisdom::move_parse ("e5 d6 ep", Color::White);
        auto result = wisdom::perft::to_perft_move (en_passant, Color::White);

        CHECK( result == "e5d6" );
    }

    SUBCASE( "Normal move" )
    {
        auto normal = wisdom::move_parse ("a7 a5");
        auto result = wisdom::perft::to_perft_move (normal, Color::Black);

        CHECK( result == "a7a5" );
    }
}