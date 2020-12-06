#ifndef EVOLVE_CHESS_PIECE_H
#define EVOLVE_CHESS_PIECE_H

#include <cassert>
#include <string>

enum class Piece
{
    None,
    King,
    Queen,
    Rook,
    Bishop,
    Knight,
    Pawn,
};

enum class Color
{
    None,
    White,
    Black
};

struct piece_with_color
{
    Piece piece_type : 4;
    Color color : 4;
};

enum color_index
{
	COLOR_INDEX_WHITE = 0U,
	COLOR_INDEX_BLACK = 1U,
};

using color_index_t = int8_t;

using piece_t = struct piece_with_color;

constexpr Piece all_promotable_piece_types[] =
{
    Piece::Bishop,
    Piece::Knight,
    Piece::Rook,
    Piece::Queen
};

constexpr Piece all_piece_types_with_none[] =
{
    Piece::None,
    Piece::King,
    Piece::Queen,
    Piece::Rook,
    Piece::Bishop,
    Piece::Knight,
    Piece::Pawn,
};

constexpr Color all_colors[] =
{
    Color::White,
    Color::Black
};

constexpr Color all_colors_with_none[] =
{
    Color::None,
    Color::White,
    Color::Black,
};

////////////////////////////////////////////////

constexpr piece_t make_piece (Color color, Piece piece_type)
{
    struct piece_with_color piece_with_color = { .piece_type = piece_type, .color = color };
    return piece_with_color;
}

constexpr unsigned int NR_PIECES = 6;

constexpr piece_t piece_and_color_none = make_piece (Color::None, Piece::None);

constexpr int piece_index (Piece piece)
{
    switch (piece)
    {
        case Piece::None:
            return 0;
        case Piece::King:
            return 1;
        case Piece::Queen:
            return 2;
        case Piece::Rook:
            return 3;
        case Piece::Bishop:
            return 4;
        case Piece::Knight:
            return 5;
        case Piece::Pawn:
            return 6;
        default:
            abort();
    }
}

/////////////////////////////////////////////////

constexpr Piece piece_type (piece_t piece)
{
    return piece.piece_type;
}

constexpr Color piece_color (piece_t piece)
{
    return piece.color;
}

constexpr bool is_color_valid (Color who)
{
    return (who == Color::White || who == Color::Black);
}

constexpr Color color_invert (Color who)
{
    assert (is_color_valid(who));
	return who == Color::White ? Color::Black : Color::White;
}

constexpr bool is_color_invalid (Color color)
{
	return !is_color_valid (color);
}

constexpr color_index_t color_index (Color who)
{
	switch (who)
	{
		case Color::White: return COLOR_INDEX_WHITE;
		case Color::Black: return COLOR_INDEX_BLACK;
		default: abort();
	}
}

constexpr color_index_t color_index_with_none (Color who)
{
    switch (who)
    {
        case Color::None:  return 0;
        case Color::White: return COLOR_INDEX_WHITE + 1;
        case Color::Black: return COLOR_INDEX_BLACK + 1;
        default: abort();
    }
}

static inline char piece_char (piece_t piece)
{
	Piece p = piece_type (piece);

	switch (p)
	{
        case Piece::King:   return 'K';
        case Piece::Queen:  return 'Q';
        case Piece::Rook:   return 'R';
        case Piece::Bishop: return 'B';
        case Piece::Knight: return 'N';
        case Piece::Pawn:   return 'p';
    	default:            return '?';
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

std::string to_string (Color who);

#endif // EVOLVE_CHESS_PIECE_H
