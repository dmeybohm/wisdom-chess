#pragma once

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/coord.hpp"
#include "wisdom-chess/engine/piece.hpp"
#include "wisdom-chess/engine/castling.hpp"

namespace wisdom
{
    struct EnPassantTarget
    {
        Coord coord;
        Color vulnerable_color;
    };


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
        explicit ParseMoveException (const string& message)
            : Error { message }
        {
        }
    };

    // Combined category/promotion encoding (fits in 4 bits):
    //   0: Default (non-capture, non-promote)
    //   1: Normal capture
    //   2: En passant
    //   3: Castling
    //   4: Promote Queen
    //   5: Promote Rook
    //   6: Promote Knight
    //   7: Promote Bishop
    //   8: Promote Queen + capture
    //   9: Promote Rook + capture
    //  10: Promote Knight + capture
    //  11: Promote Bishop + capture
    inline constexpr int8_t Combined_Default = 0;
    inline constexpr int8_t Combined_NormalCapture = 1;
    inline constexpr int8_t Combined_EnPassant = 2;
    inline constexpr int8_t Combined_Castling = 3;
    inline constexpr int8_t Combined_PromoteBase = 4;
    inline constexpr int8_t Combined_PromoteCaptureBase = 8;

    [[nodiscard]] constexpr auto
    promotionPieceOffset (Piece piece_type) noexcept
        -> int8_t
    {
        switch (piece_type)
        {
            case Piece::Queen:  return 0;
            case Piece::Rook:   return 1;
            case Piece::Knight: return 2;
            case Piece::Bishop: return 3;
            default:            return -1;
        }
    }

    [[nodiscard]] constexpr auto
    pieceFromPromotionOffset (int8_t offset) noexcept
        -> Piece
    {
        switch (offset)
        {
            case 0: return Piece::Queen;
            case 1: return Piece::Rook;
            case 2: return Piece::Knight;
            case 3: return Piece::Bishop;
            default: return Piece::None;
        }
    }

    struct Move
    {
        int8_t src;
        int8_t dst;
        int8_t combined;

    public:
        [[nodiscard]] static constexpr auto
        make (Coord src, Coord dst) noexcept
            -> Move
        {
            return Move {
                .src = narrow_cast<int8_t> (src.index()),
                .dst = narrow_cast<int8_t> (dst.index()),
                .combined = Combined_Default,
            };
        }

        [[nodiscard]] static constexpr auto
        make (
            int src_row,
            int src_col,
            int dst_row,
            int dst_col
        ) noexcept
            -> Move
        {
            Coord src = makeCoord (src_row, src_col);
            Coord dst = makeCoord (dst_row, dst_col);

            return make (src, dst);
        }

        [[nodiscard]] static constexpr auto
        makeNormalCapturing (
            int src_row,
            int src_col,
            int dst_row,
            int dst_col
        ) noexcept
            -> Move
        {
            Move move = Move::make (src_row, src_col, dst_row, dst_col);
            move.combined = Combined_NormalCapture;
            return move;
        }

        [[nodiscard]] static constexpr auto
        makeCastling (
            int src_row,
            int src_col,
            int dst_row,
            int dst_col
        ) noexcept
            -> Move
        {
            Move move = Move::make (
                src_row,
                src_col,
                dst_row,
                dst_col
            );
            move.combined = Combined_Castling;
            return move;
        }

        [[nodiscard]] static constexpr auto
        makeCastling (
            Coord src,
            Coord dst
        ) noexcept
            -> Move
        {
            return Move::makeCastling (
                src.row(),
                src.column(),
                dst.row(),
                dst.column()
            );
        }

        [[nodiscard]] static constexpr auto
        makeEnPassant (
            int src_row,
            int src_col,
            int dst_row,
            int dst_col
        ) noexcept
            -> Move
        {
            Move move = make (
                src_row,
                src_col,
                dst_row,
                dst_col
            );
            move.combined = Combined_EnPassant;
            return move;
        }

        [[nodiscard]] static constexpr auto
        makeEnPassant (
            Coord src,
            Coord dst
        ) noexcept
            -> Move
        {
            return Move::makeEnPassant (src.row(), src.column(), dst.row(), dst.column());
        }

        [[nodiscard]] static constexpr auto
        fromInt (int packed_move) -> Move
        {
            return Move {
                .src = narrow_cast<int8_t> (packed_move & 0x3f),
                .dst = narrow_cast<int8_t> ((packed_move >> 6) & 0x3f),
                .combined = narrow_cast<int8_t> ((packed_move >> 12) & 0xf),
            };
        }

        [[nodiscard]] constexpr auto
        toInt() const
            -> int
        {
            return (src & 0x3f) | ((dst & 0x3f) << 6) | ((combined & 0xf) << 12);
        }

        [[nodiscard]] constexpr auto
        getSrc() const
            -> Coord
        {
            return Coord::fromIndex (src);
        }

        [[nodiscard]] constexpr auto
        getDst() const
            -> Coord
        {
            return Coord::fromIndex (dst);
        }

        [[nodiscard]] constexpr auto
        withPromotion (Piece piece_type) const noexcept
            -> Move
        {
            assert (piece_type != Piece::None);
            Move result = *this;
            bool is_capture = (result.combined == Combined_NormalCapture);
            auto base = is_capture ? Combined_PromoteCaptureBase : Combined_PromoteBase;
            result.combined = narrow_cast<int8_t> (base + promotionPieceOffset (piece_type));
            return result;
        }

        [[nodiscard]] constexpr auto
        withCapture() const noexcept
            -> Move
        {
            assert (combined == Combined_Default);
            Move result = *this;
            result.combined = Combined_NormalCapture;
            return result;
        }

        [[nodiscard]] constexpr auto
        getMoveCategory() const
            -> MoveCategory
        {
            if (combined <= Combined_Castling)
                return static_cast<MoveCategory> (combined);
            if (combined >= Combined_PromoteCaptureBase)
                return MoveCategory::NormalCapturing;
            return MoveCategory::Default;
        }

        [[nodiscard]] constexpr auto
        isNormalCapturing() const
            -> bool
        {
            return combined == Combined_NormalCapture
                || combined >= Combined_PromoteCaptureBase;
        }

        [[nodiscard]] constexpr auto
        isPromoting() const
            -> bool
        {
            return combined >= Combined_PromoteBase;
        }

        [[nodiscard]] constexpr auto
        getPromotedPiece() const
            -> Piece
        {
            if (combined < Combined_PromoteBase)
                return Piece::None;
            int8_t offset = (combined >= Combined_PromoteCaptureBase)
                ? narrow_cast<int8_t> (combined - Combined_PromoteCaptureBase)
                : narrow_cast<int8_t> (combined - Combined_PromoteBase);
            return pieceFromPromotionOffset (offset);
        }

        [[nodiscard]] constexpr auto
        isEnPassant() const noexcept
            -> bool
        {
            return combined == Combined_EnPassant;
        }

        [[nodiscard]] constexpr auto
        isAnyCapturing() const noexcept
            -> bool
        {
            return isNormalCapturing() || isEnPassant();
        }

        [[nodiscard]] constexpr auto
        isCastling() const noexcept
            -> bool
        {
            return combined == Combined_Castling;
        }

        [[nodiscard]] constexpr auto
        isCastlingOnKingside() const noexcept
            -> bool
        {
            return isCastling() && getDst().column() == Kingside_Castled_King_Column;
        }
    };

    static_assert (sizeof (Move) == 3);
    static_assert (std::is_trivial_v<Move>);

    template <class IntegerType = int8_t>
    [[nodiscard]] constexpr auto
    castlingRowForColor (Color who)
        -> IntegerType
    {
        static_assert (std::is_integral_v<IntegerType>);
        return narrow_cast<IntegerType> (who == Color::White ? Last_Row : First_Row);
    }

    constexpr auto
    operator== (Move a, Move b) noexcept
        -> bool
    {
        return a.src == b.src
            && a.dst == b.dst
            && a.combined == b.combined;
    }

    constexpr auto
    operator!= (Move a, Move b) noexcept
        -> bool
    {
        return !operator== (a, b);
    }

    // Parse a move. Returns empty if the parse failed.
    [[nodiscard]] auto 
    moveParseOptional (const string& str, Color who) 
        -> optional<Move>;

    // The coordinate for the taken pawn.
    [[nodiscard]] auto 
    enPassantTakenPawnCoord (Coord src, Coord dst) 
        -> Coord;

    // Map source/dest coordinate to corresponding move (en passant, castling, etc)
    // This doesn't check whether the move is legal or not completely - just gets what the
    // user is intending.
    [[nodiscard]] auto 
    mapCoordinatesToMove ( 
            const Board& board, 
            Color who, 
            Coord src, 
            Coord dst, 
            optional<Piece> promoted_piece = {}) 
        -> optional<Move>;

    // Parse a move. Throws an exception if it could not parse the move.
    [[nodiscard]] auto 
    moveParse (const string& str, Color color = Color::None) 
        -> Move;

    // Convert the move to a string.
    [[nodiscard]] auto 
    asString (const Move& move) 
        -> string;

    // Send the move to the ostream.
    auto 
    operator<< (
        std::ostream& os, 
        const Move& value
    ) -> std::ostream&;
}
