#ifndef EVOLVE_CHESS_MOVE_H
#define EVOLVE_CHESS_MOVE_H

#include <string.h>
#include <assert.h>

#include "coord.h"
#include "piece.h"

enum castle
{
	CASTLE_NOTCASTLED    = 0,
	CASTLE_CASTLED       = 0x01,
	CASTLE_KINGSIDE      = 0x02,
	CASTLE_QUEENSIDE     = 0x04,
};

#define CASTLE_MASK     (0x8-1)

typedef struct move
{
	coord_t    src;
	coord_t    dst;

	piece_t    taken;
	piece_t    promoted;
} move_t;

static inline coord_t MOVE_SRC (move_t mv)
{
	return mv.src;
}

static inline coord_t MOVE_DST (move_t mv)
{
	return mv.dst;
}

static inline int is_promoting_move (move_t *move)
{
	return (PIECE_TYPE (move->promoted) != PIECE_NONE);
}

static inline piece_t move_get_promoted (move_t *move)
{
	return move->promoted & PIECE_MASK;
}

static inline piece_t move_get_taken (move_t *move)
{
	return move->taken & PIECE_MASK;
#if 0
	return move->taken;
#endif
}

static inline void move_set_taken (move_t *move, piece_t taken)
{
	move->taken = (move->taken & ~PIECE_MASK) | taken;
#if 0
	move->taken = taken;
#endif
}

static inline int is_capture_move (move_t *move)
{
#if 0
	printf ("is_capture_move: piece = %d\n", move->taken);
	printf ("piece type = %d\n", PIECE_TYPE (move->taken));
	printf ("PIECE_TYPE (move->taken) != PIECE_NONE = %d\n",
		PIECE_TYPE (move->taken) != PIECE_NONE);
#endif
	return (PIECE_TYPE (move->taken) != PIECE_NONE);
}

static inline void move_set_en_passant (move_t *move)
{
	/* use the 6th bit of ->promoted */
	move->promoted |= 0x1 << PIECE_SHIFT;
}

static inline int is_en_passant_move (move_t *move)
{
	return move->promoted & (0x1 << PIECE_SHIFT);
}

static inline void move_set_castling (move_t *move)
{
	/* use the 7th bit of ->promoted */
	move->promoted |= 0x1 << (PIECE_SHIFT+1);
}

static inline void move_set_castle_state (move_t *move, enum castle c_state)
{
	enum piece type;
	enum color color;
	piece_t old_piece;
	move_t  old_move;

	old_piece = move->taken;
	old_move  = *move;

	type = PIECE_TYPE (move->taken);
	color = PIECE_COLOR (move->taken);

	/* use the 6th bit of taken */
	move->taken = (move->taken & ~(CASTLE_MASK << PIECE_SHIFT)) |
		      ((c_state & CASTLE_MASK) << PIECE_SHIFT);

	assert (type == PIECE_TYPE (move->taken));
	assert (color == PIECE_COLOR (move->taken));
}

static inline enum castle move_get_castle_state (move_t *move)
{
	return (move->taken >> PIECE_SHIFT) & CASTLE_MASK;
}

static inline int is_castling_move (move_t *move)
{
	return move->promoted & (0x1 << (PIECE_SHIFT+1));
}

static inline move_t move_promote (move_t move, piece_t piece)
{
	move.promoted = (move.promoted & ~PIECE_MASK) | piece;

	return move;
}

/* run-of-the-mill move with no promotion involved */
static inline move_t move_create (unsigned char src_row, unsigned char src_col,
                                  unsigned char dst_row, unsigned char dst_col)
{
	move_t tmp;
       
	tmp.src      = coord_create (src_row, src_col);
	tmp.dst      = coord_create (dst_row, dst_col);

	tmp.promoted = MAKE_PIECE (COLOR_NONE, PIECE_NONE);
	tmp.taken    = MAKE_PIECE (COLOR_NONE, PIECE_NONE);

	return tmp;
}

static inline int is_null_move (move_t move)
{
	/* no move has the same position for src and dst */
	return (move.src == 0 && move.dst == 0);
}

static inline void move_nullify (move_t *move)
{
	*move = move_create (0, 0, 0, 0);
}

static inline int move_equal (move_t *a, move_t *b)
{
	/* Hm, don't compare the taken piece */
	return a->src == b->src && a->dst == b->dst && 
	       a->promoted == b->promoted;
	
}

extern char *move_str (move_t move);

extern int   move_parse (char *str, color_t who, move_t *ret_move);

#endif /* EVOLVE_CHESS_MOVE_H */
