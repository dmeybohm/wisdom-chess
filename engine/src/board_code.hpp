
#ifndef WISDOM_BOARD_CODE_HPP
#define WISDOM_BOARD_CODE_HPP

#include "global.hpp"
#include "piece.hpp"
#include "coord.hpp"
#include "move.hpp"

namespace wisdom
{
    // 3 Bits per piece type, +1 for color (special case: no piece == 0):
    constexpr int Board_Code_Bits_Per_Piece = 4;

    // 4 bits per square * 64 squares = 256 bits
    constexpr int Board_Code_Total_Bits = Board_Code_Bits_Per_Piece * Num_Rows * Num_Columns;

    using BoardCodeBitset = std::bitset<Board_Code_Total_Bits>;

    using BoardHashCode = std::size_t;

    class Board;

    static std::hash<BoardCodeBitset> board_code_hash_fn;

    class BoardCode final
    {
    private:
        BoardCodeBitset bits;

    public:
        BoardCode () = default;

        explicit BoardCode (const Board& board);

        void add_piece (Coord coord, ColoredPiece piece)
        {
            Color color = piece_color (piece);
            Piece type = piece_type (piece);

            uint8_t new_value = color == Color::None ? 0
                                 : piece_index (type) | (color_index (color) << 3);
            assert (new_value < 16);

            int row = Row (coord);
            int col = Column (coord);
            size_t bit_index = (row * Num_Columns + col) * Board_Code_Bits_Per_Piece;

            for (uint8_t i = 0; i < Board_Code_Bits_Per_Piece; i++)
            {
                bits.set (bit_index + i, (new_value & (1 << i)) > 0);
            }
        }

        void remove_piece (Coord coord)
        {
            return add_piece (coord, Piece_And_Color_None);
        }

        [[nodiscard]] auto to_string () const -> string
        {
            return bits.to_string ();
        }

        [[nodiscard]] auto bitset_ref () const& -> const BoardCodeBitset&
        {
            return bits;
        }
        void bitset_ref () const&& = delete;

        [[nodiscard]] BoardHashCode hash_code () const
        {
            return board_code_hash_fn (bits);
        }

        friend bool operator== (const BoardCode& first, const BoardCode& second)
        {
            return first.bits == second.bits;
        }

        friend bool operator!= (const BoardCode& first, const BoardCode& second)
        {
            return !(first == second);
        }

        friend std::ostream &operator<< (std::ostream& os, const BoardCode& code);

        [[nodiscard]] BoardCode with_move (const Board& board, Move move) const
        {
            auto copy = *this;
            copy.apply_move (board, move);
            return copy;
        }

        void apply_move (const Board& board, Move move);

        void unapply_move (const Board& board, Move move, const UndoMove& undo_state);

        [[nodiscard]] std::size_t count_ones () const;
    };
}

#endif //WISDOM_BOARD_CODE_HPP
