#pragma once

#include "global.hpp"
#include "coord.hpp"
#include "piece.hpp"

namespace wisdom
{
    enum class CastlingIneligible : uint8_t
    {
        Kingside = 1,
        Queenside = 2,
    };
}

namespace type_safe
{
    template <> struct flag_set_traits<wisdom::CastlingIneligible> : std::true_type
    {
        static constexpr auto size() -> std::size_t
        {
            return 2;
        }
    };
}

namespace wisdom
{
    using CastlingEligibility = type_safe::flag_set<wisdom::CastlingIneligible>;

    inline constexpr CastlingEligibility Either_Side_Eligible = type_safe::noflag;
    inline constexpr CastlingEligibility Neither_Side_Eligible
        = CastlingIneligible::Kingside | CastlingIneligible::Queenside;

    inline constexpr auto makeCastlingEligibilityFromInt (unsigned int flags) -> CastlingEligibility
    {
        CastlingEligibility result = Either_Side_Eligible;
        assert (flags <= (0x1 + 0x2));
        if (flags & 0x1)
            result.set (CastlingIneligible::Kingside);
        if (flags & 0x2)
            result.set (CastlingIneligible::Queenside);
        return result;
    }

    // Send the move to the ostream.
    inline auto operator<< (std::ostream& os, const CastlingEligibility& value) -> std::ostream&
    {
        std::string result = "{ Kingside: ";

        result += value.is_set (CastlingIneligible::Kingside) ?
            "not eligible, " : "eligible, ";
        result += "Queenside: ";
        result += value.is_set (CastlingIneligible::Queenside) ?
            "not eligible" : "eligible";
        result += " }";

        os << result;
        return os;
    }

    template <typename IntegerType = uint8_t>
    inline constexpr auto toInt (CastlingEligibility eligibility) -> IntegerType
    {
        IntegerType result = 0;
        if (eligibility.is_set (CastlingIneligible::Kingside))
            result |= 0x1;
        if (eligibility.is_set (CastlingIneligible::Queenside))
            result |= 0x2;
        return result;
    }

    class Board;

    enum class MoveCategory : int8_t
    {
        Default = 0,
        NormalCapturing = 1,
        EnPassant = 2,
        Castling = 3,
    };

    class ParseMoveException : public Error
    {
    public:
        explicit ParseMoveException (const string& message) : Error { message }
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
                .src = gsl::narrow_cast<int8_t> (src.index()),
                .dst = gsl::narrow_cast<int8_t> (dst.index()),
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

