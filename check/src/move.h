#ifndef EVOLVE_CHESS_MOVE_H
#define EVOLVE_CHESS_MOVE_H

#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "global.h"
#include "coord.h"
#include "piece.h"

typedef uint8_t castle_state_t;

enum castle
{
	CASTLE_NONE            = 0U,      // still eligible to castle on both sides
	CASTLE_CASTLED         = 0x01U,   // castled - use by move_t to indicate the move is castling
	CASTLE_KINGSIDE        = 0x02U,   // ineligible for further castling kingside
	CASTLE_QUEENSIDE       = 0x04U,   // ineligible for further castling queenside
	CASTLE_PREVIOUSLY_NONE = 0x07U,   // previously was none - used for determining if a move affects castling
};

#define MAX_CASTLE_STATES 8

typedef struct move
{
	uint8_t    src_row : 3;
	uint8_t    src_col : 3;

	uint8_t    dst_row : 3;
	uint8_t    dst_col : 3;

	uint8_t    taken_color: 2;
	uint8_t    taken_piece_type: 3;

	uint8_t    promoted_color: 2;
	uint8_t    promoted_piece_type: 3;

	uint8_t    is_en_passant : 1;
	uint8_t    is_castling : 1;

	uint8_t    current_castle_state : 3;
	uint8_t    opponent_castle_state : 3;
} move_t;

static inline coord_t MOVE_SRC (move_t mv)
{
	return coord_create (mv.src_row, mv.src_col);
}

static inline coord_t MOVE_DST (move_t mv)
{
	return coord_create (mv.dst_row, mv.dst_col);
}

static inline int is_promoting_move (const move_t *move)
{
	return move->promoted_piece_type != PIECE_NONE;
}

static inline piece_t move_get_promoted (const move_t *move)
{
    enum color color = (enum color)move->promoted_color;
    enum piece_type piece_type = (enum piece_type)move->promoted_piece_type;
	return MAKE_PIECE (color, piece_type);
}

static inline piece_t move_get_taken (const move_t *move)
{
    enum color color = (enum color)move->taken_color;
    enum piece_type piece_type = (enum piece_type)move->taken_piece_type;
	return MAKE_PIECE (color, piece_type);
}

static inline void move_set_taken (move_t *move, piece_t taken)
{
	move->taken_piece_type = PIECE_TYPE(taken);
	move->taken_color = PIECE_COLOR(taken);
}

static inline int is_capture_move (const move_t *move)
{
	return move->taken_piece_type != PIECE_NONE;
}

static inline void move_set_en_passant (move_t *move)
{
    move->is_en_passant = 1;
}

static inline int is_en_passant_move (const move_t *move)
{
	return move->is_en_passant;
}

static inline void move_set_castling (move_t *move)
{
    move->is_castling = 1;
}

static inline int move_affects_current_castle_state (move_t move)
{
    return move.current_castle_state != CASTLE_NONE;
}

static inline int move_affects_opponent_castle_state (move_t move)
{
    return move.opponent_castle_state != CASTLE_NONE;
}

static inline int is_castling_move (const move_t *move)
{
	return move->is_castling;
}

static inline int is_castling_move_on_king_side (const move_t *move)
{
	return is_castling_move(move) && move->dst_col == 6;
}

static inline unsigned char char_to_row (char chr)
{
    return 8 - (tolower (chr) - '0');
}

static inline unsigned char char_to_col (char chr)
{
    return tolower (chr) - 'a';
}

static inline move_t move_promote (move_t move, piece_t piece)
{
    move_t result = move;
    result.promoted_piece_type = PIECE_TYPE(piece);
    result.promoted_color = PIECE_COLOR(piece);
	return result;
}

// run-of-the-mill move with no promotion involved
static inline move_t move_create (uint8_t src_row, uint8_t src_col,
                                  uint8_t dst_row, uint8_t dst_col)
{
	move_t tmp = { 0 };

	tmp.src_row = src_row;
	tmp.src_col = src_col;
	tmp.dst_row = dst_row;
	tmp.dst_col = dst_col;

	return tmp;
}

static inline int is_null_move (move_t move)
{
	// no move has the same position for src and dst
	return move.src_row == 0 && move.src_col == 0 && move.dst_row == 0 && move.dst_col == 0;
}

static inline void move_nullify (move_t *move)
{
	*move = move_create (0, 0, 0, 0);
}

static inline int move_equal (const move_t *a, const move_t *b)
{
	// Hm, don't compare the taken piece
	return a->src_row == b->src_row &&
	    a->dst_row == b->dst_row &&
	    a->src_col == b->src_col &&
	    a->dst_col == b->dst_col &&
	    a->promoted_color == b->promoted_color &&
	    a->promoted_piece_type == b->promoted_piece_type;
}

// Pack the castle state into the move.
static inline castle_state_t unpack_castle_state (castle_state_t state)
{
    return state == CASTLE_PREVIOUSLY_NONE ? CASTLE_NONE : state;
}

// Unpack the castle state from the move.
static inline castle_state_t pack_castle_state (castle_state_t state)
{
    return state == CASTLE_NONE ? CASTLE_PREVIOUSLY_NONE : state;
}

static inline castle_state_t current_castle_state (const move_t *move)
{
    return unpack_castle_state(move->current_castle_state);
}

static inline castle_state_t opponent_castle_state (const move_t *move)
{
    return unpack_castle_state(move->opponent_castle_state);
}

static inline void save_current_castle_state (move_t *move, castle_state_t state)
{
    move->current_castle_state = pack_castle_state(state);
}

static inline void save_opponent_castle_state (move_t *move, castle_state_t state)
{
    move->opponent_castle_state = pack_castle_state(state);
}

/*********************************************************************/

extern char *move_str (move_t move);

extern int   move_parse (char *str, color_t who, move_t *ret_move);

#endif /* EVOLVE_CHESS_MOVE_H */