#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/evaluate.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "isKingThreatened works for bishop, rook, and king" )
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

    for (auto row = 0; row < Num_Rows; row++)
    {
        for (auto col = 0; col < Num_Columns; col++)
        {
            auto king_row = narrow<int8_t> (row);
            auto king_col = narrow<int8_t> (col);

            INFO( "King coordinate is row ", row, " column ", col );
            CHECK( isKingThreatened (board, Color::White, king_row, king_col) == (bool)white_king_threatened[row][col] );
            CHECK( isKingThreatened (board, Color::Black, king_row, king_col) == (bool)black_king_threatened[row][col] );
        }
    }

}
