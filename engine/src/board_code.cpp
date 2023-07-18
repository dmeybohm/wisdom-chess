#include "board_code.hpp"
#include "board.hpp"
#include "coord.hpp"
#include "board_builder.hpp"

#include <ostream>

namespace wisdom
{
    BoardCode::BoardCode (const Board& board)
    {
        for (auto coord : Board::all_coords ())
        {
            ColoredPiece piece = board.pieceAt (coord);
            this->add_piece (coord, piece);
        }

        auto current_turn = board.get_current_turn ();
        set_current_turn (current_turn);

        auto en_passant_targets = board.get_en_passant_targets ();
        if (en_passant_targets[Color_Index_White] != No_En_Passant_Coord)
        {
            set_en_passant_target (Color::White, en_passant_targets[Color_Index_White]);
        }
        else if (en_passant_targets[Color_Index_Black] != No_En_Passant_Coord)
        {
            set_en_passant_target (Color::Black, en_passant_targets[Color_Index_Black]);
        }
        else
        {
            // this clears both targets:
            set_en_passant_target (Color::White, No_En_Passant_Coord);
        }

        set_castle_state (Color::White, board.get_castling_eligibility(Color::White));
        set_castle_state (Color::Black, board.get_castling_eligibility(Color::Black));
    }

    auto BoardCode::from_board (const Board& board) -> BoardCode
    {
        return BoardCode { board };
    }

    auto BoardCode::from_board_builder (const BoardBuilder& builder) -> BoardCode
    {
        auto result = BoardCode {};

        for (auto coord : CoordIterator {})
        {
            ColoredPiece piece = builder.piece_at (coord);
            result.add_piece (coord, piece);
        }

        auto current_turn = builder.get_current_turn ();
        result.set_current_turn (current_turn);

        auto en_passant_targets = builder.get_en_passant_targets ();
        if (en_passant_targets[Color_Index_White] != No_En_Passant_Coord)
        {
            result.set_en_passant_target (Color::White, en_passant_targets[Color_Index_White]);
        }
        else if (en_passant_targets[Color_Index_Black] != No_En_Passant_Coord)
        {
            result.set_en_passant_target (Color::Black, en_passant_targets[Color_Index_Black]);
        }
        else
        {
            result.set_en_passant_target (Color::White, No_En_Passant_Coord);
        }

        result.set_castle_state (Color::White, builder.get_castle_state (Color::White));
        result.set_castle_state (Color::Black, builder.get_castle_state (Color::Black));
        return result;
    }

    auto BoardCode::from_default_position () -> BoardCode
    {
        BoardBuilder builder = BoardBuilder::from_default_position ();
        return BoardCode::from_board_builder (builder);
    }

    auto BoardCode::from_empty_board () -> BoardCode
    {
        return {};
    }

    void BoardCode::apply_move (const Board& board, Move move)
    {
        Coord src = move.get_src ();
        Coord dst = move.get_dst ();

        ColoredPiece src_piece = board.pieceAt (src);

        Piece src_piece_type = piece_type (src_piece);
        Color src_piece_color = piece_color (src_piece);

        if (move.is_castling ())
        {
            int src_col, dst_col;
            int row;

            if (move.is_castling_on_kingside ())
            {
                dst_col = Kingside_Castled_Rook_Column;
                src_col = Last_Column;
            }
            else
            {
                dst_col = Queenside_Castled_Rook_Column;
                src_col = 0;
            }
            row = src_piece_color == Color::White ? Last_Row : First_Row;

            Coord rook_src = make_coord (row, src_col);
            ColoredPiece rook = ColoredPiece::make (src_piece_color, Piece::Rook);
            remove_piece (rook_src);
            add_piece (make_coord (row, dst_col), rook);
        }
        else if (move.is_en_passant ())
        {
            // subtract horizontal pawn and add no piece there:
            Coord taken_pawn_coord = en_passant_taken_pawn_coord (src, dst);
            remove_piece (taken_pawn_coord);
        }

        remove_piece (src);
        add_piece (dst, src_piece);

        if (move.is_promoting ())
        {
            assert (src_piece_type == Piece::Pawn);
            add_piece (dst, move.get_promoted_piece ());
        }
    }

    auto BoardCode::count_ones () const -> std::size_t
    {
        string str = my_pieces.to_string ();
        return std::count (str.begin (), str.end (), '1');
    }

    auto operator<< (std::ostream& os, const BoardCode& code) -> std::ostream&
    {
        os << "{ bits: " << code.my_pieces << ", ancillary: " << code.my_ancillary << " }";
        return os;
    }

}
