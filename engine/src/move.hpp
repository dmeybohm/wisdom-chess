#ifndef WISDOM_CHESS_MOVE_HPP
#define WISDOM_CHESS_MOVE_HPP

#include "global.hpp"
#include "coord.hpp"
#include "piece.hpp"

namespace wisdom
{
    class Board;

    using CastlingState = uint8_t;

    static constexpr size_t Max_Packed_Capacity_In_Move = 0x0fffFFFFL; // 28 bit max

    constexpr uint8_t
            Castle_None = 0b000U;      // still eligible to castle on both sides
    constexpr uint8_t
            Castle_Kingside = 0b001U;   // ineligible for further castling kingside
    constexpr uint8_t
            Castle_Queenside = 0b010U;   // ineligible for further castling queenside
    constexpr uint8_t
            Castle_Both_Unavailable = 0b011U; // both ineligible
    constexpr uint8_t
            Castle_Previously_None = 0b111U;   // previously was none -
                                               // used for determining if a move affects castling

    enum class MoveCategory
    {
        Default = 0,
        NormalCapturing = 1,
        EnPassant = 2,
        Castling = 3,
    };

    struct UndoMove
    {
        int half_move_clock;
        array<int, Num_Players> position_score;
        Piece taken_piece_type;
        array<Coord, Num_Players> en_passant_targets;
        MoveCategory category;
        CastlingState current_castle_state;
        CastlingState opponent_castle_state;
        bool full_move_clock_updated;
    };

    static_assert(sizeof(UndoMove) <= 24);

    constexpr UndoMove Empty_Undo_State = {
        .half_move_clock = 0,
        .taken_piece_type = Piece::None,
        .en_passant_targets = { No_En_Passant_Coord, No_En_Passant_Coord },
        .category = MoveCategory::Default,
        .current_castle_state = Castle_None,
        .opponent_castle_state = Castle_None,
        .full_move_clock_updated = false,
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
        assert (source < 4);
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
                .move_category = gsl::narrow_cast<int8_t> ((size >> 21) & 0x7f),
            };
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
            return src |
                (dst << 7) |
                (promoted_piece << 14) |
                (move_category << 21);
        }
    };

    static_assert (sizeof (Move) == 4);
    static_assert (std::is_trivial_v<Move>);

    [[nodiscard]] constexpr auto captured_material (UndoMove undo_state, Color opponent)
        -> ColoredPiece
    {
        if (undo_state.category == MoveCategory::NormalCapturing)
        {
            return ColoredPiece::make (opponent, undo_state.taken_piece_type);
        }
        else if (undo_state.category == MoveCategory::EnPassant)
        {
            return ColoredPiece::make (opponent, Piece::Pawn);
        }
        else
        {
            return Piece_And_Color_None;
        }
    }

    template <class IntegerType = int8_t>
    [[nodiscard]] constexpr auto castling_row_for_color (Color who) -> IntegerType
    {
        static_assert (std::is_integral_v<IntegerType>);
        return gsl::narrow_cast<IntegerType> (
                who == Color::White ? Last_Row : First_Row
        );
    }

    using PlayerCastleState = array<CastlingState, Num_Players>;

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

    // Pack the castle state into the move.
    [[nodiscard]] constexpr auto unpack_castle_state (CastlingState state) noexcept
        -> CastlingState
    {
        return state == Castle_Previously_None ? Castle_None : state;
    }

    // Unpack the castle state from the move.
    [[nodiscard]] constexpr auto pack_castle_state (CastlingState state) noexcept
        -> CastlingState
    {
        return state == Castle_None ? Castle_Previously_None : state;
    }

    [[nodiscard]] constexpr auto current_castle_state (const UndoMove& move) noexcept
        -> CastlingState
    {
        return unpack_castle_state (move.current_castle_state);
    }

    [[nodiscard]] constexpr auto opponent_castle_state (const UndoMove& undo_state) noexcept
        -> CastlingState
    {
        return unpack_castle_state (undo_state.opponent_castle_state);
    }

    [[nodiscard]] constexpr auto is_en_passant_vulnerable (const UndoMove& undo_state, Color who) noexcept
        -> bool
    {
        return undo_state.en_passant_targets[color_index (who)] != No_En_Passant_Coord;
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
