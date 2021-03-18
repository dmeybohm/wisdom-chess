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

struct colored_piece
{
    Piece piece_type : 4;
    Color color : 4;
};

enum color_index
{
	COLOR_INDEX_WHITE = 0U,
	COLOR_INDEX_BLACK = 1U,
};

using ColorIndex = int8_t;

using ColoredPiece = struct colored_piece;

// Order here is significant - it means computer will prefer the piece at the top
// all else being equal, such as if the promoted piece cannot be saved from capture.
constexpr Piece all_promotable_piece_types[] =
{
    Piece::Queen,
    Piece::Rook,
    Piece::Bishop,
    Piece::Knight,
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

constexpr ColoredPiece make_piece (Color color, Piece piece_type)
{
    ColoredPiece piece_with_color = { .piece_type = piece_type, .color = color };
    return piece_with_color;
}

constexpr unsigned int NR_PIECES = 6;

constexpr ColoredPiece piece_and_color_none = make_piece (Color::None, Piece::None);

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

constexpr Piece piece_type (ColoredPiece piece)
{
    return piece.piece_type;
}

constexpr Color piece_color (ColoredPiece piece)
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

constexpr ColorIndex color_index (Color who)
{
	switch (who)
	{
		case Color::White: return COLOR_INDEX_WHITE;
		case Color::Black: return COLOR_INDEX_BLACK;
		default: abort();
	}
}

constexpr ColorIndex color_index_with_none (Color who)
{
    switch (who)
    {
        case Color::None:  return 0;
        case Color::White: return COLOR_INDEX_WHITE + 1;
        case Color::Black: return COLOR_INDEX_BLACK + 1;
        default: abort();
    }
}

static inline char piece_char (ColoredPiece piece)
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

constexpr bool piece_equals (ColoredPiece a, ColoredPiece b)
{
    return a.color == b.color && a.piece_type == b.piece_type;
}

constexpr bool operator == (ColoredPiece a, ColoredPiece b)
{
    return piece_equals (a, b);
}

constexpr bool operator != (ColoredPiece a, ColoredPiece b)
{
    return !piece_equals (a, b);
}

std::string to_string (Color who);

#endif // EVOLVE_CHESS_PIECE_H
