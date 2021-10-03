#ifndef WISDOM_BOARD_BUILDER_HPP
#define WISDOM_BOARD_BUILDER_HPP

#include "global.hpp"
#include "piece.hpp"
#include "move.hpp"

namespace wisdom
{
    struct BBPieceWithCoordState
    {
        Coord coord;
        Color color;
        Piece piece_type;
    };

    struct BBPieceCoordStringWithTypeState
    {
        czstring coord;
        Piece piece_type;
    };

    struct BBEnPassantState
    {
        Color player;
        Coord coord;
    };

    struct BBCastlingState
    {
        Color player;
        CastlingState castle_state;
    };

    class BoardBuilder final
    {
    private:
        vector<BBPieceWithCoordState> pieces_with_coords;
        vector<BBEnPassantState> en_passant_states;
        vector<BBCastlingState> castle_states;
        int half_moves_clock = 0;
        int full_moves = 0;

    public:
        BoardBuilder () = default;

        void add_piece (const string &coord_str, Color who, Piece piece_type);

        void add_piece (int row, int col, Color who, Piece piece_type);

        void add_pieces (Color who, const std::vector<struct BBPieceCoordStringWithTypeState> &pieces);

        void add_row_of_same_color (int row, Color who, vector<Piece> piece_types);

        void add_row_of_same_color (const string &coord_str, Color who, vector<Piece> piece_types);

        void add_row_of_same_color_and_piece (int row, Color who, Piece piece_type);

        void add_row_of_same_color_and_piece (const string &coord_str, Color who, Piece piece_type);

        void set_en_passant_target (Color who, const string &coord_str);

        void set_castling (Color who, CastlingState state);

        void set_half_moves (int new_half_moves_clock);

        void set_full_moves (int new_full_moves);

        auto build () -> unique_ptr<Board>;
    };

    class BoardBuilderError : public Error
    {
    public:
        explicit BoardBuilderError (const string &message) :
            Error (message)
        {}
    };

    Coord coord_alg (const string &coord_str);
}

#endif //WISDOM_BOARD_BUILDER_HPP
