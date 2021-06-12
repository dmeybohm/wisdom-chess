#ifndef WISDOM_CHESS_MOVE_H
#define WISDOM_CHESS_MOVE_H

#include "global.hpp"
#include "coord.hpp"
#include "piece.hpp"

namespace wisdom
{
    class Board;

    using CastlingState = uint8_t;

    constexpr uint8_t
            Castle_None = 0U;      // still eligible to castle on both sides
    constexpr uint8_t
            Castle_Castled = 0x01U;   // castled
    constexpr uint8_t
            Castle_Kingside = 0x02U;   // ineligible for further castling kingside
    constexpr uint8_t
            Castle_Queenside = 0x04U;   // ineligible for further castling queenside
    constexpr uint8_t
            Castle_Previously_None = 0x07U;   // previously was none - used for determining if a move affects castling

    enum class MoveCategory
    {
        NonCapture = 0,
        NormalCapture = 1,
        EnPassant = 2,
        Castling = 3,
    };

    struct undo_move
    {
        MoveCategory category;
        Piece taken_piece_type;

        CastlingState current_castle_state;
        CastlingState opponent_castle_state;
        int half_move_clock;
        bool full_move_clock_updated;
        Coord en_passant_target[Num_Players];
    };

    using UndoMove = struct undo_move;

    constexpr UndoMove empty_undo_state = {
            .category = MoveCategory::NonCapture,
            .taken_piece_type = Piece::None,
            .current_castle_state = Castle_None,
            .opponent_castle_state = Castle_None,
            .half_move_clock = 0,
            .full_move_clock_updated = false,
            .en_passant_target = { No_En_Passant_Coord, No_En_Passant_Coord },
    };

    struct move
    {
        int8_t src_row: 4;
        int8_t src_col: 4;

        int8_t dst_row: 4;
        int8_t dst_col: 4;

        Color promoted_color: 4;
        Piece promoted_piece_type: 4;

        MoveCategory move_category: 4;
    };

    using Move = struct move;

    static_assert(std::is_trivial<Move>::value);

    class ParseMoveException : public Error
    {
    public:
        explicit ParseMoveException (std::string message) : Error (std::move (message))
        {}
    };

    class MoveConsistencyProblem : public Error
    {
    public:
        MoveConsistencyProblem () : Error ("Move consistency error.")
        {}

        MoveConsistencyProblem (std::string extra_info) :
            Error ("Move consistency error.", std::move (extra_info) )
        {}
    };

////////////////////////////////////////////////////////////////////

    constexpr Coord move_src (Move mv)
    {
        return make_coord (mv.src_row, mv.src_col);
    }

    constexpr Coord move_dst (Move mv)
    {
        return make_coord (mv.dst_row, mv.dst_col);
    }

    constexpr int is_promoting_move (Move move)
    {
        return move.promoted_piece_type != Piece::None;
    }

    constexpr ColoredPiece move_get_promoted_piece (Move move)
    {
        return make_piece (move.promoted_color, move.promoted_piece_type);
    }

    constexpr int is_capture_move (Move move)
    {
        return move.move_category == MoveCategory::NormalCapture;
    }

    constexpr ColoredPiece captured_material (UndoMove undo_state, Color opponent)
    {
        if (undo_state.category == MoveCategory::NormalCapture)
        {
            return make_piece (opponent, undo_state.taken_piece_type);
        }
        else if (undo_state.category == MoveCategory::EnPassant)
        {
            return make_piece (opponent, Piece::Pawn);
        }
        else
        {
            return Piece_And_Color_None;
        }
    }

    constexpr bool is_en_passant_move (Move move)
    {
        return move.move_category == MoveCategory::EnPassant;
    }

    constexpr bool move_affects_current_castle_state (UndoMove move)
    {
        return move.current_castle_state != Castle_None;
    }

    constexpr bool move_affects_opponent_castle_state (UndoMove move)
    {
        return move.opponent_castle_state != Castle_None;
    }

    constexpr bool is_castling_move (Move move)
    {
        return move.move_category == MoveCategory::Castling;
    }

    constexpr bool is_double_square_pawn_move (ColoredPiece src_piece, Move move)
    {
        Coord src = move_src (move);
        Coord dst = move_dst (move);
        return piece_type (src_piece) == Piece::Pawn && abs (ROW (src) - ROW (dst)) == 2;
    }

    constexpr bool is_castling_move_on_king_side (Move move)
    {
        return is_castling_move (move) && move.dst_col == 6;
    }

    static inline int castling_row_from_color (Color who)
    {
        switch (who)
        {
            case Color::White: return 7;
            case Color::Black: return 0;
            default: throw Error {
                "Invalid color in castling_row_from_color()"
            };
        }
    }

