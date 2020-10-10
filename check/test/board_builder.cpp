#include "board_builder.hpp"

extern "C"
{
#include "../src/board.h"
#include "../src/board_positions.h"
}

void board_builder::add_piece (const char *coord_str, enum color who, enum piece_type piece_type)
{
    if (strlen(coord_str) != 2)
        throw board_builder_exception("Invalid coordinate string!");

    uint8_t col = char_to_col(coord_str[0]);
    uint8_t row = char_to_row(coord_str[1]);

    if (col >= NR_COLUMNS || row >= NR_ROWS)
        throw board_builder_exception("Invalid coordinate string!");

    this->add_piece(row, col, who, piece_type);
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

    this->pieces_with_coords.push_back(new_piece);
}

void board_builder::add_row_of_same_color_and_piece (int row, enum color who, enum piece_type piece_type)
{
    for (uint8_t col = 0; col < NR_COLUMNS; col++)
    {
        this->add_piece (row, col, who, piece_type);
    }
}

void board_builder::add_row_of_same_color (int row, enum color who, vector<enum piece_type> piece_types)
{
    size_t col = 0;

    for (auto it = piece_types.begin(); it != piece_types.end(); it++, col++)
        this->add_piece(row, col, who, *it);
}

struct board *board_builder::build ()
{
    struct piece_row
    {
        vector<enum piece_type> types;
        piece_row() : types { NR_COLUMNS + 1 } {}
    };

    size_t sz = this->pieces_with_coords.size();

    vector<piece_row> piece_types { sz };
    vector<struct board_positions> positions { sz + 1 };

    positions[sz].rank = 0;
    positions[sz].pieces = nullptr;
    positions[sz].piece_color = COLOR_NONE;

    for (size_t i = 0; i < sz; i++)
    {
        struct piece_with_coord piece_with_coord = this->pieces_with_coords[i];

        size_t col = COLUMN(piece_with_coord.coord);

        for (uint8_t c = 0; c < NR_COLUMNS; c++)
            piece_types[i].types[col] = PIECE_NONE;

        piece_types[i].types[col] = piece_with_coord.piece_type;
        piece_types[i].types[NR_COLUMNS] = PIECE_LAST;

        positions[i].rank = ROW(piece_with_coord.coord);
        positions[i].pieces = &piece_types[i].types[0];
        positions[i].piece_color = piece_with_coord.color;
    }

    struct board *board = board_new ();
    board_init_from_positions (board, &positions[0]);

    return board;
}