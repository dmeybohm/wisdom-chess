#ifndef EVOLVE_CHESS_PIECE_H
#define EVOLVE_CHESS_PIECE_H

#include <assert.h>

typedef unsigned char   piece_t;

typedef unsigned char   color_t;

enum piece
{
	PIECE_NONE   = 0U,
	PIECE_KING   = 1U,
	PIECE_QUEEN  = 2U,
	PIECE_ROOK   = 3U,
	PIECE_BISHOP = 4U,
	PIECE_KNIGHT = 5U,
	PIECE_PAWN   = 6U,
	PIECE_LAST   = 7U,
};

enum color
{
	COLOR_NONE  = 0U,
	COLOR_WHITE = 1U,
	COLOR_BLACK = 2U,
	COLOR_LAST  = 3U,
};

/* 3 bits for type of piece */
#define PIECE_TYPE_MASK    (0x08U-1)

/* 2 bits for color */
#define PIECE_COLOR_MASK   (0x18U)
#define PIECE_COLOR_SHIFT  (3U)

#define PIECE_COLOR(piece) \
	(((piece) & PIECE_COLOR_MASK) >> PIECE_COLOR_SHIFT)

#define PIECE_TYPE(piece) \
	((piece) & PIECE_TYPE_MASK)

#define PIECE_SHIFT     (5)
#define PIECE_MASK      (0x1fU)
#define NR_PIECES       (6U)

#define MAKE_PIECE(color, type) \
	(((color) << PIECE_COLOR_SHIFT) | ((type) & PIECE_TYPE_MASK))

static inline color_t color_invert (color_t color)
{
    assert (color == COLOR_WHITE || color == COLOR_BLACK);
	return color == COLOR_WHITE ? COLOR_BLACK : COLOR_WHITE;
}

static inline int is_color_invalid (color_t color)
{
	if (color == COLOR_WHITE || color == COLOR_BLACK)
		return 0;

	return 1;
}

char *piece_str (piece_t piece);

static inline char piece_chr (piece_t piece)
{
	enum piece p = PIECE_TYPE(piece);

	switch (p)
	{
	case PIECE_KING:   return 'K';
	case PIECE_QUEEN:  return 'Q';
	case PIECE_ROOK:   return 'R';
	case PIECE_BISHOP: return 'B';
	case PIECE_KNIGHT: return 'N';
	case PIECE_PAWN:   return 'p';
	default:           return '?';
	}
}

#define for_each_color(c) \
	for (c = COLOR_WHITE; c <= COLOR_BLACK; c++)

#define for_each_piece(p) \
	for (p = PIECE_NONE; p < PIECE_LAST; p++)

#endif /* EVOLVE_CHESS_PIECE_H */
