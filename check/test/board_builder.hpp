#ifndef WIZDUMB_BOARD_BUILDER_HPP
#define WIZDUMB_BOARD_BUILDER_HPP

extern "C"
{
#include "piece.h"
#include "move.h"
}

#include <cstdint>
#include <string>
#include <vector>

struct piece_with_coord
{
    coord_t coord;
    enum color color;
    enum piece_type piece_type;
};

class board_builder final
{
    std::vector<piece_with_coord> pieces_with_coords;

public:
    board_builder() = default;

    void add_piece (const char *coord_str, enum color who, enum piece_type piece_type);

    void add_piece (uint8_t row, uint8_t col, enum color who, enum piece_type piece_type);

    void add_row_of_same_color (int row, enum color who, std::vector<enum piece_type> piece_types);

    void add_row_of_same_color (const char *coord_str, enum color who, std::vector<enum piece_type> piece_types);

    void add_row_of_same_color_and_piece (int row, enum color who, enum piece_type piece_type);

    void add_row_of_same_color_and_piece (const char *coord_str, enum color who, enum piece_type piece_type);

    struct board *build();
};

class board_builder_exception : public std::exception
{
    const char *message;

public:
    explicit board_builder_exception (const char *message) : message { message } {}
    const char *what() const noexcept override { return this->message; }
};

coord_t coord_alg (const char *coord_str);


#endif //WIZDUMB_BOARD_BUILDER_HPP
