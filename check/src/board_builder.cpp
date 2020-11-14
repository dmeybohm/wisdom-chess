#include "board_builder.hpp"

#include "board.h"
#include "move.h"

coord_t coord_alg (const char *coord_str)
{
    if (!coord_str || strlen(coord_str) != 2)
        throw board_builder_exception("Invalid coordinate string!");

    uint8_t col = char_to_col(coord_str[0]);
    uint8_t row = char_to_row(coord_str[1]);

    if (row < 0 || row >= NR_ROWS)
        throw board_builder_exception("Invalid row!");

    if (col < 0 || col >= NR_COLUMNS)
        throw board_builder_exception("Invalid column!");

    return coord_create (row, col);
}

void board_builder::add_piece (const char *coord_str, enum color who, enum piece_type piece_type)
{
    if (strlen(coord_str) != 2)
        throw board_builder_exception("Invalid coordinate string!");

    coord_t algebraic = coord_alg (coord_str);

    this->add_piece (ROW(algebraic), COLUMN(algebraic), who, piece_type);
}

void board_builder::add_piece (uint8_t row, uint8_t col, enum color who, enum piece_type piece_type)
{
    struct piece_with_coord new_piece
    {
        .coord = coord_create (row, col),
        .color = who,
        .piece_type = piece_type,
    };

    if (row < 0 || row >= NR_ROWS)
        throw board_builder_exception("Invalid row!");

    if (col < 0 || col >= NR_COLUMNS)
        throw board_builder_exception("Invalid column!");

    this->pieces_with_coords.push_back (new_piece);
}

void board_builder::add_pieces (enum color who, std::vector<struct piece_coord_string_with_type> pieces)
{
    for (auto it : pieces)
        this->add_piece (it.coord, who, it.piece_type);
}

void board_builder::add_row_of_same_color_and_piece (int row, enum color who, enum piece_type piece_type)
{
    for (uint8_t col = 0; col < NR_COLUMNS; col++)
        this->add_piece (row, col, who, piece_type);
}

void board_builder::add_row_of_same_color_and_piece (const char *coord_str, enum color who, enum piece_type piece_type)
{
    coord_t coord = coord_alg (coord_str);

    for (uint8_t col = 0; col < NR_COLUMNS; col++)
        this->add_piece (ROW(coord), col, who, piece_type);
}

void board_builder::add_row_of_same_color (int row, enum color who, std::vector<enum piece_type> piece_types)
{
    size_t col = 0;

    for (auto it = piece_types.begin(); it != piece_types.end(); it++, col++)
        this->add_piece (row, col, who, *it);
}

void board_builder::add_row_of_same_color (const char *coord_str, enum color who, std::vector<enum piece_type> piece_types)
{
    coord_t coord = coord_alg (coord_str);
    size_t col = 0;

    for (auto it = piece_types.begin(); it != piece_types.end(); it++, col++)
        this->add_piece (ROW(coord), col, who, *it);
}

void board_builder::set_en_passant_target (enum color who, const char *coord_str)
{
    struct en_passant_state new_state { who, coord_alg(coord_str) };
    this->en_passant_states.push_back (new_state);
}

void board_builder::set_castling (enum color who, castle_state_t state)
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

struct board *board_builder::build ()
{
    struct piece_row
    {
        std::vector<enum piece_type> row;
    };

    size_t sz = this->pieces_with_coords.size();

    std::vector<piece_row> piece_types { sz };
    std::vector<struct board_positions> positions { sz + 1 };

    positions[sz] = { 0, COLOR_NONE, nullptr };

    for (size_t i = 0; i < sz; i++)
    {
        struct piece_with_coord &piece_with_coord = this->pieces_with_coords[i];
        std::vector<enum piece_type> &current_piece_row = piece_types[i].row;

        uint8_t col = COLUMN(piece_with_coord.coord);
        uint8_t row = ROW(piece_with_coord.coord);

        current_piece_row.assign (NR_COLUMNS + 1, PIECE_NONE);
        current_piece_row[col] = piece_with_coord.piece_type;
        current_piece_row[NR_COLUMNS] = PIECE_LAST;

        positions[i] = { row, piece_with_coord.color, &current_piece_row[0] };
    }

    struct board *result = board_from_positions (&positions[0]);

    if (!en_passant_states.empty())
    {
        for (auto state : en_passant_states)
        {
            color_index_t index = color_index(state.player);
            result->en_passant_target[index] = state.coord;
        }
    }

    if (!castle_states.empty())
    {
        for (auto state : castle_states)
        {
            color_index_t index = color_index(state.player);
            result->castled[index] = state.castle_state;
        }
    }

    result->half_move_clock = this->half_moves_clock;
    result->full_moves = this->full_moves;

    return result;
}
