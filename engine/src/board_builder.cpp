#include "board_builder.hpp"

#include "board.hpp"
#include "move.hpp"

namespace wisdom
{
    Coord coord_alg (const std::string &coord_str)
    {
        if (coord_str.size () != 2)
            throw BoardBuilderError ("Invalid coordinate string!");

        int col = char_to_col (coord_str[0]);
        int row = char_to_row (coord_str[1]);

        if (row < 0 || row >= Num_Rows)
            throw BoardBuilderError ("Invalid row!");

        if (col < 0 || col >= Num_Columns)
            throw BoardBuilderError ("Invalid column!");

        return make_coord (row, col);
    }

    void BoardBuilder::add_piece (const std::string &coord_str, Color who, Piece piece_type)
    {
        if (coord_str.size () != 2)
            throw BoardBuilderError ("Invalid coordinate string!");

        Coord algebraic = coord_alg (coord_str);

        this->add_piece (ROW (algebraic), COLUMN (algebraic), who, piece_type);
    }

    void BoardBuilder::add_piece (int8_t row, int8_t col, Color who, Piece piece_type)
    {
        struct BBPieceWithCoordState new_piece {
                .coord = make_coord (row, col),
                .color = who,
                .piece_type = piece_type,
        };

        if (row < 0 || row >= Num_Rows)
            throw BoardBuilderError ("Invalid row!");

        if (col < 0 || col >= Num_Columns)
            throw BoardBuilderError ("Invalid column!");

        this->pieces_with_coords.push_back (new_piece);
    }

    void BoardBuilder::add_pieces (Color who, const std::vector<struct BBPieceCoordStringWithTypeState> &pieces)
    {
        for (auto it : pieces)
            this->add_piece (it.coord, who, it.piece_type);
    }

    void BoardBuilder::add_row_of_same_color_and_piece (int row, Color who, Piece piece_type)
    {
        for (int col = 0; col < Num_Columns; col++)
            this->add_piece (row, col, who, piece_type);
    }

    void BoardBuilder::add_row_of_same_color_and_piece (const std::string &coord_str, Color who, Piece piece_type)
    {
        Coord coord = coord_alg (coord_str);

        for (int col = 0; col < Num_Columns; col++)
            this->add_piece (ROW (coord), col, who, piece_type);
    }

    void BoardBuilder::add_row_of_same_color (int row, Color who, std::vector<Piece> piece_types)
    {
        size_t col = 0;

        for (auto it = piece_types.begin (); it != piece_types.end (); it++, col++)
            this->add_piece (row, col, who, *it);
    }

    void BoardBuilder::add_row_of_same_color (const std::string &coord_str, Color who, std::vector<Piece> piece_types)
    {
        Coord coord = coord_alg (coord_str);
        size_t col = 0;

        for (auto it = piece_types.begin (); it != piece_types.end (); it++, col++)
            this->add_piece (ROW (coord), col, who, *it);
    }

    void BoardBuilder::set_en_passant_target (Color who, const std::string &coord_str)
    {
        struct BBEnPassantState new_state { who, coord_alg (coord_str) };
        this->en_passant_states.push_back (new_state);
    }

    void BoardBuilder::set_castling (Color who, CastlingState state)
    {
        struct BBCastlingState new_state { .player = who, .castle_state = state };
        castle_states.push_back (new_state);
    }

    void BoardBuilder::set_half_moves (int new_half_moves_clock)
    {
        this->half_moves_clock = new_half_moves_clock;
    }

    void BoardBuilder::set_full_moves (int new_full_moves)
    {
        this->full_moves = new_full_moves;
    }

    Board BoardBuilder::build ()
    {
        struct piece_row
        {
            std::vector<Piece> row;
        };

        std::size_t sz = this->pieces_with_coords.size ();

        std::vector<struct piece_row> piece_types { sz };
        std::vector<struct BoardPositions> positions { sz };

        for (size_t i = 0; i < sz; i++)
        {
            struct BBPieceWithCoordState &piece_with_coord = this->pieces_with_coords[i];
            std::vector<Piece> &current_piece_row = piece_types[i].row;

            int col = COLUMN (piece_with_coord.coord);
            int row = ROW (piece_with_coord.coord);

            current_piece_row.assign (Num_Columns, Piece::None);
            current_piece_row[col] = piece_with_coord.piece_type;

            positions[i] = { row, piece_with_coord.color, current_piece_row };
        }

        Board result = Board { positions };

        if (!en_passant_states.empty ())
        {
            for (auto state : en_passant_states)
            {
                ColorIndex index = color_index (state.player);
                result.en_passant_target[index] = state.coord;
            }
        }

        if (!castle_states.empty ())
        {
            for (auto state : castle_states)
            {
                ColorIndex index = color_index (state.player);
                result.castled[index] = state.castle_state;
            }
        }

        result.half_move_clock = this->half_moves_clock;
        result.full_moves = this->full_moves;

        return result;
    }
}