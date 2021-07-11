
#include "board_code.hpp"
#include "board.hpp"
#include "coord.hpp"

#include <ostream>

namespace wisdom
{
    BoardCode::BoardCode (const Board &board)
    {
        for (auto coord : All_Coords_Iterator)
        {
            ColoredPiece piece = piece_at (board, coord);
            this->add_piece (coord, piece);
        }
    }

    void BoardCode::apply_move (const Board &board, Move move)
    {
        Coord src = move_src (move);
        Coord dst = move_dst (move);

        ColoredPiece src_piece = piece_at (board, src);

        Piece src_piece_type = piece_type (src_piece);
        Color src_piece_color = piece_color (src_piece);

        if (is_castling_move (move))
        {
            int src_col, dst_col;
            int row;

            if (is_castling_move_on_king_side (move))
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

            Coord rook_src = make_coord (row, src_col);
            ColoredPiece rook = make_piece (src_piece_color, Piece::Rook);
            remove_piece (rook_src);
            add_piece (make_coord (row, dst_col), rook);
        }
        else if (is_en_passant_move (move))
        {
            // subtract horizontal pawn and add no piece there:
            Coord taken_pawn_coord = en_passant_taken_pawn_coord (src, dst);
            remove_piece (taken_pawn_coord);
        }

        remove_piece (src);
        add_piece (dst, src_piece);

        if (is_promoting_move (move))
        {
            assert (src_piece_type == Piece::Pawn);
            add_piece (dst, move_get_promoted_piece (move));
        }
    }

    void BoardCode::unapply_move (const Board &board,
                                  Move move, UndoMove undo_state)
    {
        Coord src = move_src (move);
        Coord dst = move_dst (move);

        ColoredPiece src_piece = piece_at (board, dst);

        Color src_piece_color = piece_color (src_piece);
        Color opponent_color = color_invert (src_piece_color);

        if (is_promoting_move (move))
        {
            src_piece = make_piece (src_piece_color, Piece::Pawn);
        }

        if (is_castling_move (move))
        {
            int src_col, dst_col;
            int row;

            if (is_castling_move_on_king_side (move))
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

            Coord rook_src = make_coord (row, src_col);
            ColoredPiece rook = make_piece (src_piece_color, Piece::Rook);
            add_piece (rook_src, rook);
            remove_piece (make_coord (row, dst_col));
        }

        add_piece (src, src_piece);
        remove_piece (dst);

        if (is_capture_move (move))
        {
            ColoredPiece captured = captured_material (undo_state, opponent_color);
            add_piece (dst, captured);
        }

        if (is_en_passant_move (move))
        {
            ColoredPiece captured_pawn = make_piece (opponent_color, Piece::Pawn);
            Coord taken_pawn_coord = en_passant_taken_pawn_coord (src, dst);
            add_piece (taken_pawn_coord, captured_pawn);
        }
    }

    std::size_t BoardCode::count_ones ()
    {
        std::string str = bits.to_string ();
        return std::count (str.begin (), str.end (), '1');
    }

    std::ostream &operator<< (std::ostream &os, const BoardCode &code)
    {
        os << "{ bits: " << code.bits << " }";
        return os;
    }
}