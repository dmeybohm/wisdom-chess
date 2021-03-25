#ifndef WISDOM_BOARD_BUILDER_HPP
#define WISDOM_BOARD_BUILDER_HPP

#include "global.hpp"
#include "piece.hpp"
#include "move.hpp"

#include <cstdint>
#include <string>
#include <vector>
#include <array>

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
        const char *coord;
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
        std::vector<BBPieceWithCoordState> pieces_with_coords;
        std::vector<BBEnPassantState> en_passant_states;
        std::vector<BBCastlingState> castle_states;
        int16_t half_moves_clock = 0;
        int16_t full_moves = 0;

    public:
        BoardBuilder () = default;

        void add_piece (const std::string &coord_str, Color who, Piece piece_type);

        void add_piece (int8_t row, int8_t col, Color who, Piece piece_type);

        void add_pieces (Color who, const std::vector<struct BBPieceCoordStringWithTypeState> &pieces);

        void add_row_of_same_color (int row, Color who, std::vector<Piece> piece_types);

        void add_row_of_same_color (const std::string &coord_str, Color who, std::vector<Piece> piece_types);

        void add_row_of_same_color_and_piece (int row, Color who, Piece piece_type);

        void add_row_of_same_color_and_piece (const std::string &coord_str, Color who, Piece piece_type);

        void set_en_passant_target (Color who, const std::string &coord_str);

        void set_castling (Color who, CastlingState state);

        void set_half_moves (int new_half_moves_clock);

        void set_full_moves (int new_full_moves);

        Board build ();
    };

    class BoardBuilderException : public std::exception
    {
    private:
        const char *message;

    public:
        explicit BoardBuilderException (const char *message) : message { message }
        {}

        [[nodiscard]] const char *what () const noexcept override
        { return this->message; }
    };

    Coord coord_alg (const std::string &coord_str);
}

#endif //WISDOM_BOARD_BUILDER_HPP
