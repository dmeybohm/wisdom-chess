#ifndef WISDOM_CHESS_MOVE_HPP
#define WISDOM_CHESS_MOVE_HPP

#include "global.hpp"
#include "coord.hpp"
#include "piece.hpp"

namespace wisdom
{
    enum class CastlingEligible : uint8_t
    {
        EitherSideEligible = 0b000U,
        KingsideIneligible = 0b001U,
        QueensideIneligible = 0b010U,
        BothSidesIneligible = static_cast<uint8_t>(CastlingEligible::KingsideIneligible) | static_cast<uint8_t>(CastlingEligible::QueensideIneligible)
    };
    using CastlingEligibility = flags::flags<wisdom::CastlingEligible>;
}

ALLOW_FLAGS_FOR_ENUM(wisdom::CastlingEligible);

namespace wisdom
{
    class Board;

    static constexpr size_t Max_Packed_Capacity_In_Move = 0x001FFFFFL; // 21 bit max

    enum class MoveCategory : int8_t
    {
        Default = 0,
        NormalCapturing = 1,
        EnPassant = 2,
        Castling = 3,
        PackedCapacity = 4,
    };

    class ParseMoveException : public Error
    {
    public:
        explicit ParseMoveException (const string& message) : Error { message }
        {}
    };

    class MoveConsistencyProblem : public Error
    {
    public:
        MoveConsistencyProblem() : Error ("Move consistency error.")
        {}

        explicit MoveConsistencyProblem (string extra_info) :
            Error ("Move consistency error.", std::move (extra_info) )
        {}
    };

    [[nodiscard]] constexpr auto moveCategoryFromInt (int source) -> MoveCategory
    {
        assert (source <= 4);
        return static_cast<MoveCategory> (source);
    }

    [[nodiscard]] constexpr auto toInt (MoveCategory move_category) -> int
    {
        return static_cast<int> (move_category);
    }

    [[nodiscard]] constexpr auto toInt8 (MoveCategory move_category) -> int
    {
        return static_cast<int8_t> (move_category);
    }

    struct Move
    {
        int8_t src;
        int8_t dst;
        int8_t promoted_piece;
        int8_t move_category;

    public:
        [[nodiscard]] static constexpr auto make (Coord src, Coord dst) noexcept
        -> Move
        {
            return Move {
                .src = gsl::narrow_cast<int8_t> (coordIndex (src)),
                .dst = gsl::narrow_cast<int8_t> (coordIndex (dst)),
                .promoted_piece = gsl::narrow_cast<int8_t> (0),
                .move_category = gsl::narrow_cast<int8_t> (0),
            };
        }

        [[nodiscard]] static constexpr auto make (int src_row, int src_col,
                                                  int dst_row, int dst_col) noexcept
            -> Move
        {
            Coord src = makeCoord (src_row, src_col);
            Coord dst = makeCoord (dst_row, dst_col);

            return make (src, dst);
        }

        [[nodiscard]] static constexpr auto makeNormalCapturing (int src_row, int src_col,
                                                                   int dst_row, int dst_col) noexcept
            -> Move
        {
            Move move = Move::make (src_row, src_col, dst_row, dst_col);
            move.move_category = toInt8 (MoveCategory::NormalCapturing);
            return move;
        }

        [[nodiscard]] static constexpr auto makeCastling (int src_row, int src_col,
                                                           int dst_row, int dst_col) noexcept
            -> Move
        {
            Move move = Move::make (src_row, src_col, dst_row, dst_col);
            move.move_category = toInt8 (MoveCategory::Castling);
            return move;
        }

        [[nodiscard]] static constexpr auto makeCastling (Coord src, Coord dst) noexcept
            -> Move
        {
            return Move::makeCastling (Row (src), Column (src), Row (dst), Column (dst));
        }

        [[nodiscard]] static constexpr auto makeEnPassant (int src_row, int src_col,
                                                           int dst_row, int dst_col) noexcept
            -> Move
        {
            Move move = make (src_row, src_col, dst_row, dst_col);
            move.move_category = toInt8 (MoveCategory::EnPassant);
            return move;
        }

        [[nodiscard]] static constexpr auto makeEnPassant (Coord src, Coord dst) noexcept
            -> Move
        {
            return Move::makeEnPassant (Row (src), Column (src), Row (dst), Column (dst));
        }

        // Pack a 28-bit integer into the move. Used for move lists to store the list length.
        [[nodiscard]] static constexpr auto makeAsPackedCapacity (size_t size) noexcept
            -> Move
        {
            assert (size <= Max_Packed_Capacity_In_Move);
            return {
                .src = gsl::narrow_cast<int8_t> (size & 0x7f),
                .dst = gsl::narrow_cast<int8_t> ((size >> 7) & 0x7f),
                .promoted_piece = gsl::narrow_cast<int8_t> ((size >> 14) & 0x7f),
                .move_category = toInt8 (MoveCategory::PackedCapacity),
            };
        }

        [[nodiscard]] static constexpr auto fromInt (int packed_move)
            -> Move
        {
            return Move {
                .src = gsl::narrow<int8_t> (packed_move & 0x7f),
                .dst = gsl::narrow<int8_t> ((packed_move >> 8) & 0x7f),
                .promoted_piece = gsl::narrow<int8_t> ((packed_move >> 16) & 0x7f),
                .move_category = gsl::narrow<int8_t> ((packed_move >> 24) & 0x7f),
            };
        }

