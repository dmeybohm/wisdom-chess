#ifndef WISDOM_CHESS_BOARD_CODE_HPP
#define WISDOM_CHESS_BOARD_CODE_HPP

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

    constexpr int Ancillary_Bits = 16;

    inline std::hash<BoardCodeBitset> pieces_hash_fn;
    inline std::hash<std::bitset<Ancillary_Bits>> ancillary_hash_fn;

    using AncillaryBitset = std::bitset<Ancillary_Bits>;

    using EnPassantTargets = std::array<Coord, Num_Players>;

    class Board;

    class BoardCode final
    {
    private:
        BoardCodeBitset my_pieces;
        AncillaryBitset my_ancillary;

        enum AncillaryBitPositions {
            CURRENT_TURN_BIT = 0,
            EN_PASSANT_WHITE_TARGET = 1,
            EN_PASSANT_BLACK_TARGET = 5,
            CASTLING_STATE_WHITE_TARGET = 9,
            CASTLING_STATE_BLACK_TARGET = 12,
            CURRENT_TURN_MASK = 0b1,
            EN_PASSANT_MASK = 0b11111111,
            CASTLE_ONE_COLOR_MASK = 0b11,
            EN_PASSANT_PRESENT = 0b1000,
        };

    public:
        BoardCode (const Board& board, EnPassantTargets, Color current_turn, PlayerCastleState);

        static auto from_board (const Board& board) -> BoardCode;

        static auto default_position_board_code () -> BoardCode;

        void add_piece (Coord coord, ColoredPiece piece)
        {
            Color color = piece_color (piece);
            Piece type = piece_type (piece);

            uint8_t new_value = piece == Piece_And_Color_None ? 0 :
                                 piece_index (type) | (color_index (color) << 3);
            assert (new_value < 16);

            int8_t row = Row (coord);
            int8_t col = Column (coord);
            size_t bit_index = (row * Num_Columns + col) * Board_Code_Bits_Per_Piece;

            for (uint8_t i = 0; i < Board_Code_Bits_Per_Piece; i++)
            {
                my_pieces.set (bit_index + i, (new_value & (1 << i)) > 0);
            }
        }

        void remove_piece (Coord coord)
        {
            return add_piece (coord, Piece_And_Color_None);
        }

        void set_en_passant_target (Color color, Coord coord)
        {
            std::size_t target_bit_shift = color == Color::White ?
                    EN_PASSANT_WHITE_TARGET :
                    EN_PASSANT_BLACK_TARGET;
            auto coord_bits = Column<std::size_t> (coord);
            coord_bits |= (coord == No_En_Passant_Coord) ? 0 : EN_PASSANT_PRESENT;
            coord_bits <<= target_bit_shift;

            assert (coord == No_En_Passant_Coord || (
                    Row (coord) == (color == Color::White ? White_En_Passant_Row : Black_En_Passant_Row)));

            // clear both targets initially. There can be only one at a given time.
            my_ancillary &= ~(EN_PASSANT_MASK << EN_PASSANT_WHITE_TARGET);
            my_ancillary |= coord_bits;
        }

        [[nodiscard]] auto en_passant_target (Color vulnerable_color) const noexcept -> Coord
        {
            std::size_t target_bits = my_ancillary.to_ulong ();
            std::size_t target_bit_shift = vulnerable_color == Color::White ?
                                             EN_PASSANT_WHITE_TARGET :
                                             EN_PASSANT_BLACK_TARGET;

            target_bits &= EN_PASSANT_MASK << EN_PASSANT_WHITE_TARGET;
            target_bits >>= target_bit_shift;
            auto col = gsl::narrow<int8_t> (target_bits & 0x7);
            auto is_present = (bool)(target_bits & EN_PASSANT_PRESENT);
            auto row = vulnerable_color == Color::White ? White_En_Passant_Row : Black_En_Passant_Row;
            return is_present ? make_coord (row, col) : No_En_Passant_Coord;
        }

        void set_current_turn (Color who)
        {
            auto bits = color_index (who);
            auto current_turn_bit = bits & (CURRENT_TURN_MASK << CURRENT_TURN_BIT);
            my_ancillary &= ~(CURRENT_TURN_MASK << CURRENT_TURN_BIT);
            my_ancillary |= current_turn_bit;
        }

        [[nodiscard]] auto castle_state (Color who) const -> CastlingState
        {
            std::size_t target_bits = my_ancillary.to_ulong ();
            std::size_t target_bit_shift = who == Color::White ?
                                                  CASTLING_STATE_WHITE_TARGET :
                                                  CASTLING_STATE_BLACK_TARGET;
            auto castle_state = gsl::narrow_cast<uint8_t> (target_bits >> target_bit_shift);
            return castle_state & CASTLE_ONE_COLOR_MASK;
        }

        void set_castle_state (Color who, CastlingState castling_state)
        {
            assert (castling_state != Castle_Previously_None); // only store the actual flags.
            std::size_t bit_number = who == Color::White ?
                                                  CASTLING_STATE_WHITE_TARGET :
                                                  CASTLING_STATE_BLACK_TARGET;
            std::size_t mask = CASTLE_ONE_COLOR_MASK << bit_number;

            my_ancillary &= ~mask;
            my_ancillary |= castling_state << bit_number;
        }

        [[nodiscard]] auto current_turn () const -> Color
        {
            auto bits = my_ancillary.to_ulong ();
            auto index = gsl::narrow_cast<int8_t> (bits & (CURRENT_TURN_MASK << CURRENT_TURN_BIT));
            return color_from_color_index (index);
        }

        [[nodiscard]] auto en_passant_targets () const noexcept -> EnPassantTargets
        {
            EnPassantTargets result = {
                    en_passant_target (Color::White),
                    en_passant_target (Color::Black)
            };
            return result;
        }

        [[nodiscard]] auto to_string () const -> string
        {
            return my_pieces.to_string ();
        }

        [[nodiscard]] auto bitset_ref () const& -> const BoardCodeBitset&
        {
            return my_pieces;
        }
        void bitset_ref () const&& = delete;

        // Return by value:
        [[nodiscard]] auto get_ancillary_bits () -> AncillaryBitset
        {
            return my_ancillary;
        }

        [[nodiscard]] BoardHashCode hash_code () const
        {
            // todo use Zobrist hashing here instead
            return pieces_hash_fn (my_pieces) ^ ancillary_hash_fn (my_ancillary);
        }

        friend bool operator== (const BoardCode& first, const BoardCode& second)
        {
            return first.my_pieces == second.my_pieces &&
                first.my_ancillary == second.my_ancillary;
        }

        friend bool operator!= (const BoardCode& first, const BoardCode& second)
        {
            return !(first == second);
        }

        friend std::ostream &operator<< (std::ostream& os, const BoardCode& code);

        [[nodiscard]] auto with_move (const Board& board, Move move) const -> BoardCode
        {
            auto copy = *this;
            copy.apply_move (board, move);
            return copy;
        }

        void apply_move (const Board& board, Move move);

        void unapply_move (const Board& board, Move move, const UndoMove& undo_state);

        [[nodiscard]] auto count_ones () const -> std::size_t;
    };
}

namespace std
{
    template<>
    struct hash<wisdom::BoardCode>
    {
        auto operator() (const wisdom::BoardCode& code) const -> std::size_t
        {
            return code.hash_code ();
        }
    };
}

#endif //WISDOM_CHESS_BOARD_CODE_HPP
