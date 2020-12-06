
#ifndef WIZDUMB_BOARD_CODE_HPP
#define WIZDUMB_BOARD_CODE_HPP

#include "global.h"
#include "piece.h"
#include "coord.h"
#include "move.h"

#include <bitset>

// 3 Bits per piece type, +1 for color (special case: no piece == 0):
constexpr int Board_Code_Bits_Per_Piece = 4;

// 4 bits per square * 64 squares = 256 bits
constexpr int Board_Code_Total_Bits = Board_Code_Bits_Per_Piece
            * Num_Rows * Num_Columns;

using board_code_bitset = std::bitset<Board_Code_Total_Bits>;

struct board;

class board_code final
{
private:
    board_code_bitset bits;

public:
    void add_piece (coord_t coord, piece_t piece)
    {
        Color color = piece_color(piece);
        Piece type = piece_type (piece);

        color_index_t cindex = color_index_with_none (color);
        uint8_t new_value = cindex == 0 ? 0
                : piece_index (type) | (color_index(color) << 3);
        assert (new_value < 16);

        size_t bit_index = (coord.row * Num_Columns + coord.col) * Board_Code_Bits_Per_Piece;

        for (uint8_t i = 0; i < 4; i++)
        {
            if (new_value & (1 << i))
                bits.set (bit_index + i);
            else
                bits.reset (bit_index + i);
        }
    }

    void remove_piece (coord_t coord)
    {
        return add_piece (coord, piece_and_color_none);
    }

    [[nodiscard]] std::string to_string () const
    {
        return bits.to_string ();
    }

    [[nodiscard]] const board_code_bitset &bitset_ref () const
    {
        return bits;
    }

    friend bool operator == (const board_code &first, const board_code &second)
    {
        return first.bits == second.bits;
    }

    friend bool operator != (const board_code &first, const board_code &second)
    {
        return !(first == second);
    }

    void apply_move (const struct board &board, move_t move);
    void unapply_move (const struct board &board, move_t move, undo_move_t undo_state);
};

#endif //WIZDUMB_BOARD_CODE_HPP
