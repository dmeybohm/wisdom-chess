#ifndef WIZDUMB_BOARD_BUILDER_HPP
#define WIZDUMB_BOARD_BUILDER_HPP

extern "C"
{
#include "../src/piece.h"
#include "../src/move.h"
}

#include <cstdint>
#include <string>
#include <vector>

using namespace std;

struct piece_with_coord
{
    coord_t coord;
    enum color color;
    enum piece_type piece_type;
};

class board_builder
{
    vector<piece_with_coord> pieces_with_coords{};

public:
    board_builder() = default;

    void add_piece (const char *coord_str, enum color who, enum piece_type piece_type);

    void add_piece (uint8_t row, uint8_t col, enum color who, enum piece_type piece_type);

    void add_row_of_same_color (int row, enum color who, vector<enum piece_type> piece_types);

    void add_row_of_same_color_and_piece (int row, enum color who, enum piece_type piece_type);

    struct board *build();
};

class board_builder_exception : public exception
{
    const char *message;

public:
    explicit board_builder_exception (const char *message) { this->message = message; }
    virtual const char *what() const noexcept { return this->message; }
};

#endif //WIZDUMB_BOARD_BUILDER_HPP
