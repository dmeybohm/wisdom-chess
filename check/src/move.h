#ifndef EVOLVE_CHESS_MOVE_H
#define EVOLVE_CHESS_MOVE_H

#include <cassert>
#include <string>

#include "global.h"
#include "coord.h"
#include "piece.h"

struct board;

typedef int8_t castle_state_t;

enum castle
{
	CASTLE_NONE            = 0U,      // still eligible to castle on both sides
	CASTLE_CASTLED         = 0x01U,   // castled
	CASTLE_KINGSIDE        = 0x02U,   // ineligible for further castling kingside
	CASTLE_QUEENSIDE       = 0x04U,   // ineligible for further castling queenside
	CASTLE_PREVIOUSLY_NONE = 0x07U,   // previously was none - used for determining if a move affects castling
};

constexpr int MAX_CASTLE_STATES = 8;

enum class MoveCategory
{
    NonCapture = 0,
    NormalCapture = 1,
    EnPassant = 2,
    Castling = 3,
};

typedef struct undo_move
{
    MoveCategory                category;
    Piece                       taken_piece_type;
    
    castle_state_t              current_castle_state;
    castle_state_t              opponent_castle_state;
    coord_t                     en_passant_target[Num_Players];
} undo_move_t;

constexpr undo_move_t empty_undo_state =
{
    .category = MoveCategory::NonCapture,
    .taken_piece_type = Piece::None,
    .current_castle_state = CASTLE_NONE,
    .opponent_castle_state = CASTLE_NONE,
    .en_passant_target = { no_en_passant_coord, no_en_passant_coord },
};

typedef struct move
{
	int8_t             src_row : 4;
	int8_t             src_col : 4;

	int8_t             dst_row : 4;
	int8_t             dst_col : 4;

	Color              promoted_color: 4;
	Piece              promoted_piece_type: 4;

	MoveCategory       move_category : 4;
} move_t;

class parse_move_exception : public std::exception
{
    const char *message;

public:
    explicit parse_move_exception (const char *message) : message { message } {}
    [[nodiscard]] const char *what() const noexcept override { return this->message; }
};

////////////////////////////////////////////////////////////////////

constexpr coord_t move_src (move_t mv)
{
	return make_coord (mv.src_row, mv.src_col);
}

constexpr coord_t move_dst (move_t mv)
{
	return make_coord (mv.dst_row, mv.dst_col);
}

constexpr int is_promoting_move (move_t move)
{
	return move.promoted_piece_type != Piece::None;
}

constexpr piece_t move_get_promoted_piece (move_t move)
{
	return make_piece (move.promoted_color, move.promoted_piece_type);
}

constexpr int is_capture_move (move_t move)
{
	return move.move_category == MoveCategory::NormalCapture;
}

constexpr piece_t captured_material (undo_move_t undo_state, Color opponent)
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
        return piece_and_color_none;
    }
}

constexpr bool is_en_passant_move (move_t move)
{
	return move.move_category == MoveCategory::EnPassant;
}

constexpr bool move_affects_current_castle_state (undo_move_t move)
{
    return move.current_castle_state != CASTLE_NONE;
}

constexpr bool move_affects_opponent_castle_state (undo_move_t move)
{
    return move.opponent_castle_state != CASTLE_NONE;
}

constexpr bool is_castling_move (move_t move)
{
	return move.move_category == MoveCategory::Castling;
}

constexpr bool is_double_square_pawn_move (piece_t src_piece, move_t move)
{
    coord_t src = move_src (move);
    coord_t dst = move_dst (move);
    return piece_type (src_piece) == Piece::Pawn && abs(ROW(src) - ROW(dst)) == 2;
}

constexpr bool is_castling_move_on_king_side (move_t move)
{
	return is_castling_move(move) && move.dst_col == 6;
}

static inline int8_t castling_row_from_color (Color who)
{
    switch (who)
    {
        case Color::White:
            return 7;
        case Color::Black:
            return 0;
        default:
            assert (0);
    }
}

constexpr move_t copy_move_with_promotion (move_t move, piece_t piece)
{
    move_t result = move;
    result.promoted_piece_type = piece_type (piece);
    result.promoted_color = piece_color (piece);
	return result;
}

