#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/evaluate.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE("isKingThreatened works for bishop, rook, and king")
{
    BoardBuilder builder;

    builder.addPiece ("a8", Color::Black, Piece::King);
    builder.addPiece ("a1", Color::White, Piece::King);

    builder.addPiece ("c3", Color::Black, Piece::Bishop);
    builder.addPiece ("d4", Color::White, Piece::Rook);

    auto board = Board { builder };
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
            REQUIRE( isKingThreatened (board, Color::White, row, col) == (bool)white_king_threatened[row][col] );
//            INFO("Black king coordinate is ", row, " ", col);
            REQUIRE( isKingThreatened (board, Color::Black, row, col) == (bool)black_king_threatened[row][col] );
        }
    }

}
