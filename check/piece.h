#ifndef EVOLVE_CHESS_PIECE_H
#define EVOLVE_CHESS_PIECE_H

typedef unsigned char   piece_t;

typedef unsigned char   color_t;

enum piece
{
	PIECE_NONE   = 0,
	PIECE_KING   = 1,
	PIECE_QUEEN  = 2,
	PIECE_ROOK   = 3,
	PIECE_BISHOP = 4,
	PIECE_KNIGHT = 5,
	PIECE_PAWN   = 6,
	PIECE_LAST   = 7,
};

enum color
{
	COLOR_WHITE = 0,
	COLOR_BLACK = 1,
	COLOR_LAST  = 2,
	COLOR_NONE  = 3,
};

/* 3 bits for type of piece */
#define PIECE_TYPE_MASK    (0x08-1)

/* 2 bits for color */
#define PIECE_COLOR_MASK   (0x18)
#define PIECE_COLOR_SHIFT  (3)

#define PIECE_COLOR(piece) \
	(((piece) & PIECE_COLOR_MASK) >> PIECE_COLOR_SHIFT)

#define PIECE_TYPE(piece) \
	((piece) & PIECE_TYPE_MASK)

#define PIECE_SHIFT     (5)
#define PIECE_MASK      (0x1f)

#define MAKE_PIECE(color, type) \
	(((color) << PIECE_COLOR_SHIFT) | ((type) & PIECE_TYPE_MASK))

static inline color_t color_invert (color_t color)
{
	return !color;
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

#endif /* EVOLVE_CHESS_PIECE_H */
