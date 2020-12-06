
#include "board_code.hpp"
#include "board.h"

void board_code::apply_move (const struct board &board, move_t move)
{
    coord_t src = move_src (move);
    coord_t dst = move_dst (move);

    piece_t src_piece = piece_at (board, src);
    piece_t dst_piece = piece_at (board, dst);

    Piece src_piece_type = piece_type (src_piece);
    Color src_piece_color = piece_color (src_piece);

    if (is_castling_move(move))
    {
        int8_t src_col, dst_col;
        int8_t row;

        if (is_castling_move_on_king_side(move))
        {
            dst_col = King_Castled_Rook_Column;
            src_col = Last_Column;
        }
        else
        {
            dst_col = Queen_Castled_Rook_Column;
            src_col = 0;
        }
        row = src_piece_color == Color::White ? Last_Row : First_Row;

        coord_t rook_src = make_coord (row, src_col);
        piece_t rook = make_piece (src_piece_color, Piece::Rook);
        remove_piece (rook_src);
        add_piece (make_coord (row, dst_col), rook);
    }
    else if (is_en_passant_move(move))
    {
        // subtract horizontal pawn and add no piece there:
        coord_t taken_pawn_coord = en_passant_taken_pawn_coord (src, dst);
        remove_piece (taken_pawn_coord);
    }

    remove_piece (src);
    add_piece (dst, dst_piece);

    if (is_promoting_move(move))
    {
        assert (src_piece_type == Piece::Pawn);
        add_piece (dst, move_get_promoted_piece (move));
    }
}

void board_code::unapply_move (const struct board &board,
                               move_t move, undo_move_t undo_state)
{

}