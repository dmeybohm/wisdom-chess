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

    inline constexpr std::size_t Zobrist_Table_Size = Num_Players * Num_Piece_Types * Num_Squares;

    using BoardCodeArray = array<uint64_t, Zobrist_Table_Size>;

    [[nodiscard]] constexpr auto
    zobristPieceIndex (Color piece_color, Piece piece_type) noexcept
        -> int
    {
        return colorIndex (piece_color) * Num_Piece_Types + (pieceIndex (piece_type) - 1);
    }

    [[nodiscard]] consteval auto
    initializeBoardCodes() noexcept
        -> BoardCodeArray
    {
        BoardCodeArray code_array {};
        CompileTimeRandom random;

        for (auto color : { Color::White, Color::Black })
        {
            for (auto piece : { Piece::Pawn, Piece::Knight, Piece::Bishop,
                                Piece::Rook, Piece::Queen, Piece::King })
            {
                auto piece_index = zobristPieceIndex (color, piece);
                for (auto square = 0; square < Num_Squares; square++)
                    code_array[piece_index * Num_Squares + square] = getCompileTimeRandom48 (&random);
            }
        }

        return code_array;
    }

    inline constexpr BoardCodeArray Hash_Code_Table = initializeBoardCodes();

    inline constexpr int Total_Metadata_Bits = 16;

    // The low Total_Metadata_Bits hold the turn, castling and en passant
    // state; the Zobrist hash of the pieces occupies the bits above.
    inline constexpr std::uint64_t Metadata_Mask = (std::uint64_t { 1 } << Total_Metadata_Bits) - 1;
    inline constexpr std::uint64_t Piece_Hash_Mask = ~Metadata_Mask;

    [[nodiscard]] constexpr auto
    boardCodeHash (Coord coord, ColoredPiece piece) noexcept
        -> std::uint64_t
    {
        auto coord_index = coord.index();
        auto piece_index = zobristPieceIndex (piece.color(), piece.type());

        return Hash_Code_Table[piece_index * Num_Squares + coord_index] << Total_Metadata_Bits;
    }

    class BoardCode final
    {
    private:
        enum MetadataBits : std::size_t
        {
            CURRENT_TURN_BIT = 0,
            LEGAL_EN_PASSANT_TARGET_BIT = 1,
            ILLEGAL_EN_PASSANT_TARGET_BIT = 6,
            CASTLING_STATE_WHITE_BIT = 11,
            CASTLING_STATE_BLACK_BIT = 13,
            CURRENT_TURN_MASK = 0b1,
            EN_PASSANT_MASK = 0b11111,
            CASTLE_ONE_COLOR_MASK = 0b11,
            EN_PASSANT_PRESENT = 0b1000,
            EN_PASSANT_IS_WHITE = 0b10000,
        };

        static constexpr std::uint16_t En_Passant_Targets_Mask =
            (EN_PASSANT_MASK << LEGAL_EN_PASSANT_TARGET_BIT)
            | (EN_PASSANT_MASK << ILLEGAL_EN_PASSANT_TARGET_BIT);

    public:
        explicit BoardCode (const Board& board) noexcept;

        [[nodiscard]] static auto
        fromBoard (const Board& board) noexcept
            -> BoardCode;

        [[nodiscard]] static auto
        fromBoardBuilder (const BoardBuilder& builder) noexcept
            -> BoardCode;

        [[nodiscard]] static auto
        fromDefaultPosition()
            -> BoardCode;

        [[nodiscard]] static auto
        fromEmptyBoard() noexcept
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

        // Store the en passant target, recording whether a legal capture
        // of it exists. At most one target is stored at a time.
        void setEnPassantTarget (
            Color color,
            Coord coord,
            EnPassantTargetState state
        ) noexcept
        {
            EXPECTS(
                coord.row() == (color == Color::White
                                    ? White_En_Passant_Row : Black_En_Passant_Row)
            );

            auto coord_bits = coord.column<std::size_t>()
                | EN_PASSANT_PRESENT
                | (color == Color::White
                       ? std::size_t { EN_PASSANT_IS_WHITE }
                       : std::size_t { 0 });
            std::size_t target_bit_shift = state == EnPassantTargetState::Legal
                ? LEGAL_EN_PASSANT_TARGET_BIT
                : ILLEGAL_EN_PASSANT_TARGET_BIT;

            auto metadata = getMetadataBits();
            metadata &= ~En_Passant_Targets_Mask;
            metadata |= coord_bits << target_bit_shift;
            setMetadataBits (metadata);
        }

        void clearEnPassantTarget() noexcept
        {
            auto metadata = getMetadataBits();
            metadata &= ~En_Passant_Targets_Mask;
            setMetadataBits (metadata);
        }

        // The en passant target, whether or not a legal capture of it exists.
        [[nodiscard]] auto
        getAnyEnPassantTarget() const noexcept
            -> optional<EnPassantTarget>
        {
            auto metadata = getMetadataBits();
            std::size_t target_bits = (metadata >> LEGAL_EN_PASSANT_TARGET_BIT) & EN_PASSANT_MASK;
            if ((target_bits & EN_PASSANT_PRESENT) == 0)
                target_bits = (metadata >> ILLEGAL_EN_PASSANT_TARGET_BIT) & EN_PASSANT_MASK;

            return decodeEnPassantTarget (target_bits);
        }

        // The en passant target, if a legal capture of it exists.
        [[nodiscard]] auto
        getLegalEnPassantTarget() const noexcept
            -> optional<EnPassantTarget>
        {
            auto metadata = getMetadataBits();
            return decodeEnPassantTarget (
                (metadata >> LEGAL_EN_PASSANT_TARGET_BIT) & EN_PASSANT_MASK
            );
        }

        // The code with any en passant target that has no legal capture
        // left out.
        [[nodiscard]] auto
        withoutIllegalEnPassantTarget() const noexcept
            -> BoardCode
        {
            BoardCode result = *this;
            result.my_code &= ~(std::uint64_t { EN_PASSANT_MASK } << ILLEGAL_EN_PASSANT_TARGET_BIT);
            return result;
        }

        void setCurrentTurn (Color who) noexcept
        {
            auto bits = colorIndex (who);
            auto current_turn_bit = bits & (CURRENT_TURN_MASK << CURRENT_TURN_BIT);
            auto metadata_bits = getMetadataBits();

            metadata_bits &= ~(CURRENT_TURN_MASK << CURRENT_TURN_BIT);
            metadata_bits |= current_turn_bit;
            setMetadataBits (metadata_bits);
        }

        [[nodiscard]] auto
        getCastleState (Color who) const noexcept
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
            uint8_t castling_bits = toUint (castling_states);
            std::size_t bit_number = who == Color::White
                ? CASTLING_STATE_WHITE_BIT
                : CASTLING_STATE_BLACK_BIT;
            std::size_t mask = CASTLE_ONE_COLOR_MASK << bit_number;

            auto metadata_bits = getMetadataBits();
            metadata_bits &= ~mask;
            metadata_bits |= castling_bits << bit_number;
            setMetadataBits (metadata_bits);
        }

        [[nodiscard]] auto
        getCurrentTurn() const noexcept
            -> Color
        {
            auto bits = getMetadataBits();
            auto index = narrow_debug<int8_t> (
                bits & (CURRENT_TURN_MASK << CURRENT_TURN_BIT)
            );
            return colorFromColorIndex (index);
        }

        [[nodiscard]] auto
        getMetadataBits() const noexcept
            -> std::uint16_t
        {
            return narrow_debug<uint16_t> (my_code & Metadata_Mask);
        }

        [[nodiscard]] auto
        asString() const
            -> string;

        [[nodiscard]] constexpr auto
        getHashCode() const noexcept
            -> BoardHashCode
        {
            return my_code;
        }

        [[nodiscard]] friend auto
        operator== (const BoardCode& first, const BoardCode& second) noexcept
            -> bool
        {
            return first.my_code == second.my_code;
        }

        [[nodiscard]] friend auto
        operator!= (const BoardCode& first, const BoardCode& second) noexcept
            -> bool
        {
            return !(first == second);
        }

        friend auto
        operator<< (std::ostream& os, const BoardCode& code)
            -> std::ostream&;

        void applyMove (const Board& board, Move move) noexcept;

    private:
        [[nodiscard]] static auto
        decodeEnPassantTarget (std::size_t target_bits) noexcept
            -> optional<EnPassantTarget>
        {
            if ((target_bits & EN_PASSANT_PRESENT) == 0)
                return nullopt;

            auto col = narrow<int8_t> (target_bits & 0x7);
            Color vulnerable_color = ((target_bits & EN_PASSANT_IS_WHITE) > 0)
                ? Color::White
                : Color::Black;
            auto row = vulnerable_color == Color::White
                ? White_En_Passant_Row
                : Black_En_Passant_Row;

            return EnPassantTarget {
                .coord = makeCoord (row, col),
                .vulnerable_color = vulnerable_color
            };
        }

        // Private and only used for initialization.
        BoardCode();

        constexpr void setMetadataBits (uint16_t new_metadata) noexcept
        {
            my_code = (my_code & Piece_Hash_Mask) | new_metadata;
        }

    private:
        // 48-bit Zobrist hash of the pieces above 16 bits of metadata.
        std::uint64_t my_code = 0;
    };
}

namespace std
{
    template <>
    struct hash<wisdom::BoardCode>
    {
        [[nodiscard]] auto
        operator() (const wisdom::BoardCode& code) const noexcept
            -> std::size_t
        {
            return code.getHashCode();
        }
    };
}
