#pragma once

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/piece.hpp"
#include "wisdom-chess/engine/coord.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/random.hpp"

namespace wisdom
{
    using BoardHashCode = std::uint64_t;

    class Board;
    class BoardBuilder;

    using BoardCodeArray = array<uint64_t, (Num_Players + 1) * Num_Piece_Types * Num_Squares>;

    [[nodiscard]] consteval auto
    initializeBoardCodes()
        -> BoardCodeArray
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

    inline constexpr BoardCodeArray Hash_Code_Table = initializeBoardCodes();

    inline constexpr int Total_Metadata_Bits = 16;

    [[nodiscard]] constexpr auto
    boardCodeHash (Coord coord, ColoredPiece piece)
        -> std::uint64_t
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
            EN_PASSANT_TARGET_BIT = 1,
            CASTLING_STATE_WHITE_BIT = 7,
            CASTLING_STATE_BLACK_BIT = 10,
            CURRENT_TURN_MASK = 0b1,
            EN_PASSANT_MASK = 0b11111,
            CASTLE_ONE_COLOR_MASK = 0b11,
            EN_PASSANT_PRESENT = 0b1000,
            EN_PASSANT_IS_WHITE = 0b10000,
        };

    public:
        explicit BoardCode (const Board& board);

        [[nodiscard]] static auto
        fromBoard (const Board& board)
            -> BoardCode;

        [[nodiscard]] static auto
        fromBoardBuilder (const BoardBuilder& builder)
            -> BoardCode;

        [[nodiscard]] static auto
        fromDefaultPosition()
            -> BoardCode;

        [[nodiscard]] static auto
        fromEmptyBoard()
            -> BoardCode;

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
            std::size_t target_bit_shift = EN_PASSANT_TARGET_BIT;
            auto coord_bits = coord.column<std::size_t>()
                | EN_PASSANT_PRESENT
                | (color == Color::White ? EN_PASSANT_IS_WHITE : 0);
            coord_bits <<= target_bit_shift;

            assert (
                coord.row() == (color == Color::White
                                    ? White_En_Passant_Row : Black_En_Passant_Row)
            );

            // clear both targets initially. There can be only one at a given time.
            auto metadata = getMetadataBits();
            metadata &= ~(EN_PASSANT_MASK << EN_PASSANT_TARGET_BIT);
            metadata |= coord_bits;
            setMetadataBits (metadata);
        }

        void clearEnPassantTarget() noexcept
        {
            auto metadata = getMetadataBits();
            metadata &= ~(EN_PASSANT_MASK << EN_PASSANT_TARGET_BIT);
            setMetadataBits (metadata);
        }

        [[nodiscard]] auto
        enPassantTarget () const noexcept
            -> optional<EnPassantTarget>
        {
            auto target_bits = getMetadataBits();
            auto target_bit_shift = EN_PASSANT_TARGET_BIT;

            target_bits &= EN_PASSANT_MASK << EN_PASSANT_TARGET_BIT;
            target_bits >>= target_bit_shift;
            auto col = narrow<int8_t> (target_bits & 0x7);
            bool is_present = ((target_bits & EN_PASSANT_PRESENT) > 0);
            Color vulnerable_color = ((target_bits & EN_PASSANT_IS_WHITE) > 0)
                ? Color::White
                : Color::Black;
            auto row = vulnerable_color == Color::White 
                ? White_En_Passant_Row 
                : Black_En_Passant_Row;

            return is_present
                ? std::make_optional (EnPassantTarget {
                      .coord = makeCoord (row, col),
                      .vulnerable_color = vulnerable_color
                  })
                : nullopt;
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

        [[nodiscard]] auto
        castleState (Color who) const
            -> CastlingEligibility
        {
            auto target_bits = getMetadataBits();
            auto target_bit_shift = who == Color::White 
                ? CASTLING_STATE_WHITE_BIT
                : CASTLING_STATE_BLACK_BIT;

            return makeCastlingEligibilityFromInt (
                (target_bits >> target_bit_shift) & CASTLE_ONE_COLOR_MASK
            );
        }

        void setCastleState (Color who, CastlingEligibility castling_states) noexcept
        {
            uint8_t castling_bits = toInt (castling_states);
            std::size_t bit_number = who == Color::White 
                ? CASTLING_STATE_WHITE_BIT
                : CASTLING_STATE_BLACK_BIT;
            std::size_t mask = CASTLE_ONE_COLOR_MASK << bit_number;

            auto metadataBits = getMetadataBits();
            metadataBits &= ~mask;
            metadataBits |= castling_bits << bit_number;
            setMetadataBits (metadataBits);
        }

        [[nodiscard]] auto
        currentTurn() const noexcept
            -> Color
        {
            auto bits = getMetadataBits();
            auto index = narrow_cast<int8_t> (
                bits & (CURRENT_TURN_MASK << CURRENT_TURN_BIT)
            );
            return colorFromColorIndex (index);
        }

        [[nodiscard]] auto
        getMetadataBits() const noexcept
            -> std::uint16_t
        {
            return narrow_cast<uint16_t> (my_code & 0xffff);
        }

        [[nodiscard]] auto
        asString() const noexcept
            -> string;

        [[nodiscard]] auto
        hashCode() const noexcept
            -> BoardHashCode
        {
            return my_code;
        }

        friend auto
        operator== (const BoardCode& first, const BoardCode& second) noexcept
            -> bool
        {
            return first.my_code == second.my_code;
        }

        friend auto
        operator!= (const BoardCode& first, const BoardCode& second) noexcept
            -> bool
        {
            return !(first == second);
        }

        friend auto
        operator<< (std::ostream& os, const BoardCode& code)
            -> std::ostream&;

        [[nodiscard]] auto
        withMove (const Board& board, Move move) const noexcept
            -> BoardCode
        {
            auto copy = *this;
            copy.applyMove (board, move);
            return copy;
        }

        void applyMove (const Board& board, Move move) noexcept;

        [[nodiscard]] auto
        numberOfSetBits() const
            -> std::size_t;

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
    template <>
    struct hash<wisdom::BoardCode>
    {
        auto
        operator() (const wisdom::BoardCode& code) const noexcept
            -> std::size_t
        {
            return code.hashCode();
        }
    };
}
