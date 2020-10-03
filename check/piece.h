#ifndef EVOLVE_CHESS_PIECE_H
#define EVOLVE_CHESS_PIECE_H

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint8_t   piece_t;

typedef uint8_t   color_t;

typedef uint8_t   color_index_t;

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

enum color_index
{
	COLOR_INDEX_WHITE = 0U,
	COLOR_INDEX_BLACK = 1U,
};

// 3 bits for type of piece
#define PIECE_TYPE_MASK     (0x08U-1U)

// 2 bits for color
#define PIECE_COLOR_MASK    (0x18U)
#define PIECE_COLOR_SHIFT   (3U)

#define PIECE_SHIFT         (5U)
#define PIECE_MASK          (0x1fU)
#define NR_PIECES           (6U)

static inline piece_t MAKE_PIECE (enum color color, enum piece piece)
{
    uint8_t raw_piece = (piece & PIECE_TYPE_MASK);
    uint8_t raw_color = (color << PIECE_COLOR_SHIFT);
    return (enum piece)(raw_piece | raw_color);
}

static inline enum piece PIECE_TYPE (piece_t piece)
{
    unsigned int raw_piece = piece;
    return (enum piece) (raw_piece & PIECE_TYPE_MASK);
}

static inline enum color PIECE_COLOR (piece_t piece)
{
    unsigned int raw_piece = piece;
	return (enum color) ((raw_piece & PIECE_COLOR_MASK) >> PIECE_COLOR_SHIFT);
}

static inline color_t color_invert (color_t color)
{
    assert (color == COLOR_WHITE || color == COLOR_BLACK);
	return color == COLOR_WHITE ? COLOR_BLACK : COLOR_WHITE;
}

static inline color_index_t color_index (color_t who)
{
	switch (who)
	{
		case COLOR_WHITE: return COLOR_INDEX_WHITE;
		case COLOR_BLACK: return COLOR_INDEX_BLACK;
		default: abort();
	}
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

#endif // EVOLVE_CHESS_PIECE_H