    constexpr Move copy_move_with_promotion (Move move, ColoredPiece piece)
    {
        Move result = move;
        result.promoted_piece_type = piece_type (piece);
        result.promoted_color = piece_color (piece);
        return result;
    }

    // run-of-the-mill move with no promotion involved
    constexpr Move make_move (int src_row, int src_col,
                              int dst_row, int dst_col)
    {
        assert (src_row >= 0 && src_row < Num_Rows);
        assert (dst_row >= 0 && src_row < Num_Rows);
        assert (src_col >= 0 && src_row < Num_Columns);
        assert (dst_col >= 0 && src_row < Num_Columns);
        Move result = {
                .src_row = static_cast<int8_t>(src_row),
                .src_col = static_cast<int8_t>(src_col),
                .dst_row = static_cast<int8_t>(dst_row),
                .dst_col = static_cast<int8_t>(dst_col),
                .promoted_color = Color::None,
                .promoted_piece_type = Piece::None,
                .move_category = MoveCategory::NonCapture,
        };

        return result;
    }

    constexpr Move make_move (Coord src, Coord dst)
    {
        return make_move (ROW (src), COLUMN (src), ROW (dst), COLUMN (dst));
    }

    constexpr Move make_capturing_move (int src_row, int src_col,
                                        int dst_row, int dst_col)
    {
        Move move = make_move (src_row, src_col, dst_row, dst_col);
        move.move_category = MoveCategory::NormalCapture;
        return move;
    }

    constexpr Move make_castling_move (int src_row, int src_col,
                                       int dst_row, int dst_col)
    {
        Move move = make_move (src_row, src_col, dst_row, dst_col);
        move.move_category = MoveCategory::Castling;
        return move;
    }

    static inline Move copy_move_with_capture (Move move)
    {
        Coord src = move_src (move);
        Coord dst = move_dst (move);
        assert (move.move_category == MoveCategory::NonCapture);
        Move result = make_move (ROW (src), COLUMN (src), ROW (dst), COLUMN (dst));
        result.move_category = MoveCategory::NormalCapture;
        return result;
    }

    constexpr Move make_en_passant_move (int src_row, int src_col,
                                         int dst_row, int dst_col)
    {
        Move move = make_move (src_row, src_col, dst_row, dst_col);
        move.move_category = MoveCategory::EnPassant;
        return move;
    }

    constexpr bool move_equals (Move a, Move b)
    {
        return a.src_row == b.src_row &&
               a.dst_row == b.dst_row &&
               a.src_col == b.src_col &&
               a.dst_col == b.dst_col &&
               a.move_category == b.move_category &&
               a.promoted_color == b.promoted_color &&
               a.promoted_piece_type == b.promoted_piece_type;
    }

    constexpr bool operator== (Move a, Move b)
    {
        return move_equals (a, b);
    }

    constexpr bool operator!= (Move a, Move b)
    {
        return !move_equals (a, b);
    }

    // Pack the castle state into the move.
    constexpr CastlingState unpack_castle_state (CastlingState state)
    {
        return state == Castle_Previously_None ? Castle_None : state;
    }

    // Unpack the castle state from the move.
    constexpr CastlingState pack_castle_state (CastlingState state)
    {
        return state == Castle_None ? Castle_Previously_None : state;
    }

    constexpr CastlingState current_castle_state (UndoMove move)
    {
        return unpack_castle_state (move.current_castle_state);
    }

    constexpr CastlingState opponent_castle_state (UndoMove undo_state)
    {
        return unpack_castle_state (undo_state.opponent_castle_state);
    }

    static inline void save_current_castle_state (UndoMove &undo_state, CastlingState state)
    {
        undo_state.current_castle_state = pack_castle_state (state);
    }

    static inline void save_opponent_castle_state (UndoMove &undo_state, CastlingState state)
    {
        undo_state.opponent_castle_state = pack_castle_state (state);
    }

    constexpr bool is_en_passant_vulnerable (UndoMove undo_state, Color who)
    {
        return undo_state.en_passant_target[color_index (who)] != No_En_Passant_Coord;
    }

    /////////////////////////////////////////////////////////////////////

    UndoMove do_move (Board &board, Color who, Move move);

    void undo_move (Board &board, Color who, Move move,
                    UndoMove undo_state);

    // Parse a move. Returns empty if the parse failed.
    std::optional<Move> move_parse_optional (const std::string &str, Color who);

    Coord en_passant_taken_pawn_coord (Coord src, Coord dst);

    // Parse a move. Throws an exception if could not parse the move.
    Move move_parse (const std::string &str, Color color = Color::None);

    /////////////////////////////////////////////////////////////////////

    Move parse_move (const std::string &str, Color color = Color::None);

    std::string to_string (const Move &move);

    std::ostream &operator<< (std::ostream &os, const Move &value);

}

#endif // WISDOM_CHESS_MOVE_H