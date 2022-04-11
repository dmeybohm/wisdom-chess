#ifndef WISDOM_BOARD_BUILDER_HPP
#define WISDOM_BOARD_BUILDER_HPP

#include "global.hpp"
#include "piece.hpp"
#include "move.hpp"

namespace wisdom
{
    class BoardBuilder final
    {
    public:
        struct PieceWithCoordState
        {
            Coord coord;
            Color color;
            Piece piece_type;
        };

        struct PieceCoordStringWithTypeState
        {
            czstring coord;
            Piece piece_type;
        };

        struct EnPassantState
        {
            Color player;
            Coord coord;
        };

        struct ColorAndCastlingState
        {
            Color player;
            CastlingState castle_state;
        };

        BoardBuilder () = default;

        void add_piece (const string& coord_str, Color who, Piece piece_type);

        void add_piece (int row, int col, Color who, Piece piece_type);

        void add_pieces (Color who, const vector<PieceCoordStringWithTypeState>& pieces);

        void add_row_of_same_color (int row, Color who, vector<Piece> piece_types);

        void add_row_of_same_color (const string& coord_str, Color who, vector<Piece> piece_types);

        void add_row_of_same_color_and_piece (int row, Color who, Piece piece_type);

        void add_row_of_same_color_and_piece (const string& coord_str, Color who, Piece piece_type);

        void set_en_passant_target (Color vulnerable_color, const string& coord_str);

        void set_castling (Color who, CastlingState state);

        void set_half_moves (int new_half_moves_clock);

        void set_full_moves (int new_full_moves);

        [[nodiscard]] auto build () const -> unique_ptr<Board>;

    private:
        vector<PieceWithCoordState> pieces_with_coords;
        vector<EnPassantState> en_passant_states;
        vector<ColorAndCastlingState> castle_states;
        int half_moves_clock = 0;
        int full_moves = 0;
    };

    class BoardBuilderError : public Error
    {
    public:
        explicit BoardBuilderError (const string &message) :
            Error (message)
        {}
    };
}

#endif //WISDOM_BOARD_BUILDER_HPP
