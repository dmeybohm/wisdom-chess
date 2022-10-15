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
        NormalMovement = 0,
        NormalCapturing = 1,
        SpecialEnPassant = 2,
        SpecialCastling = 3,
    };

    struct UndoMove
    {
        int half_move_clock;
        int position_score[Num_Players];
        Piece taken_piece_type;
        Coord en_passant_targets[Num_Players];
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
        .category = MoveCategory::NormalMovement,
        .current_castle_state = Castle_None,
        .opponent_castle_state = Castle_None,
        .full_move_clock_updated = false,
    };

    struct Move
    {
        int8_t src;
        int8_t dst;
        int8_t promoted_piece;
        int8_t move_category_and_packed_flag;
    };
    static_assert(sizeof(Move) == 4);
    static_assert(std::is_trivial<Move>::value);

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

    [[nodiscard]] constexpr auto move_src (Move mv) -> Coord
    {
        return make_coord_from_index (mv.src);
    }

    [[nodiscard]] constexpr auto move_dst (Move mv) -> Coord
    {
        return make_coord_from_index (mv.dst);
    }

    [[nodiscard]] constexpr auto move_category_from_int (int source) -> MoveCategory
    {
        assert (source < 4);
        return static_cast<MoveCategory> (source);
    }

    [[nodiscard]] constexpr auto get_move_category (Move move) -> MoveCategory
    {
        return move_category_from_int (move.move_category_and_packed_flag);
    }

    [[nodiscard]] constexpr auto to_int (MoveCategory move_category) -> int
    {
        return static_cast<int> (move_category);
    }

    [[nodiscard]] constexpr auto to_int8 (MoveCategory move_category) -> int
    {
        return static_cast<int8_t> (move_category);
    }

    [[nodiscard]] constexpr auto is_promoting_move (Move move) -> bool
    {
        return move.promoted_piece != Piece_And_Color_None.piece_type_and_color;
    }

    [[nodiscard]] constexpr auto move_get_promoted_piece (Move move) -> ColoredPiece
    {
        auto piece = piece_from_int (move.promoted_piece & 0xf);
        auto color = (move.promoted_piece & 0x10) == 0x10 ? Color::Black : Color::White;
        return make_piece (color, piece);
    }

    [[nodiscard]] constexpr auto is_normal_capturing_move (Move move) -> bool
    {
        auto category = get_move_category (move);
        return category == MoveCategory::NormalCapturing;
    }

    [[nodiscard]] constexpr auto captured_material (UndoMove undo_state, Color opponent)
        -> ColoredPiece
    {
        if (undo_state.category == MoveCategory::NormalCapturing)
        {
            return make_piece (opponent, undo_state.taken_piece_type);
        }
        else if (undo_state.category == MoveCategory::SpecialEnPassant)
        {
            return make_piece (opponent, Piece::Pawn);
        }
        else
        {
            return Piece_And_Color_None;
        }
    }

    [[nodiscard]] constexpr auto is_special_en_passant_move (Move move) noexcept
        -> bool
    {
        auto category = get_move_category (move);
        return category == MoveCategory::SpecialEnPassant;
    }

    [[nodiscard]] constexpr auto is_any_capturing_move (Move move) noexcept
        -> bool
    {
        return is_normal_capturing_move (move) || is_special_en_passant_move (move);
    }

    [[nodiscard]] constexpr auto is_special_castling_move (Move move) noexcept
        -> bool
    {
        auto category = get_move_category (move);
        return category == MoveCategory::SpecialCastling;
    }

    [[nodiscard]] constexpr auto is_castling_move_on_king_side (Move move) noexcept
        -> bool
    {
        return is_special_castling_move (move) && Column (move_dst (move)) == 6;
    }

    [[nodiscard]] constexpr auto copy_move_with_promotion (Move move, ColoredPiece piece) noexcept
        -> Move
    {
        Move result = move;
        auto promoted_piece_type = to_int8 (piece_type (piece));
        auto promoted_color_index = color_index (piece_color (piece));
        result.promoted_piece = gsl::narrow_cast<int8_t> (
            (promoted_color_index << 4) | (promoted_piece_type & 0xf)
        );
        return result;
    }

    // run-of-the-mill move with no promotion or capturing involved
    [[nodiscard]] constexpr auto make_regular_move (Coord src, Coord dst) noexcept
        -> Move
    {
        return Move {
            .src = gsl::narrow_cast<int8_t> (coord_index (src)),
            .dst = gsl::narrow_cast<int8_t> (coord_index (dst)),
            .promoted_piece = 0,
            .move_category_and_packed_flag = 0,
        };
    }

    [[nodiscard]] constexpr auto make_regular_move (int src_row, int src_col,
                                                    int dst_row, int dst_col) noexcept
        -> Move
    {
        Coord src = make_coord (src_row, src_col);
        Coord dst = make_coord (dst_row, dst_col);

        return make_regular_move (src, dst);
    }

    [[nodiscard]] inline auto make_move_with_packed_capacity (size_t size) noexcept
        -> Move
    {
        assert (size <= Max_Packed_Capacity_In_Move);
        return {
            .src = gsl::narrow_cast<int8_t> (size & 0x7f),
            .dst = gsl::narrow_cast<int8_t> ((size >> 7) & 0x7f),
            .promoted_piece = gsl::narrow_cast<int8_t> ((size >> 14) & 0x7f),
            .move_category_and_packed_flag = gsl::narrow_cast<int8_t> ((size >> 21) & 0x7f),
        };
    }

    [[nodiscard]] inline auto unpack_capacity_from_move (Move move) noexcept
        -> size_t
    {
        return move.src |
            (move.dst << 7) |
            (move.promoted_piece << 14) |
            (move.move_category_and_packed_flag << 21);
    }

    [[nodiscard]] constexpr auto make_normal_capturing_move (int src_row, int src_col,
                                                             int dst_row, int dst_col) noexcept
        -> Move
    {
        Move move = make_regular_move (src_row, src_col, dst_row, dst_col);
        move.move_category_and_packed_flag = to_int8 (MoveCategory::NormalCapturing);
        return move;
    }

    [[nodiscard]] constexpr auto make_special_castling_move (int src_row, int src_col,
                                                             int dst_row, int dst_col) noexcept
        -> Move
    {
        Move move = make_regular_move (src_row, src_col, dst_row, dst_col);
        move.move_category_and_packed_flag = to_int8 (MoveCategory::SpecialCastling);
        return move;
    }

    template <class IntegerType = int8_t>
    [[nodiscard]] constexpr auto castling_row_for_color (Color who) -> IntegerType
    {
        static_assert (std::is_integral_v<IntegerType>);
        return gsl::narrow_cast<IntegerType> (
                who == Color::White ? Last_Row : First_Row
        );
    }

    [[nodiscard]] constexpr auto make_special_castling_move (Coord src, Coord dst) noexcept
        -> Move
    {
        return make_special_castling_move (Row (src), Column (src),
                                           Row (dst), Column (dst));
    }

    [[nodiscard]] constexpr auto copy_move_with_capture (Move move) noexcept
        -> Move
    {
        Coord src = move_src (move);
        Coord dst = move_dst (move);
        assert (move.move_category_and_packed_flag == to_int8 (MoveCategory::NormalMovement));

        Move result = make_regular_move (Row (src), Column (src),
                                         Row (dst), Column (dst));
        result.move_category_and_packed_flag = to_int8 (MoveCategory::NormalCapturing);
        return result;
    }

    using PlayerCastleState = array<CastlingState, Num_Players>;

    [[nodiscard]] constexpr auto make_special_en_passant_move (int src_row, int src_col,
                                                               int dst_row, int dst_col) noexcept
        -> Move
    {
        Move move = make_regular_move (src_row, src_col, dst_row, dst_col);
        move.move_category_and_packed_flag = to_int8 (MoveCategory::SpecialEnPassant);
        return move;
    }

    [[nodiscard]] constexpr auto make_special_en_passant_move (Coord src, Coord dst) noexcept
        -> Move
    {
        return make_special_en_passant_move (Row (src), Column (src),
                                             Row (dst), Column (dst));
    }

    [[nodiscard]] constexpr auto move_equals (Move a, Move b) noexcept
        -> bool
    {
        return a.src == b.src &&
               a.dst == b.dst &&
               a.promoted_piece == b.promoted_piece &&
               a.move_category_and_packed_flag == b.move_category_and_packed_flag;
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