        [[nodiscard]] auto toInt() const -> int
        {
            return src | (dst << 8) | (promoted_piece << 16) | (move_category << 24);
        }

        [[nodiscard]] constexpr auto getSrc() const -> Coord
        {
            return Coord::fromIndex (src);
        }

        [[nodiscard]] constexpr auto getDst() const -> Coord
        {
            return Coord::fromIndex (dst);
        }

        [[nodiscard]] constexpr auto withPromotion (ColoredPiece piece) const noexcept
            -> Move
        {
            assert (piece != Piece_And_Color_None);
            Move result = *this;
            auto promoted_piece_type = toInt8 (pieceType (piece));
            auto promoted_color_index = colorIndex (pieceColor (piece));
            result.promoted_piece = gsl::narrow_cast<int8_t> (
                (promoted_color_index << 4) | (promoted_piece_type & 0xf)
            );
            return result;
        }

        [[nodiscard]] constexpr auto withCapture() const noexcept
            -> Move
        {
            assert (move_category == toInt8 (MoveCategory::Default));

            Move result = Move::make (getSrc(), getDst());
            result.move_category = toInt8 (MoveCategory::NormalCapturing);
            return result;
        }

        [[nodiscard]] constexpr auto getMoveCategory() const -> MoveCategory
        {
            return moveCategoryFromInt (move_category);
        }

        [[nodiscard]] constexpr auto isNormalCapturing() const -> bool
        {
            return getMoveCategory() == MoveCategory::NormalCapturing;
        }

        [[nodiscard]] constexpr auto isPromoting() const -> bool
        {
            return promoted_piece != toInt8 (Piece_And_Color_None);
        }

        [[nodiscard]] constexpr auto getPromotedPiece() const -> ColoredPiece
        {
            auto piece = pieceFromInt (promoted_piece & 0xf);
            auto color = piece == Piece::None ? Color::None :
                (promoted_piece & 0x10) == 0x10 ? Color::Black : Color::White;
            return ColoredPiece::make (color, piece);
        }

        [[nodiscard]] constexpr auto isEnPassant() const noexcept
            -> bool
        {
            return getMoveCategory() == MoveCategory::EnPassant;
        }

        [[nodiscard]] constexpr auto isAnyCapturing() const noexcept
            -> bool
        {
            return isNormalCapturing() || isEnPassant();
        }

        [[nodiscard]] constexpr auto isCastling() const noexcept
            -> bool
        {
            return getMoveCategory() == MoveCategory::Castling;
        }

        [[nodiscard]] constexpr auto isCastlingOnKingside() const noexcept
            -> bool
        {
            return isCastling() && Column (getDst()) == Kingside_Castled_King_Column;
        }

        [[nodiscard]] constexpr auto toUnpackedCapacity() const noexcept
            -> size_t
        {
            assert (getMoveCategory() == MoveCategory::PackedCapacity);
            return src |
                (dst << 7) |
                (promoted_piece << 14);
        }
    };

    static_assert (sizeof (Move) == 4);
    static_assert (std::is_trivial_v<Move>);

    template <class IntegerType = int8_t>
    [[nodiscard]] constexpr auto castlingRowForColor (Color who) -> IntegerType
    {
        static_assert (std::is_integral_v<IntegerType>);
        return gsl::narrow_cast<IntegerType> (
                who == Color::White ? Last_Row : First_Row
        );
    }

    using PlayerCastleState = array<CastlingEligibility, Num_Players>;

    [[nodiscard]] constexpr auto moveEquals (Move a, Move b) noexcept
        -> bool
    {
        return a.src == b.src &&
               a.dst == b.dst &&
               a.promoted_piece == b.promoted_piece &&
               a.move_category == b.move_category;
    }

    constexpr auto operator== (Move a, Move b) noexcept
        -> bool
    {
        return moveEquals (a, b);
    }

    constexpr auto operator!= (Move a, Move b) noexcept
        -> bool
    {
        return !moveEquals (a, b);
    }

    // Parse a move. Returns empty if the parse failed.
    [[nodiscard]] auto moveParseOptional (const string& str, Color who) -> optional<Move>;

    // The coordinate for the taken pawn.
    [[nodiscard]] auto enPassantTakenPawnCoord (Coord src, Coord dst) -> Coord;

    // Map source/dest coordinate to corresponding move (en passant, castling, etc)
    // This doesn't check whether the move is legal or not completely - just gets what the
    // user is intending.
    [[nodiscard]] auto mapCoordinatesToMove (const Board& board, Color who,
                                             Coord src, Coord dst,
                                             optional<Piece> promoted_piece = {})
        -> optional<Move>;

    // Parse a move. Throws an exception if it could not parse the move.
    [[nodiscard]] auto moveParse (const string& str, Color color = Color::None) -> Move;

    // Convert the move to a string.
    [[nodiscard]] auto asString (const Move& move) -> string;

    // Send the move to the ostream.
    auto operator<< (std::ostream& os, const Move& value) -> std::ostream&;
}

#endif // WISDOM_CHESS_MOVE_HPP
