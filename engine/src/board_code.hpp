#ifndef WISDOM_CHESS_BOARD_CODE_HPP
#define WISDOM_CHESS_BOARD_CODE_HPP

#include "global.hpp"
#include "piece.hpp"
#include "coord.hpp"
#include "move.hpp"
#include "random.hpp"

namespace wisdom
{
    using EnPassantTargets = array<Coord, Num_Players>;

    using BoardHashCode = std::uint64_t;

    class Board;
    class BoardBuilder;

    using BoardCodeArray = array<uint64_t, (Num_Players + 1) * Num_Piece_Types * Num_Squares>;

    [[nodiscard]] constexpr auto initializeBoardCodes() -> BoardCodeArray
    {
        BoardCodeArray code_array {};
        CompileTimeRandom random;

        for (auto piece_type = 0; piece_type < Num_Piece_Types * (Num_Players + 1); piece_type++)
        {
            for (auto square = 0; square < Num_Squares; square++)
                code_array[(piece_type * Num_Squares) + square] = getCompileTimeRandom48 (random);
        }

        return code_array;
    }

    static constexpr BoardCodeArray Hash_Code_Table = initializeBoardCodes();

    static constexpr int Total_Metadata_Bits = 16;

    [[nodiscard]] constexpr auto boardCodeHash (Coord coord, ColoredPiece piece) -> std::uint64_t
    {
        auto coord_index = coord.index();
        auto piece_color = piece.color();
        auto piece_type = piece.type();
        auto piece_index = pieceIndex (piece_type) * (toInt (piece_color) + 1);

        return Hash_Code_Table[piece_index * Num_Squares + coord_index] << Total_Metadata_Bits;
    }

    class BoardCode final
    {
    private:
        enum MetadataBits : std::size_t
        {
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
        explicit BoardCode (const Board& board);

        [[nodiscard]] static auto fromBoard (const Board& board) -> BoardCode;

        [[nodiscard]] static auto fromBoardBuilder (const BoardBuilder& builder) -> BoardCode;

        [[nodiscard]] static auto fromDefaultPosition() -> BoardCode;

        [[nodiscard]] static auto fromEmptyBoard() -> BoardCode;

        void addPiece (Coord coord, ColoredPiece piece) noexcept
        {
            if (piece == Piece_And_Color_None)
                return;

            auto hash = boardCodeHash (coord, piece);
            my_code ^= hash;
        }

        void removePiece (Coord coord, ColoredPiece piece) noexcept
        {
            addPiece (coord, piece);
        }

        void setEnPassantTarget (Color color, Coord coord) noexcept
        {
            std::size_t target_bit_shift = color == Color::White
                ? EN_PASSANT_WHITE_TARGET : EN_PASSANT_BLACK_TARGET;
            auto coord_bits = Column<std::size_t> (coord);
            coord_bits |= (coord == No_En_Passant_Coord) ? 0 : EN_PASSANT_PRESENT;
            coord_bits <<= target_bit_shift;

            assert (coord == No_En_Passant_Coord || (
                    Row (coord) == (color == Color::White ? White_En_Passant_Row : Black_En_Passant_Row)));

            // clear both targets initially. There can be only one at a given time.
            auto metadata = getMetadataBits();
            metadata &= ~(EN_PASSANT_MASK << EN_PASSANT_WHITE_TARGET);
            metadata |= coord_bits;
            setMetadataBits (metadata);
        }

        [[nodiscard]] auto enPassantTarget (Color vulnerable_color) const noexcept -> Coord
        {
            auto target_bits = getMetadataBits();
            auto target_bit_shift = vulnerable_color == Color::White
                ? EN_PASSANT_WHITE_TARGET : EN_PASSANT_BLACK_TARGET;

            target_bits &= EN_PASSANT_MASK << EN_PASSANT_WHITE_TARGET;
            target_bits >>= target_bit_shift;
            auto col = gsl::narrow<int8_t> (target_bits & 0x7);
            auto is_present = (bool)(target_bits & EN_PASSANT_PRESENT);
            auto row = vulnerable_color == Color::White ? White_En_Passant_Row : Black_En_Passant_Row;
            return is_present ? makeCoord (row, col) : No_En_Passant_Coord;
        }