// run-of-the-mill move with no promotion involved
constexpr move_t make_move (int8_t src_row, int8_t src_col,
                            int8_t dst_row, int8_t dst_col)
{
	move_t result = {
	        .src_row = src_row,
	        .src_col = src_col,
	        .dst_row = dst_row,
	        .dst_col = dst_col,
	        .promoted_color = Color::None,
	        .promoted_piece_type = Piece::None,
	        .move_category = MoveCategory::NonCapture,
	};

	return result;
}

constexpr move_t make_move (coord_t src, coord_t dst)
{
    return make_move (ROW (src), COLUMN (src), ROW (dst), COLUMN (dst));
}

constexpr move_t make_capturing_move (int8_t src_row, int8_t src_col,
                                      int8_t dst_row, int8_t dst_col)
{
    move_t move = make_move (src_row, src_col, dst_row, dst_col);
    move.move_category = MoveCategory::NormalCapture;
    return move;
}

constexpr move_t make_castling_move (int8_t src_row, int8_t src_col,
                                     int8_t dst_row, int8_t dst_col)
{
    move_t move = make_move (src_row, src_col, dst_row, dst_col);
    move.move_category = MoveCategory::Castling;
    return move;
}

static inline move_t copy_move_with_capture (move_t move)
{
    coord_t src = move_src (move);
    coord_t dst = move_dst (move);
    assert (move.move_category == MoveCategory::NonCapture);
    move_t result = make_move (ROW (src), COLUMN (src), ROW (dst), COLUMN (dst));
    result.move_category = MoveCategory::NormalCapture;
    return result;
}

constexpr move_t make_en_passant_move (int8_t src_row, int8_t src_col,
                                       int8_t dst_row, int8_t dst_col)
{
    move_t move = make_move (src_row, src_col, dst_row, dst_col);
    move.move_category = MoveCategory::EnPassant;
    return move;
}

constexpr bool is_null_move (move_t move)
{
	// no move has the same position for src and dst
	return move.src_row == 0 && move.src_col == 0 &&
	    move.dst_row == 0 && move.dst_col == 0;
}

constexpr move_t null_move = make_move (0, 0, 0, 0);

constexpr bool move_equals (move_t a, move_t b)
{
	return a.src_row == b.src_row &&
	    a.dst_row == b.dst_row &&
	    a.src_col == b.src_col &&
	    a.dst_col == b.dst_col &&
	    a.move_category == b.move_category &&
	    a.promoted_color == b.promoted_color &&
	    a.promoted_piece_type == b.promoted_piece_type;
}

constexpr bool operator == (move_t a, move_t b)
{
    return move_equals (a, b);
}

constexpr bool operator != (move_t a, move_t b)
{
    return !move_equals (a, b);
}

// Pack the castle state into the move.
constexpr castle_state_t unpack_castle_state (castle_state_t state)
{
    return state == CASTLE_PREVIOUSLY_NONE ? CASTLE_NONE : state;
}

// Unpack the castle state from the move.
constexpr castle_state_t pack_castle_state (castle_state_t state)
{
    return state == CASTLE_NONE ? CASTLE_PREVIOUSLY_NONE : state;
}

constexpr castle_state_t current_castle_state (undo_move_t move)
{
    return unpack_castle_state(move.current_castle_state);
}

constexpr castle_state_t opponent_castle_state (undo_move_t undo_state)
{
    return unpack_castle_state(undo_state.opponent_castle_state);
}

static inline void save_current_castle_state (undo_move_t *undo_state, castle_state_t state)
{
    undo_state->current_castle_state = pack_castle_state(state);
}

static inline void save_opponent_castle_state (undo_move_t *undo_state, castle_state_t state)
{
    undo_state->opponent_castle_state = pack_castle_state(state);
}

constexpr bool is_en_passant_vulnerable (undo_move_t undo_state, Color who)
{
    return undo_state.en_passant_target[color_index(who)] != no_en_passant_coord;
}

/////////////////////////////////////////////////////////////////////

undo_move_t   do_move         (struct board &board, Color who, move_t move);
void          undo_move       (struct board &board, Color who, move_t move,
                               undo_move_t undo_state);
move_t        move_parse      (const std::string &str, Color who);

coord_t en_passant_taken_pawn_coord (coord_t src, coord_t dst);

// Parse a move
move_t parse_move (const std::string &str, Color color = Color::None);

/////////////////////////////////////////////////////////////////////

std::string to_string (const move_t &move);

/////////////////////////////////////////////////////////////////////



#endif // EVOLVE_CHESS_MOVE_H
