#include "doctest/doctest.h"
#include "board_builder.hpp"
#include "board.hpp"
#include "check.hpp"

using namespace wisdom;

TEST_CASE("is_king_threatened works for bishop, rook, and king")
{
    BoardBuilder builder;

    builder.add_piece ("a8", Color::Black, Piece::King);
    builder.add_piece ("a1", Color::White, Piece::King);

    builder.add_piece ("c3", Color::Black, Piece::Bishop);
    builder.add_piece ("d4", Color::White, Piece::Rook);

    Board board = builder.build();
    int white_king_threatened[Num_Rows][Num_Columns] = {
            { 0, 1, 0, 0, 0, 0, 0, 0 },
            { 1, 1, 0, 0, 0, 0, 0, 0 },
            { 0, 0, 0, 0, 0, 0, 0, 0 },
            { 1, 0, 0, 0, 0, 0, 0, 0 },
            { 0, 1, 0, 1, 0, 0, 0, 0 },
            { 0, 0, 0, 0, 0, 0, 0, 0 },
            { 0, 1, 0, 1, 0, 0, 0, 0 },
            { 1, 0, 0, 0, 1, 0, 0, 0 },
    };

    int black_king_threatened[Num_Rows][Num_Columns] = {
            { 0, 0, 0, 1, 0, 0, 0, 0 },
            { 0, 0, 0, 1, 0, 0, 0, 0 },
            { 0, 0, 0, 1, 0, 0, 0, 0 },
            { 0, 0, 0, 1, 0, 0, 0, 0 },
            { 1, 1, 1, 0, 1, 1, 1, 1 },
            { 0, 0, 0, 1, 0, 0, 0, 0 },
            { 1, 1, 0, 1, 0, 0, 0, 0 },
            { 0, 1, 0, 1, 0, 0, 0, 0 },
    };

    for (auto row = 0; row < 8; row++)
    {
        for (auto col = 7; col < 8; col++)
        {
//            INFO("White king coordinate is ", row, " ", col);
            REQUIRE( is_king_threatened (board, Color::White, row, col) == (bool)white_king_threatened[row][col] );
//            INFO("Black king coordinate is ", row, " ", col);
            REQUIRE( is_king_threatened (board, Color::Black, row, col) == (bool)black_king_threatened[row][col] );
        }
    }

}
