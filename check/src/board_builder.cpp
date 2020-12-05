#include "board_builder.hpp"

#include "board.h"
#include "move.h"

coord_t coord_alg (const std::string &coord_str)
{
    if (coord_str.size() != 2)
        throw board_builder_exception("Invalid coordinate string!");

    int8_t col = char_to_col(coord_str[0]);
    int8_t row = char_to_row(coord_str[1]);

    if (row < 0 || row >= NR_ROWS)
        throw board_builder_exception("Invalid row!");

    if (col < 0 || col >= NR_COLUMNS)
        throw board_builder_exception("Invalid column!");

    return make_coord (row, col);
}

void board_builder::add_piece (const std::string &coord_str, Color who, Piece piece_type)
{
    if (coord_str.size() != 2)
        throw board_builder_exception("Invalid coordinate string!");

    coord_t algebraic = coord_alg (coord_str);

    this->add_piece (ROW(algebraic), COLUMN(algebraic), who, piece_type);
}

void board_builder::add_piece (int8_t row, int8_t col, Color who, Piece piece_type)
{
    struct piece_with_coord new_piece
    {
        .coord = make_coord (row, col),
        .color = who,
        .piece_type = piece_type,
    };

    if (row < 0 || row >= NR_ROWS)
        throw board_builder_exception("Invalid row!");

    if (col < 0 || col >= NR_COLUMNS)
        throw board_builder_exception("Invalid column!");

    this->pieces_with_coords.push_back (new_piece);
}

void board_builder::add_pieces (Color who, const std::vector<struct piece_coord_string_with_type> &pieces)
{
    for (auto it : pieces)
        this->add_piece (it.coord, who, it.piece_type);
}

void board_builder::add_row_of_same_color_and_piece (int row, Color who, Piece piece_type)
{
    for (int8_t col = 0; col < NR_COLUMNS; col++)
        this->add_piece (row, col, who, piece_type);
}

void board_builder::add_row_of_same_color_and_piece (const std::string &coord_str, Color who, Piece piece_type)
{
    coord_t coord = coord_alg (coord_str);

    for (int8_t col = 0; col < NR_COLUMNS; col++)
        this->add_piece (ROW(coord), col, who, piece_type);
}

void board_builder::add_row_of_same_color (int row, Color who, std::vector<Piece> piece_types)
{
    size_t col = 0;

    for (auto it = piece_types.begin(); it != piece_types.end(); it++, col++)
        this->add_piece (row, col, who, *it);
}

void board_builder::add_row_of_same_color (const std::string &coord_str, Color who, std::vector<Piece> piece_types)
{
    coord_t coord = coord_alg (coord_str);
    size_t col = 0;

    for (auto it = piece_types.begin(); it != piece_types.end(); it++, col++)
        this->add_piece (ROW(coord), col, who, *it);
}

void board_builder::set_en_passant_target (Color who, const std::string &coord_str)
{
    struct en_passant_state new_state { who, coord_alg(coord_str) };
    this->en_passant_states.push_back (new_state);
}

void board_builder::set_castling (Color who, castle_state_t state)
{
    struct bb_castle_state new_state { .player = who, .castle_state = state };
    castle_states.push_back (new_state);
}

void board_builder::set_half_moves (int new_half_moves_clock)
{
    this->half_moves_clock = new_half_moves_clock;
}

void board_builder::set_full_moves (int new_full_moves)
{
    this->full_moves = new_full_moves;
}

struct board board_builder::build ()
{
    struct piece_row
    {
        std::vector<Piece> row;
    };

    size_t sz = this->pieces_with_coords.size();

    std::vector<struct piece_row> piece_types { sz };
    std::vector<struct board_positions> positions { sz };

    for (size_t i = 0; i < sz; i++)
    {
        struct piece_with_coord &piece_with_coord = this->pieces_with_coords[i];
        std::vector<Piece> &current_piece_row = piece_types[i].row;

        int8_t col = COLUMN(piece_with_coord.coord);
        int8_t row = ROW(piece_with_coord.coord);

        current_piece_row.assign (NR_COLUMNS, Piece::None);
        current_piece_row[col] = piece_with_coord.piece_type;

        positions[i] = { row, piece_with_coord.color, current_piece_row };
    }

    struct board result = board { positions };

    if (!en_passant_states.empty())
    {
        for (auto state : en_passant_states)
        {
            color_index_t index = color_index(state.player);
            result.en_passant_target[index] = state.coord;
        }
    }

    if (!castle_states.empty())
    {
        for (auto state : castle_states)
        {
            color_index_t index = color_index(state.player);
            result.castled[index] = state.castle_state;
        }
    }

    result.half_move_clock = this->half_moves_clock;
    result.full_moves = this->full_moves;

    return result;
}
