#ifndef EVOLVE_CHESS_MOVE_H
#define EVOLVE_CHESS_MOVE_H

#include <string.h>
#include <assert.h>

#include "global.h"
#include "coord.h"
#include "piece.h"

struct board;

typedef uint8_t castle_state_t;

enum castle
{
	CASTLE_NONE            = 0U,      // still eligible to castle on both sides
	CASTLE_CASTLED         = 0x01U,   // castled
	CASTLE_KINGSIDE        = 0x02U,   // ineligible for further castling kingside
	CASTLE_QUEENSIDE       = 0x04U,   // ineligible for further castling queenside
	CASTLE_PREVIOUSLY_NONE = 0x07U,   // previously was none - used for determining if a move affects castling
};

constexpr int MAX_CASTLE_STATES = 8;

enum move_category
{
    MOVE_CATEGORY_NON_CAPTURE = 0,
    MOVE_CATEGORY_NORMAL_CAPTURE = 1,
    MOVE_CATEGORY_EN_PASSANT = 2,
    MOVE_CATEGORY_CASTLING = 3,
};

typedef struct undo_move
{
    enum move_category category;
    enum piece_type    taken_piece_type;
    
    castle_state_t     current_castle_state;
    castle_state_t     opponent_castle_state;
} undo_move_t;

constexpr undo_move_t empty_undo_state = {
    .category = MOVE_CATEGORY_NON_CAPTURE,
    .taken_piece_type = PIECE_NONE,
    .current_castle_state = CASTLE_NONE,
    .opponent_castle_state = CASTLE_NONE,
};

typedef struct move
{
	uint8_t            src_row : 3;
	uint8_t            src_col : 3;

	uint8_t            dst_row : 3;
	uint8_t            dst_col : 3;

	enum color         promoted_color: 2;
	enum piece_type    promoted_piece_type: 3;

	enum move_category move_category : 3;
} move_t;

constexpr coord_t MOVE_SRC (move_t mv)
{
	return coord_create (mv.src_row, mv.src_col);
}

constexpr coord_t MOVE_DST (move_t mv)
{
	return coord_create (mv.dst_row, mv.dst_col);
}

constexpr int is_promoting_move (move_t move)
{
	return move.promoted_piece_type != PIECE_NONE;
}

constexpr piece_t move_get_promoted_piece (move_t move)
{
	return MAKE_PIECE (move.promoted_color, move.promoted_piece_type);
}

constexpr int is_capture_move (move_t move)
{
	return move.move_category == MOVE_CATEGORY_NORMAL_CAPTURE;
}

constexpr piece_t captured_material (undo_move_t undo_state, enum color opponent)
{
    if (undo_state.category == MOVE_CATEGORY_NORMAL_CAPTURE)
    {
        return MAKE_PIECE( opponent, undo_state.taken_piece_type );
    }
    else if (undo_state.category == MOVE_CATEGORY_EN_PASSANT)
    {
        return MAKE_PIECE( opponent, PIECE_PAWN);
    }
    else
    {
        return PIECE_AND_COLOR_NONE;
    }
}

constexpr bool is_en_passant_move (move_t move)
{
	return move.move_category == MOVE_CATEGORY_EN_PASSANT;
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
	return move.move_category == MOVE_CATEGORY_CASTLING;
}

constexpr bool is_castling_move_on_king_side (move_t move)
{
	return is_castling_move(move) && move.dst_col == 6;
}

static inline uint8_t castling_row_from_color (enum color who)
{
    switch (who)
    {
        case COLOR_WHITE:
            return 7;
        case COLOR_BLACK:
            return 0;
        default:
            assert (0);
    }
}

constexpr move_t move_with_promotion (move_t move, piece_t piece)
{
    move_t result = move;
    result.promoted_piece_type = PIECE_TYPE(piece);
    result.promoted_color = PIECE_COLOR(piece);
	return result;
}

// run-of-the-mill move with no promotion involved
constexpr move_t move_create (uint8_t src_row, uint8_t src_col,
                                  uint8_t dst_row, uint8_t dst_col)
{
	move_t result = {
	        .src_row = src_row,
	        .src_col = src_col,
	        .dst_row = dst_row,
	        .dst_col = dst_col,
	        .promoted_color = COLOR_NONE,
	        .promoted_piece_type = PIECE_NONE,
	        .move_category = MOVE_CATEGORY_NON_CAPTURE,
	};

	return result;
}

constexpr move_t move_create_capturing (uint8_t src_row, uint8_t src_col,
                                            uint8_t dst_row, uint8_t dst_col)
{
    move_t move = move_create (src_row, src_col, dst_row, dst_col);
    move.move_category = MOVE_CATEGORY_NORMAL_CAPTURE;
    return move;
}

constexpr move_t move_create_castling (uint8_t src_row, uint8_t src_col,
                                           uint8_t dst_row, uint8_t dst_col)
{
    move_t move = move_create (src_row, src_col, dst_row, dst_col);
    move.move_category = MOVE_CATEGORY_CASTLING;
    return move;
}

static inline move_t move_with_capture (move_t move)
{
    coord_t src = MOVE_SRC(move);
    coord_t dst = MOVE_DST(move);
    assert (move.move_category == MOVE_CATEGORY_NON_CAPTURE);
    move_t result = move_create (ROW(src), COLUMN(src), ROW(dst), COLUMN(dst));
    result.move_category = MOVE_CATEGORY_NORMAL_CAPTURE;
    return result;
}

constexpr move_t move_create_en_passant (int8_t src_row, uint8_t src_col,
                                             uint8_t dst_row, uint8_t dst_col)
{
    move_t move = move_create( src_row, src_col, dst_row, dst_col);
    move.move_category = MOVE_CATEGORY_EN_PASSANT;
    return move;
}

constexpr bool is_null_move (move_t move)
{
	// no move has the same position for src and dst
	return move.src_row == 0 && move.src_col == 0 &&
	    move.dst_row == 0 && move.dst_col == 0;
}

constexpr move_t move_null ()
{
    return move_create (0, 0, 0, 0);
}

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

/////////////////////////////////////////////////////////////////////

undo_move_t   do_move         (struct board *board, enum color who, move_t move);
void          undo_move       (struct board *board, enum color who, move_t move,
                               undo_move_t undo_state);
move_t        move_parse      (const char *str, enum color who);
const char   *move_str        (move_t move);

coord_t en_passant_taken_pawn_coord (coord_t src, coord_t dst);

/////////////////////////////////////////////////////////////////////

#endif // EVOLVE_CHESS_MOVE_H
