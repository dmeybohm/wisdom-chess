#include "board_builder.hpp"

#include "board.hpp"
#include "move.hpp"
#include "coord.hpp"

namespace wisdom
{
    void BoardBuilder::add_piece (const string& coord_str, Color who, Piece piece_type)
    {
        if (coord_str.size () != 2)
            throw BoardBuilderError ("Invalid coordinate string!");

        Coord algebraic = coord_parse (coord_str);

        this->add_piece (Row (algebraic), Column (algebraic), who, piece_type);
    }

    void BoardBuilder::add_piece (int row, int col, Color who, Piece piece_type)
    {
        PieceWithCoordState new_piece {
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

    void BoardBuilder::add_pieces (Color who, const vector<PieceCoordStringWithTypeState> &pieces)
    {
        for (auto&& it : pieces)
            this->add_piece (it.coord, who, it.piece_type);
    }

    void BoardBuilder::add_row_of_same_color_and_piece (int row, Color who, Piece piece_type)
    {
        for (int col = 0; col < Num_Columns; col++)
            this->add_piece (row, col, who, piece_type);
    }

    void BoardBuilder::add_row_of_same_color_and_piece (const string& coord_str, Color who,
                                                        Piece piece_type)
    {
        Coord coord = coord_parse (coord_str);

        for (int col = 0; col < Num_Columns; col++)
            this->add_piece (Row (coord), col, who, piece_type);
    }

    void BoardBuilder::add_row_of_same_color (int row, Color who, const vector<Piece>& piece_types)
    {
        int col = 0;

        for (auto piece_type : piece_types)
            this->add_piece (row, col++, who, piece_type);
    }

    void BoardBuilder::add_row_of_same_color (const string& coord_str, Color who,
                                              const vector<Piece>& piece_types)
    {
        Coord coord = coord_parse (coord_str);
        int col = 0;

        for (auto piece_type : piece_types)
            this->add_piece (Row (coord), col++, who, piece_type);
    }

    void BoardBuilder::set_current_turn (Color who)
    {
        this->current_turn = who;
    }

    void BoardBuilder::set_en_passant_target (Color vulnerable_color, const string& coord_str)
    {
        EnPassantState new_state { vulnerable_color, coord_parse (coord_str) };
        this->en_passant_states.push_back (new_state);
    }

    void BoardBuilder::set_castling (Color who, CastlingState state)
    {
        ColorAndCastlingState new_state { .player = who, .castle_state = state };
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

    auto BoardBuilder::build () const -> unique_ptr<Board>
    {
        struct PieceRow
        {
            vector<Piece> row;
        };

        std::size_t sz = this->pieces_with_coords.size ();

        vector<PieceRow> piece_types { sz };
        vector<BoardPositions> positions { sz };

        for (size_t i = 0; i < sz; i++)
        {
            const PieceWithCoordState& piece_with_coord = this->pieces_with_coords[i];
            vector<Piece>& current_piece_row = piece_types[i].row;

            int8_t col = Column (piece_with_coord.coord);
            int8_t row = Row (piece_with_coord.coord);

            current_piece_row.assign (Num_Columns, Piece::None);
            current_piece_row[col] = piece_with_coord.piece_type;

            positions[i] = { row, piece_with_coord.color, current_piece_row };
        }

        auto result = make_unique<Board> (positions);

        if (!en_passant_states.empty ())
        {
            for (auto state : en_passant_states)
            {
                result->set_en_passant_target (state.player, state.coord);
            }
        }

        if (!castle_states.empty ())
        {
            for (auto state : castle_states)
            {
                result->set_castle_state (state.player, state.castle_state);
            }
        }

        result->my_half_move_clock = this->half_moves_clock;
        result->my_full_move_clock = this->full_moves;
        result->set_current_turn (current_turn);

        return result;
    }
}
