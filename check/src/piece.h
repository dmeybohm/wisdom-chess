#ifndef EVOLVE_CHESS_PIECE_H
#define EVOLVE_CHESS_PIECE_H

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstdio>

enum piece_type
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

struct piece_with_color
{
#ifdef __GNUC__
    enum piece_type piece_type : 4;
    enum color color : 3;
#else // MSVC doesn't support unsigned enums :-(
	enum piece_type piece_type;
	enum color color;
#endif
};

enum color_index
{
	COLOR_INDEX_WHITE = 0U,
	COLOR_INDEX_BLACK = 1U,
};

typedef uint8_t   color_index_t;

typedef struct piece_with_color piece_t;

////////////////////////////////////////////////

constexpr piece_t MAKE_PIECE (enum color color, enum piece_type piece_type)
{
    struct piece_with_color piece_with_color = { .piece_type = piece_type, .color = color };
    return piece_with_color;
}

constexpr unsigned int NR_PIECES = 6;

constexpr piece_t PIECE_AND_COLOR_NONE = MAKE_PIECE (COLOR_NONE, PIECE_NONE);

/////////////////////////////////////////////////

constexpr enum piece_type PIECE_TYPE (piece_t piece)
{
    return piece.piece_type;
}

constexpr enum color PIECE_COLOR (piece_t piece)
{
    return piece.color;
}

constexpr bool is_color_valid (enum color who)
{
    return (who == COLOR_WHITE || who == COLOR_BLACK);
}

constexpr enum color color_invert (enum color who)
{
    assert (is_color_valid(who));
	return who == COLOR_WHITE ? COLOR_BLACK : COLOR_WHITE;
}

constexpr bool is_color_invalid (enum color color)
{
	return !is_color_valid (color);
}

constexpr color_index_t color_index (enum color who)
{
    assert (is_color_valid(who));
	switch (who)
	{
	    default:
		case COLOR_WHITE: return COLOR_INDEX_WHITE;
		case COLOR_BLACK: return COLOR_INDEX_BLACK;
	}
}

char *piece_str (piece_t piece);

static inline char piece_chr (piece_t piece)
{
	enum piece_type p = PIECE_TYPE(piece);

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

constexpr bool piece_equals (piece_t a, piece_t b)
{
    return a.color == b.color && a.piece_type == b.piece_type;
}

constexpr bool operator == (piece_t a, piece_t b)
{
    return piece_equals (a, b);
}

constexpr bool operator != (piece_t a, piece_t b)
{
    return !piece_equals (a, b);
}

////////////////////////////////////////////////////

#define for_each_color(c) \
	for (c = COLOR_WHITE; c <= COLOR_BLACK; (c) = static_cast<enum color>(static_cast<int>((c))+ 1))

#define for_each_piece(p) \
	for (p = PIECE_NONE; p < PIECE_LAST; (p) = static_cast<enum piece_type>(static_cast<int>((p)) + 1))

#define for_each_color_index(c) \
    for (c = COLOR_INDEX_WHITE; c <= COLOR_INDEX_BLACK; c++)

#define for_each_promotable_piece(p) \
    for (p = PIECE_QUEEN; \
    p < PIECE_PAWN; \
    (p) = static_cast<enum piece_type>(static_cast<int>((p)) + 1))

#endif // EVOLVE_CHESS_PIECE_H
