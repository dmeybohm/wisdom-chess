#include "board.h"
#include "move.h"
#include "validate.h"

static void check_it (struct board *board, color_t who, move_t mv, int expr)
{
    if (!expr)
    {
        printf ("move considering: %s \n", move_str(mv));
        board_dump (board);
        abort ();
    }
}

void validate_castle (struct board *board, castle_state_t state, color_t who, move_t mv)
{
    // check positions of the pieces:
    uint8_t row = who == COLOR_WHITE ? 7 : 0;
    uint8_t col = state == CASTLE_QUEENSIDE ? 0 : 7;
    piece_t supposed_king = PIECE_AT (board, row, 4);
    piece_t supposed_rook = PIECE_AT (board, row, col);
    if (able_to_castle (board, who, state))
    {
        check_it( board, who, mv, PIECE_TYPE(supposed_king) == PIECE_KING );
        check_it( board, who, mv, PIECE_TYPE(supposed_rook) == PIECE_ROOK );
        check_it( board, who, mv, PIECE_COLOR(supposed_king) == who );
        check_it( board, who, mv, PIECE_COLOR(supposed_rook) == who );
        check_it( board, who, mv, coord_equals (king_position(board, who), coord_create(row, 4)) );
    }
}

void validate_castle_state (struct board *board, move_t mv)
{
    validate_castle (board, CASTLE_QUEENSIDE, COLOR_WHITE, mv);
    validate_castle (board, CASTLE_KINGSIDE, COLOR_WHITE, mv);
    validate_castle (board, CASTLE_QUEENSIDE, COLOR_BLACK, mv);
    validate_castle (board, CASTLE_KINGSIDE, COLOR_BLACK, mv);
}