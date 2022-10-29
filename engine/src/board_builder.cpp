#include "board_builder.hpp"

#include "board.hpp"
#include "move.hpp"
#include "coord.hpp"

namespace wisdom
{
    auto BoardBuilder::from_default_position () -> BoardBuilder
    {
        array<Piece, Num_Columns> back_rank = {
                Piece::Rook, Piece::Knight, Piece::Bishop, Piece::Queen, Piece::King,
                Piece::Bishop, Piece::Knight, Piece::Rook,
        };

        array<Piece, Num_Columns> pawns = {
                Piece::Pawn, Piece::Pawn, Piece::Pawn, Piece::Pawn, Piece::Pawn,
                Piece::Pawn, Piece::Pawn, Piece::Pawn,
        };

        auto result = BoardBuilder {};
        result.add_row_of_same_color (0, Color::Black, Default_Piece_Row);
        result.add_row_of_same_color_and_piece (1, Color::Black, Piece::Pawn);
        result.add_row_of_same_color_and_piece (6, Color::White, Piece::Pawn);
        result.add_row_of_same_color (7, Color::White, Default_Piece_Row);

        result.set_castling (Color::White, CastlingIneligible::None);
        result.set_castling (Color::Black, CastlingIneligible::None);

        return result;
    }

    void BoardBuilder::add_piece (const string& coord_str, Color who, Piece piece_type)
    {
        if (coord_str.size () != 2)
            throw BoardBuilderError ("Invalid coordinate string!");

        Coord algebraic = coord_parse (coord_str);

        add_piece (Row (algebraic), Column (algebraic), who, piece_type);
    }

    auto BoardBuilder::calculate_castle_state_from_position (Color who) const
        -> CastlingIneligibility
    {
        auto row = castling_row_for_color (who);
        int8_t king_col = King_Column;

        CastlingIneligibility state = CastlingIneligible::None;
        ColoredPiece prospective_king = piece_at (make_coord (row, king_col));
        ColoredPiece prospective_queen_rook = piece_at (make_coord (row, 0));
        ColoredPiece prospective_king_rook = piece_at (make_coord (row, 7));

        if (piece_type (prospective_king) != Piece::King ||
            piece_color (prospective_king) != who ||
            piece_type (prospective_queen_rook) != Piece::Rook ||
            piece_color (prospective_queen_rook) != who)
        {
            state |= CastlingIneligible::Queenside;
        }
        if (piece_type (prospective_king) != Piece::King ||
            piece_color (prospective_king) != who ||
            piece_type (prospective_king_rook) != Piece::Rook ||
            piece_color (prospective_king_rook) != who)
        {
            state |= CastlingIneligible::Kingside;
        }

        return state;
    }

    void BoardBuilder::add_piece (int row, int col, Color who, Piece piece_type)
    {
        if (row < 0 || row >= Num_Rows)
            throw BoardBuilderError ("Invalid row!");

        if (col < 0 || col >= Num_Columns)
            throw BoardBuilderError ("Invalid column!");

        if (piece_type == Piece::None)
            return;

        auto coord = make_coord (row, col);
        my_squares[coord_index (coord)] = ColoredPiece::make (who, piece_type);

        if (piece_type == Piece::King)
            my_king_positions[color_index (who)] = coord;
    }

    void BoardBuilder::add_pieces (Color who, const vector<CoordAndPiece> &pieces)
    {
        for (auto&& it : pieces)
            add_piece (it.coord, who, it.piece_type);
    }

    void BoardBuilder::add_row_of_same_color_and_piece (int row, Color who, Piece piece_type)
    {
        for (int col = 0; col < Num_Columns; col++)
            add_piece (row, col, who, piece_type);
    }

    void BoardBuilder::add_row_of_same_color_and_piece (const string& coord_str, Color who,
                                                        Piece piece_type)
    {
        Coord coord = coord_parse (coord_str);

        for (int col = 0; col < Num_Columns; col++)
            add_piece (Row (coord), col, who, piece_type);
    }

    void BoardBuilder::add_row_of_same_color (int row, Color who,
                                              const PieceRow& piece_types)
    {
        for (auto col = 0; col < Num_Columns; col++)
            add_piece (row, col, who, piece_types[col]);
    }

    void BoardBuilder::add_row_of_same_color (const string& coord_str, Color who,
                                              const PieceRow& piece_types)
    {
        Coord coord = coord_parse (coord_str);

        for (auto col = 0; col < Num_Columns; col++)
            add_piece (Row (coord), col, who, piece_types[col]);
    }

    void BoardBuilder::set_current_turn (Color who)
    {
        my_current_turn = who;
    }

    void BoardBuilder::set_en_passant_target (Color vulnerable_color, const string& coord_str)
    {
        my_en_passant_targets[color_index (vulnerable_color)] = coord_parse (coord_str);
    }

    void BoardBuilder::set_castling (Color who, CastlingIneligibility state)
    {
        my_castle_states[color_index (who)] = state;
    }

    void BoardBuilder::set_half_moves_clock (int new_half_moves_clock)
    {
        my_half_moves_clock = new_half_moves_clock;
    }

    void BoardBuilder::set_full_moves (int new_full_moves)
    {
        my_full_moves = new_full_moves;
    }
}
