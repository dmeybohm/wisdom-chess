#include "board_builder.hpp"

extern "C"
{
#include "../src/board.h"
}

void board_builder::add_piece (const char *coord_str, enum color who, enum piece_type piece_type)
{
    if (strlen(coord_str) != 2)
        throw board_builder_exception("Invalid coordinate string!");

    uint8_t col = char_to_col(coord_str[0]);
    uint8_t row = char_to_row(coord_str[1]);

    this->add_piece (row, col, who, piece_type);
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

void board_builder::add_row_of_same_color_and_piece (int row, enum color who, enum piece_type piece_type)
{
    for (uint8_t col = 0; col < NR_COLUMNS; col++)
        this->add_piece (row, col, who, piece_type);
}

void board_builder::add_row_of_same_color (int row, enum color who, std::vector<enum piece_type> piece_types)
{
    size_t col = 0;

    for (auto it = piece_types.begin(); it != piece_types.end(); it++, col++)
        this->add_piece (row, col, who, *it);
}

struct board *board_builder::build ()
{
    struct piece_row
    {
        std::vector<enum piece_type> row;
        piece_row() : row { NR_COLUMNS + 1 } {}
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

        current_piece_row.assign (NR_COLUMNS, PIECE_NONE);
        current_piece_row[col] = piece_with_coord.piece_type;
        current_piece_row[NR_COLUMNS] = PIECE_LAST;

        positions[i] = { row, piece_with_coord.color, &current_piece_row[0] };
    }

    return board_from_positions (&positions[0]);
}