        void setCurrentTurn (Color who) noexcept
        {
            auto bits = colorIndex (who);
            auto current_turn_bit = bits & (CURRENT_TURN_MASK << CURRENT_TURN_BIT);
            auto metadataBits = getMetadataBits();

            metadataBits &= ~(CURRENT_TURN_MASK << CURRENT_TURN_BIT);
            metadataBits |= current_turn_bit;
            setMetadataBits (metadataBits);
        }

        [[nodiscard]] auto castleState (Color who) const -> CastlingEligibility
        {
            auto target_bits = getMetadataBits();
            auto target_bit_shift = who == Color::White
                ? CASTLING_STATE_WHITE_TARGET : CASTLING_STATE_BLACK_TARGET;

            CastlingEligibility result = EitherSideEligible;
            return makeCastlingEligibilityFromInt (
                    (target_bits >> target_bit_shift) & CASTLE_ONE_COLOR_MASK
            );
        }

        void setCastleState (Color who, CastlingEligibility castling_states) noexcept
        {
            uint8_t castling_bits = toInt (castling_states);
            std::size_t bit_number = who == Color::White
                ? CASTLING_STATE_WHITE_TARGET : CASTLING_STATE_BLACK_TARGET;
            std::size_t mask = CASTLE_ONE_COLOR_MASK << bit_number;

            auto metadataBits = getMetadataBits();
            metadataBits &= ~mask;
            metadataBits |= castling_bits << bit_number;
            setMetadataBits (metadataBits);
        }

        [[nodiscard]] auto currentTurn() const noexcept -> Color
        {
            auto bits = getMetadataBits();
            auto index = gsl::narrow_cast<int8_t> (bits & (CURRENT_TURN_MASK << CURRENT_TURN_BIT));
            return colorFromColorIndex (index);
        }

        [[nodiscard]] auto enPassantTargets() const noexcept -> EnPassantTargets
        {
            EnPassantTargets result = {
                enPassantTarget (Color::White),
                enPassantTarget (Color::Black)
            };
            return result;
        }

        [[nodiscard]] auto getMetadataBits() const noexcept -> std::uint16_t
        {
            return gsl::narrow_cast<uint16_t> (my_code & 0xffff);
        }

        [[nodiscard]] auto asString() const noexcept -> string
        {
            std::bitset<64> bits { my_code };

            return bits.to_string();
        }

        [[nodiscard]] auto hashCode() const noexcept -> BoardHashCode
        {
            return my_code;
        }

        friend auto operator== (const BoardCode& first, const BoardCode& second) noexcept -> bool
        {
            return first.my_code == second.my_code;
        }

        friend auto operator!= (const BoardCode& first, const BoardCode& second) noexcept -> bool
        {
            return !(first == second);
        }

        friend auto operator<< (std::ostream& os, const BoardCode& code) -> std::ostream&;

        [[nodiscard]] auto withMove (const Board& board, Move move) const noexcept -> BoardCode
        {
            auto copy = *this;
            copy.applyMove (board, move);
            return copy;
        }

        void applyMove (const Board& board, Move move) noexcept;

        [[nodiscard]] auto numberOfSetBits() const -> std::size_t;

    private:
        // Private and only used for initialization.
        BoardCode();

        void setMetadataBits (uint16_t new_metadata)
        {
            my_code = (my_code & 0xfffFFFFffff0000ULL) | new_metadata;
        }

    private:
        // 48-bits Zobrist hash + a few bits for the metadata.
        std::uint64_t my_code = 0;
    };
}

namespace std
{
    template<>
    struct hash<wisdom::BoardCode>
    {
        auto operator() (const wisdom::BoardCode& code) const -> std::size_t
        {
            return code.hashCode();
        }
    };
}

#endif //WISDOM_CHESS_BOARD_CODE_HPP
