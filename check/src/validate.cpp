#include "board.h"
#include "move.h"
#include "validate.h"

static void check_it (struct board &board, Color who, move_t mv, int expr)
{
    if (!expr)
    {
        printf ("move considering: %s \n", to_string(mv).c_str());
        board.dump ();
        abort ();
    }
}

static void validate_castle (struct board &board, castle_state_t state, Color who, move_t mv)
{
    // check positions of the pieces:
    int8_t row = who == Color::White ? 7 : 0;
    int8_t col = state == CASTLE_QUEENSIDE ? 0 : 7;
    piece_t supposed_king = piece_at (board, row, 4);
    piece_t supposed_rook = piece_at (board, row, col);
    if (able_to_castle (board, who, state))
    {
        check_it(board, who, mv, piece_type (supposed_king) == Piece::King );
        check_it(board, who, mv, piece_type (supposed_rook) == Piece::Rook );
        check_it(board, who, mv, piece_color (supposed_king) == who );
        check_it(board, who, mv, piece_color (supposed_rook) == who );
        check_it( board, who, mv, coord_equals (king_position(board, who), coord_create(row, 4)) );
    }
}

void validate_castle_state (struct board &board, move_t mv)
{
#ifndef NDEBUG
    validate_castle (board, CASTLE_QUEENSIDE, Color::White, mv);
    validate_castle (board, CASTLE_KINGSIDE, Color::White, mv);
    validate_castle (board, CASTLE_QUEENSIDE, Color::Black, mv);
    validate_castle (board, CASTLE_KINGSIDE, Color::Black, mv);
#endif
}