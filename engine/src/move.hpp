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
        MoveConsistencyProblem () : Error ("Move consistency error.")
        {}

        explicit MoveConsistencyProblem (string extra_info) :
            Error ("Move consistency error.", std::move (extra_info) )
        {}
    };

    [[nodiscard]] constexpr auto move_category_from_int (int source) -> MoveCategory
    {
        assert (source <= 4);
        return static_cast<MoveCategory> (source);
    }

    [[nodiscard]] constexpr auto to_int (MoveCategory move_category) -> int
    {
        return static_cast<int> (move_category);
    }

    [[nodiscard]] constexpr auto to_int8 (MoveCategory move_category) -> int
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
                .src = gsl::narrow_cast<int8_t> (coord_index (src)),
                .dst = gsl::narrow_cast<int8_t> (coord_index (dst)),
                .promoted_piece = gsl::narrow_cast<int8_t> (0),
                .move_category = gsl::narrow_cast<int8_t> (0),
            };
        }

        [[nodiscard]] static constexpr auto make (int src_row, int src_col,
                                                  int dst_row, int dst_col) noexcept
            -> Move
        {
            Coord src = make_coord (src_row, src_col);
            Coord dst = make_coord (dst_row, dst_col);

            return make (src, dst);
        }

        [[nodiscard]] static constexpr auto make_normal_capturing (int src_row, int src_col,
                                                                   int dst_row, int dst_col) noexcept
            -> Move
        {
            Move move = Move::make (src_row, src_col, dst_row, dst_col);
            move.move_category = to_int8 (MoveCategory::NormalCapturing);
            return move;
        }

        [[nodiscard]] static constexpr auto make_castling (int src_row, int src_col,
                                                           int dst_row, int dst_col) noexcept
            -> Move
        {
            Move move = Move::make (src_row, src_col, dst_row, dst_col);
            move.move_category = to_int8 (MoveCategory::Castling);
            return move;
        }

        [[nodiscard]] static constexpr auto make_castling (Coord src, Coord dst) noexcept
            -> Move
        {
            return Move::make_castling (Row (src), Column (src), Row (dst), Column (dst));
        }

        [[nodiscard]] static constexpr auto make_en_passant (int src_row, int src_col,
                                                             int dst_row, int dst_col) noexcept
            -> Move
        {
            Move move = make (src_row, src_col, dst_row, dst_col);
            move.move_category = to_int8 (MoveCategory::EnPassant);
            return move;
        }

        [[nodiscard]] static constexpr auto make_en_passant (Coord src, Coord dst) noexcept
            -> Move
        {
            return Move::make_en_passant (Row (src), Column (src), Row (dst), Column (dst));
        }

        // Pack a 28-bit integer into the move. Used for move lists to store the list length.
        [[nodiscard]] static constexpr auto make_as_packed_capacity (size_t size) noexcept
            -> Move
        {
            assert (size <= Max_Packed_Capacity_In_Move);
            return {
                .src = gsl::narrow_cast<int8_t> (size & 0x7f),
                .dst = gsl::narrow_cast<int8_t> ((size >> 7) & 0x7f),
                .promoted_piece = gsl::narrow_cast<int8_t> ((size >> 14) & 0x7f),
                .move_category = to_int8 (MoveCategory::PackedCapacity),
            };
        }

        [[nodiscard]] static constexpr auto from_int (int packed_move)
            -> Move
        {
            return Move {
                .src = gsl::narrow<int8_t> (packed_move & 0x7f),
                .dst = gsl::narrow<int8_t> ((packed_move >> 8) & 0x7f),
                .promoted_piece = gsl::narrow<int8_t> ((packed_move >> 16) & 0x7f),
                .move_category = gsl::narrow<int8_t> ((packed_move >> 24) & 0x7f),
            };
        }

        [[nodiscard]] auto to_int () -> int
        {
            return src | (dst << 8) | (promoted_piece << 16) | (move_category << 24);
        }

        [[nodiscard]] constexpr auto get_src () const -> Coord
        {
            return make_coord_from_index (src);
        }

        [[nodiscard]] constexpr auto get_dst () const -> Coord
        {
            return make_coord_from_index (dst);
        }

        [[nodiscard]] constexpr auto with_promotion (ColoredPiece piece) const noexcept
            -> Move
        {
            assert (piece != Piece_And_Color_None);
            Move result = *this;
            auto promoted_piece_type = to_int8 (piece_type (piece));
            auto promoted_color_index = color_index (piece_color (piece));
            result.promoted_piece = gsl::narrow_cast<int8_t> (
                (promoted_color_index << 4) | (promoted_piece_type & 0xf)
            );
            return result;
        }

        [[nodiscard]] constexpr auto with_capture () const noexcept
            -> Move
        {
            assert (move_category == to_int8 (MoveCategory::Default));

            Move result = Move::make (get_src (), get_dst ());
            result.move_category = to_int8 (MoveCategory::NormalCapturing);
            return result;
        }

        [[nodiscard]] constexpr auto get_move_category () const -> MoveCategory
        {
            return move_category_from_int (move_category);
        }

        [[nodiscard]] constexpr auto is_normal_capturing () const -> bool
        {
            return get_move_category () == MoveCategory::NormalCapturing;
        }

        [[nodiscard]] constexpr auto is_promoting () const -> bool
        {
            return promoted_piece != to_int8 (Piece_And_Color_None);
        }

        [[nodiscard]] constexpr auto get_promoted_piece () const -> ColoredPiece
        {
            auto piece = piece_from_int (promoted_piece & 0xf);
            auto color = piece == Piece::None ? Color::None :
                (promoted_piece & 0x10) == 0x10 ? Color::Black : Color::White;
            return ColoredPiece::make (color, piece);
        }

        [[nodiscard]] constexpr auto is_en_passant () const noexcept
            -> bool
        {
            return get_move_category () == MoveCategory::EnPassant;
        }

        [[nodiscard]] constexpr auto is_any_capturing () const noexcept
            -> bool
        {
            return is_normal_capturing () || is_en_passant ();
        }

        [[nodiscard]] constexpr auto is_castling () const noexcept
            -> bool
        {
            return get_move_category () == MoveCategory::Castling;
        }

        [[nodiscard]] constexpr auto is_castling_on_kingside () const noexcept
            -> bool
        {
            return is_castling () && Column (get_dst ()) == Kingside_Castled_King_Column;
        }

        [[nodiscard]] constexpr auto to_unpacked_capacity () const noexcept
            -> size_t
        {
            assert (get_move_category () == MoveCategory::PackedCapacity);
            return src |
                (dst << 7) |
                (promoted_piece << 14);
        }
    };

    static_assert (sizeof (Move) == 4);
    static_assert (std::is_trivial_v<Move>);

    template <class IntegerType = int8_t>
    [[nodiscard]] constexpr auto castling_row_for_color (Color who) -> IntegerType
    {
        static_assert (std::is_integral_v<IntegerType>);
        return gsl::narrow_cast<IntegerType> (
                who == Color::White ? Last_Row : First_Row
        );
    }

    using PlayerCastleState = array<CastlingEligibility, Num_Players>;

    [[nodiscard]] constexpr auto move_equals (Move a, Move b) noexcept
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
        return move_equals (a, b);
    }

    constexpr auto operator!= (Move a, Move b) noexcept
        -> bool
    {
        return !move_equals (a, b);
    }

    // Parse a move. Returns empty if the parse failed.
    [[nodiscard]] auto move_parse_optional (const string& str, Color who) -> optional<Move>;

    // The coordinate for the taken pawn.
    [[nodiscard]] auto en_passant_taken_pawn_coord (Coord src, Coord dst) -> Coord;

    // Map source/dest coordinate to corresponding move (en passant, castling, etc)
    // This doesn't check whether the move is legal or not completely - just gets what the
    // user is intending.
    [[nodiscard]] auto map_coordinates_to_move (const Board& board, Color who,
                                  Coord src, Coord dst,
                                  optional<Piece> promoted_piece = {})
        -> optional<Move>;

    // Parse a move. Throws an exception if it could not parse the move.
    [[nodiscard]] auto move_parse (const string& str, Color color = Color::None) -> Move;

    // Convert the move to a string.
    [[nodiscard]] auto to_string (const Move& move) -> string;

    // Send the move to the ostream.
    auto operator<< (std::ostream& os, const Move& value) -> std::ostream&;
}

#endif // WISDOM_CHESS_MOVE_HPP
