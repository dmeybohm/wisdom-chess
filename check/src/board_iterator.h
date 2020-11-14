#ifndef WIZDUMB_BOARD_ITERATOR_H
#define WIZDUMB_BOARD_ITERATOR_H

#include <iterator>

#include "board.h"

struct board;

class board_iterator final
{
private:
    struct board *my_board;
    uint8_t my_row;
    uint8_t my_col;

public:
    // Previously provided by std::iterator - see update below
    using value_type = piece_t;
    using difference_type = std::ptrdiff_t;
    using pointer = piece_t*;
    using reference = piece_t&;
    using iterator_category = std::input_iterator_tag;

    explicit board_iterator(struct board *board, uint8_t row, uint8_t col) :
            my_board { board },
            my_row { row },
            my_col { col }
    {
    }

    piece_t operator*() const
    {
        return my_board->board[my_row][my_col];
    }

    bool operator == (const board_iterator & other) const
    {
        return my_board == other.my_board &&
            my_row == other.my_row &&
            my_col == other.my_col;
    }

    bool operator != (const board_iterator & other) const
    {
        return !(*this == other);
    }

    board_iterator& operator++()
    {
        my_col++;
        if (my_col == NR_COLUMNS)
        {
            my_row++;
            my_col = 0;
        }
        return *this;
    }
};


#endif //WIZDUMB_BOARD_ITERATOR_H
