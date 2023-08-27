#ifndef WISDOM_CHESS_BOARD_CODE_HPP
#define WISDOM_CHESS_BOARD_CODE_HPP

#include "global.hpp"
#include "piece.hpp"
#include "coord.hpp"
#include "move.hpp"

namespace wisdom
{
    using EnPassantTargets = array<Coord, Num_Players>;

    using BoardHashCode = std::size_t;

    class Board;
    class BoardBuilder;

    class BoardCode final
    {
    private:
        // 3 Bits per piece type, +1 for color (special case: no piece == 0):
        static constexpr int Bits_Per_Piece = 4;

        // 4 bits per square * 64 squares = 256 bits
        static constexpr int Total_Piece_Bits = Bits_Per_Piece * Num_Rows * Num_Columns;

        static constexpr int Bits_Per_Pieces_Element = sizeof(BoardHashCode) * 8;
        static constexpr int Num_Pieces_Elements = Total_Piece_Bits / Bits_Per_Pieces_Element;

        using PiecesBitset = array<bitset<Bits_Per_Pieces_Element>, Num_Pieces_Elements>;

        static constexpr int Metadata_Total_Bits = 16;

        using MetadataBitset = bitset<Metadata_Total_Bits>;

        static std::hash<MetadataBitset> metadata_hash_fn;

        enum PieceBitPositions : std::size_t {
            PIECE_MASK = 0b1111ULL,
        };

        enum AncillaryBitPositions : std::size_t {
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

        void addPiece (Coord coord, ColoredPiece piece)
        {
            Color color = piece.color();
            Piece type = piece.type();

            std::size_t new_value = piece == Piece_And_Color_None
                ? 0 : pieceIndex (type) | (colorIndex (color) << 3);
            assert (new_value < 16);

            auto position = piecesBitsetPositionFromCoord (coord);
            auto new_bits = new_value << position.index_within_element;

            my_pieces[position.element] &= ~(PIECE_MASK << position.index_within_element);
            my_pieces[position.element] |= new_bits;
        }

        void removePiece (Coord coord)
        {
            return addPiece (coord, Piece_And_Color_None);
        }

        void setEnPassantTarget (Color color, Coord coord)
        {
            std::size_t target_bit_shift = color == Color::White
                ? EN_PASSANT_WHITE_TARGET : EN_PASSANT_BLACK_TARGET;
            auto coord_bits = Column<std::size_t> (coord);
            coord_bits |= (coord == No_En_Passant_Coord) ? 0 : EN_PASSANT_PRESENT;
            coord_bits <<= target_bit_shift;

            assert (coord == No_En_Passant_Coord || (
                    Row (coord) == (color == Color::White ? White_En_Passant_Row : Black_En_Passant_Row)));

            // clear both targets initially. There can be only one at a given time.
            my_metadata &= ~(EN_PASSANT_MASK << EN_PASSANT_WHITE_TARGET);
            my_metadata |= coord_bits;
        }

        [[nodiscard]] auto enPassantTarget (Color vulnerable_color) const noexcept -> Coord
        {
            std::size_t target_bits = my_metadata.to_ulong();
            std::size_t target_bit_shift = vulnerable_color == Color::White
                ? EN_PASSANT_WHITE_TARGET : EN_PASSANT_BLACK_TARGET;

            target_bits &= EN_PASSANT_MASK << EN_PASSANT_WHITE_TARGET;
            target_bits >>= target_bit_shift;
            auto col = gsl::narrow<int8_t> (target_bits & 0x7);
            auto is_present = (bool)(target_bits & EN_PASSANT_PRESENT);
            auto row = vulnerable_color == Color::White ? White_En_Passant_Row : Black_En_Passant_Row;
            return is_present ? makeCoord (row, col) : No_En_Passant_Coord;
        }

        void setCurrentTurn (Color who)
        {
            auto bits = colorIndex (who);
            auto current_turn_bit = bits & (CURRENT_TURN_MASK << CURRENT_TURN_BIT);
            my_metadata &= ~(CURRENT_TURN_MASK << CURRENT_TURN_BIT);
            my_metadata |= current_turn_bit;
        }

        [[nodiscard]] auto castleState (Color who) const -> CastlingEligibility
        {
            std::size_t target_bits = my_metadata.to_ulong();
            std::size_t target_bit_shift = who == Color::White
                ? CASTLING_STATE_WHITE_TARGET : CASTLING_STATE_BLACK_TARGET;
            CastlingEligibility result = CastlingEligible::EitherSideEligible;
            auto new_value = gsl::narrow_cast<uint8_t> (
                    (target_bits >> target_bit_shift) & CASTLE_ONE_COLOR_MASK
            );
            result.set_underlying_value (new_value);
            return result;
        }

        void setCastleState (Color who, CastlingEligibility castling_states)
        {
            uint8_t castling_bits = castling_states.underlying_value();
            std::size_t bit_number = who == Color::White
                ? CASTLING_STATE_WHITE_TARGET : CASTLING_STATE_BLACK_TARGET;
            std::size_t mask = CASTLE_ONE_COLOR_MASK << bit_number;

            my_metadata &= ~mask;
            my_metadata |= castling_bits << bit_number;
        }

        [[nodiscard]] auto currentTurn() const -> Color
        {
            auto bits = my_metadata.to_ulong();
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

        [[nodiscard]] auto asString() const -> string
        {
            std::string result;

            result.reserve (Total_Piece_Bits + Metadata_Total_Bits);

            // Most-significant bits first (big-endian):
            for (auto i = my_pieces.crbegin(); i != my_pieces.crend(); i++)
                result += i->to_string();

            result += my_metadata.to_string();
            return result;
        }

        [[nodiscard]] auto getPiecesBitsetRef() const& -> const PiecesBitset&
        {
            return my_pieces;
        }
        void getPiecesBitsetRef() const&& = delete;

        // Return by value:
        [[nodiscard]] auto getMetadataBits() -> MetadataBitset
        {
            return my_metadata;
        }

        [[nodiscard]] auto hashCode() const -> BoardHashCode
        {
            BoardHashCode result = metadata_hash_fn (my_metadata);

            for (const auto& bits : my_pieces)
                result = (result * 31) ^ bits.to_ullong();

            return result;
        }

        friend auto operator== (const BoardCode& first, const BoardCode& second) -> bool
        {
            return first.my_pieces == second.my_pieces &&
                first.my_metadata == second.my_metadata;
        }

        friend auto operator!= (const BoardCode& first, const BoardCode& second) -> bool
        {
            return !(first == second);
        }

        friend auto operator<< (std::ostream& os, const BoardCode& code) -> std::ostream&;

        [[nodiscard]] auto withMove (const Board& board, Move move) const -> BoardCode
        {
            auto copy = *this;
            copy.applyMove (board, move);
            return copy;
        }

        void applyMove (const Board& board, Move move);

        [[nodiscard]] auto countOnes() const -> std::size_t;

    private:
        // Private and only used for initialization.
        BoardCode() = default;

        struct PiecesBitsetPosition
        {
            std::size_t element;
            std::size_t index_within_element;
        };

        static auto piecesBitsetPositionFromCoord (Coord coord) -> PiecesBitsetPosition
        {
            auto bit_index = coordIndex<std::size_t> (coord) * Bits_Per_Piece;

            return PiecesBitsetPosition {
                .element = bit_index / Bits_Per_Pieces_Element,
                .index_within_element = bit_index % Bits_Per_Pieces_Element,
            };
        }

    private:
        PiecesBitset my_pieces;
        MetadataBitset my_metadata;
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
