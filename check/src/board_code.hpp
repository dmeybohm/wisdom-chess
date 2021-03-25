
#ifndef WISDOM_BOARD_CODE_HPP
#define WISDOM_BOARD_CODE_HPP

#include "global.hpp"
#include "piece.hpp"
#include "coord.hpp"
#include "move.hpp"

#include <bitset>

namespace wisdom
{
    // 3 Bits per piece type, +1 for color (special case: no piece == 0):
    constexpr int Board_Code_Bits_Per_Piece = 4;

    // 4 bits per square * 64 squares = 256 bits
    constexpr int Board_Code_Total_Bits = Board_Code_Bits_Per_Piece * Num_Rows * Num_Columns;

    using BoardCodeBitset = std::bitset<Board_Code_Total_Bits>;

    struct Board;

    struct BoardHash final
    {
        BoardCodeBitset hash; // todo: rework this using Zobrist hashing

        explicit BoardHash ( BoardCodeBitset bits ) : hash { bits } {}
    };

    class BoardCode final
    {
    private:
        BoardCodeBitset bits;

    public:
        BoardCode () = default;

        explicit BoardCode (const Board &board);

        void add_piece (Coord coord, ColoredPiece piece)
        {
            Color color = piece_color (piece);
            Piece type = piece_type (piece);

            uint8_t new_value = color == Color::None ? 0
                                                     : piece_index (type) | (color_index (color) << 3);
            assert (new_value < 16);

            size_t bit_index = (coord.row * Num_Columns + coord.col) * Board_Code_Bits_Per_Piece;

            for (uint8_t i = 0; i < Board_Code_Bits_Per_Piece; i++)
            {
                bits.set (bit_index + i, (new_value & (1 << i)) > 0);
            }
        }

        void remove_piece (Coord coord)
        {
            return add_piece (coord, piece_and_color_none);
        }

        [[nodiscard]] std::string to_string () const
        {
            return bits.to_string ();
        }

        [[nodiscard]] const BoardCodeBitset &bitset_ref () const
        {
            return bits;
        }

        [[nodiscard]] BoardHash hash_code () const
        {
            return BoardHash { bits };
        }

        friend bool operator== (const BoardCode &first, const BoardCode &second)
        {
            return first.bits == second.bits;
        }

        friend bool operator!= (const BoardCode &first, const BoardCode &second)
        {
            return !(first == second);
        }

        void apply_move (const Board &board, Move move);

        void unapply_move (const Board &board, Move move, UndoMove undo_state);

        std::size_t count_ones ();
    };
}
#endif //WISDOM_BOARD_CODE_HPP
