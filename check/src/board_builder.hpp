#ifndef WIZDUMB_BOARD_BUILDER_HPP
#define WIZDUMB_BOARD_BUILDER_HPP

#include "global.h"
#include "piece.h"
#include "move.h"

#include <cstdint>
#include <string>
#include <vector>
#include <array>

struct piece_with_coord
{
    coord_t coord;
    Color color;
    Piece piece_type;
};

struct piece_coord_string_with_type
{
    const char *coord;
    Piece piece_type;
};

struct en_passant_state
{
    Color player;
    coord_t coord;
};

struct bb_castle_state
{
    Color player;
    castle_state_t castle_state;
};

class board_builder final
{
private:
    std::vector<piece_with_coord> pieces_with_coords;
    std::vector<en_passant_state> en_passant_states;
    std::vector<bb_castle_state> castle_states;
    int half_moves_clock = 0;
    int full_moves = 0;

public:
    board_builder() = default;

    void add_piece (const std::string &coord_str, Color who, Piece piece_type);

    void add_piece (int8_t row, int8_t col, Color who, Piece piece_type);

    void add_pieces (Color who, const std::vector<struct piece_coord_string_with_type> &pieces);

    void add_row_of_same_color (int row, Color who, std::vector<Piece> piece_types);

    void add_row_of_same_color (const std::string &coord_str, Color who, std::vector<Piece> piece_types);

    void add_row_of_same_color_and_piece (int row, Color who, Piece piece_type);

    void add_row_of_same_color_and_piece (const std::string &coord_str, Color who, Piece piece_type);

    void set_en_passant_target (Color who, const std::string &coord_str);

    void set_castling (Color who, castle_state_t state);

    void set_half_moves (int new_half_moves_clock);

    void set_full_moves (int new_full_moves);

    board build();
};

class board_builder_exception : public std::exception
{
private:
    const char *message;

public:
    explicit board_builder_exception (const char *message) : message { message } {}
    [[nodiscard]] const char *what() const noexcept override { return this->message; }
};

coord_t coord_alg (const std::string & coord_str);

#endif //WIZDUMB_BOARD_BUILDER_HPP
