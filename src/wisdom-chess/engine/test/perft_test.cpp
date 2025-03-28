#include <iostream>

#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/str.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"
#include "wisdom-chess/engine/generate.hpp"

#include "wisdom-chess-tests.hpp"
#include "wisdom-chess-perft.hpp"

using std::string;
using std::vector;
using wisdom::Board;
using wisdom::BoardBuilder;
using wisdom::Color;
using wisdom::Piece;
using wisdom::FenParser;

TEST_CASE( "Perft move list")
{
    BoardBuilder builder;

    builder.addPiece ("a7", Color::White, Piece::Pawn);
    builder.addPiece ("e8", Color::Black, Piece::King);
    builder.addPiece ("h8", Color::Black, Piece::Rook);
    builder.addPiece ("e1", Color::White, Piece::King);
    builder.addPiece ("a1", Color::White, Piece::Rook);

    builder.addPiece ("e5", Color::White, Piece::Pawn);
    builder.addPiece ("d7", Color::Black, Piece::Pawn);

    auto board = Board { builder };

    string perft_move_list = wisdom::join({
        "e1c1", // white queen-side castle
        "d7d5",
        "e5d6", // white en-passant
        "e8g8", // black king-side castle
        "a7a8Q", // white promotion
    }, " ");

    auto wisdom_move_list = wisdom::perft::toMoveList (board, Color::White, perft_move_list);
    auto converted = wisdom_move_list.asString();

    REQUIRE( converted == "{ [O-O-O] [d7 d5] [e5 d6 ep] [O-O] [a7 a8(Q)] }" );
}

TEST_CASE( "toPerftMove" )
{
    SUBCASE( "Castling move" )
    {
        auto white_castle = moveParse ("o-o", Color::White);
        auto black_castle = moveParse ("o-o-o", Color::Black);
        auto white_result = wisdom::perft::toPerftMove (white_castle, Color::White);
        auto black_result = wisdom::perft::toPerftMove (black_castle, Color::Black);

        CHECK( white_result == "e1g1" );
        CHECK( black_result == "e8c8" );
    }

    SUBCASE( "Promoted move" )
    {
        auto promote_bishop = wisdom::moveParse ("e7e8(B)", Color::White);
        auto promote_result = wisdom::perft::toPerftMove (promote_bishop, Color::White);

        CHECK( promote_result == "e7e8B" );
    }

    SUBCASE( "En-passant" )
    {
        auto en_passant = wisdom::moveParse ("e5 d6 ep", Color::White);
        auto result = wisdom::perft::toPerftMove (en_passant, Color::White);

        CHECK( result == "e5d6" );
    }

    SUBCASE( "Normal move" )
    {
        auto normal = wisdom::moveParse ("a7 a5");
        auto result = wisdom::perft::toPerftMove (normal, Color::Black);

        CHECK( result == "a7a5" );
    }
}
