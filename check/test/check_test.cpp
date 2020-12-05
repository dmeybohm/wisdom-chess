#include "catch.hpp"
#include "board_builder.hpp"
#include <iostream>

#include "board.h"
#include "check.h"

TEST_CASE("is_king_threatened works for bishop, rook, and king", "[check]")
{
    board_builder builder;

    builder.add_piece("a8", Color::Black, Piece::King);
    builder.add_piece("a1", Color::White, Piece::King);

    builder.add_piece("c3", Color::Black, Piece::Bishop);
    builder.add_piece("d4", Color::White, Piece::Rook);

    struct board board = builder.build();
    int white_king_threatened[NR_ROWS][NR_COLUMNS] = {
            { 0, 1, 0, 0, 0, 0, 0, 1 },
            { 1, 1, 0, 0, 0, 0, 1, 0 },
            { 0, 0, 0, 0, 0, 1, 0, 0 },
            { 1, 0, 0, 0, 1, 0, 0, 0 },
            { 0, 1, 0, 1, 0, 0, 0, 0 },
            { 0, 0, 0, 0, 0, 0, 0, 0 },
            { 0, 1, 0, 1, 0, 0, 0, 0 },
            { 1, 0, 0, 0, 1, 0, 0, 0 },
    };

    int black_king_threatened[NR_ROWS][NR_COLUMNS] = {
            { 0, 0, 0, 1, 0, 0, 0, 0 },
            { 0, 0, 0, 1, 0, 0, 0, 0 },
            { 0, 0, 0, 1, 0, 0, 0, 0 },
            { 0, 0, 0, 1, 0, 0, 0, 0 },
            { 1, 1, 1, 0, 1, 1, 1, 1 },
            { 0, 0, 0, 1, 0, 0, 0, 0 },
            { 1, 1, 0, 1, 0, 0, 0, 0 },
            { 0, 1, 0, 1, 0, 0, 0, 0 },
    };

    auto row = GENERATE(7);
    auto col = GENERATE(7);

    REQUIRE( is_king_threatened (board, Color::White, row, col) == (bool)white_king_threatened[row][col] );
    REQUIRE( is_king_threatened (board, Color::Black, row, col) == (bool)black_king_threatened[row][col] );

//    REQUIRE( is_king_threatened (board, Color::White, 7, 0) == (bool)white_king_threatened[7][0] );
//    REQUIRE( is_king_threatened (board, Color::White, 7, 1) == (bool)white_king_threatened[7][1] );
//    REQUIRE( is_king_threatened (board, Color::White, 7, 2) == (bool)white_king_threatened[7][2] );
//    REQUIRE( is_king_threatened (board, Color::White, 7, 3) == (bool)white_king_threatened[7][3] );
//    REQUIRE( is_king_threatened (board, Color::White, 7, 4) == (bool)white_king_threatened[7][4] );
//    REQUIRE( is_king_threatened (board, Color::White, 7, 5) == (bool)white_king_threatened[7][5] );
//    REQUIRE( is_king_threatened (board, Color::White, 7, 6) == (bool)white_king_threatened[7][6] );
//    REQUIRE( is_king_threatened (board, Color::White, 7, 7) == (bool)white_king_threatened[7][7] );
//
//    REQUIRE( is_king_threatened (board, Color::Black, 7, 0) == (bool)black_king_threatened[7][0] );
//    REQUIRE( is_king_threatened (board, Color::Black, 7, 1) == (bool)black_king_threatened[7][1] );
//    REQUIRE( is_king_threatened (board, Color::Black, 7, 2) == (bool)black_king_threatened[7][2] );
//    REQUIRE( is_king_threatened (board, Color::Black, 7, 3) == (bool)black_king_threatened[7][3] );
//    REQUIRE( is_king_threatened (board, Color::Black, 7, 4) == (bool)black_king_threatened[7][4] );
//    REQUIRE( is_king_threatened (board, Color::Black, 7, 5) == (bool)black_king_threatened[7][5] );
//    REQUIRE( is_king_threatened (board, Color::Black, 7, 6) == (bool)black_king_threatened[7][6] );
//    REQUIRE( is_king_threatened (board, Color::Black, 7, 7) == (bool)black_king_threatened[7][7] );
}
