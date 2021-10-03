#ifndef WISDOM_CHESS_PIECE_HPP
#define WISDOM_CHESS_PIECE_HPP

#include "global.hpp"

namespace wisdom
{
    class PieceError : public Error
    {
    public:
        explicit PieceError (string extra_info) :
            Error("Piece error", std::move (extra_info))
        {}
    };

    enum class Piece
    {
        None,
        Pawn,
        Knight,
        Bishop,
        Rook,
        Queen,
        King,
    };

    enum class Color
    {
        None,
        White,
        Black
    };

    struct ColoredPiece
    {
        Piece piece_type: 4;
        Color color: 4;
    };

    constexpr int Color_Index_White = 0;
    constexpr int Color_Index_Black = 1;

    using ColorIndex = int8_t;

    static_assert(std::is_trivial<ColoredPiece>::value);

    // Order here is significant - it means computer will prefer the piece at the top
    // all else being equal, such as if the promoted piece cannot be saved from capture.
    constexpr Piece All_Promotable_Piece_Types[] = {
            Piece::Queen,
            Piece::Rook,
            Piece::Bishop,
            Piece::Knight,
    };

    constexpr auto make_piece (Color color, Piece piece_type) -> ColoredPiece
    {
        ColoredPiece piece_with_color = { .piece_type = piece_type, .color = color };
        return piece_with_color;
    }

    constexpr ColoredPiece Piece_And_Color_None = make_piece (Color::None, Piece::None);

    constexpr auto piece_index (Piece piece) -> int
    {
        switch (piece)
        {
            case Piece::None:
                return 0;
            case Piece::Pawn:
                return 1;
            case Piece::Knight:
                return 2;
            case Piece::Bishop:
                return 3;
            case Piece::Rook:
                return 4;
            case Piece::Queen:
                return 5;
            case Piece::King:
                return 6;
            default:
                throw PieceError { "Invalid piece type" };
        }
    }

    constexpr auto piece_type (ColoredPiece piece) -> Piece
    {
        return piece.piece_type;
    }

    constexpr auto piece_color (ColoredPiece piece) -> Color
    {
        return piece.color;
    }

    constexpr auto is_color_valid (Color who) -> bool
    {
        return (who == Color::White || who == Color::Black);
    }

    constexpr auto color_invert (Color who) -> Color
    {
        assert (is_color_valid (who));
        return who == Color::White ? Color::Black : Color::White;
    }

    constexpr auto color_index (Color who) -> ColorIndex
    {
        switch (who)
        {
            case Color::White:
                return Color_Index_White;
            case Color::Black:
                return Color_Index_Black;
            default:
                throw PieceError { "Invalid color index" };
        }
    }

    constexpr auto piece_from_char (char p) -> Piece
    {
        switch (p)
        {
            case 'k': case 'K':
                return Piece::King;
            case 'q': case 'Q':
                return Piece::Queen;
            case 'r': case 'R':
                return Piece::Rook;
            case 'b': case 'B':
                return Piece::Bishop;
            case 'n': case 'N':
                return Piece::Knight;
            case 'p': case 'P':
                return Piece::Pawn;
            default:
                throw Error { "Invalid piece character"};
        }
    }

    constexpr auto piece_char (ColoredPiece piece) -> char
    {
        Piece p = piece_type (piece);

        switch (p)
        {
            case Piece::King:
                return 'K';
            case Piece::Queen:
                return 'Q';
            case Piece::Rook:
                return 'R';
            case Piece::Bishop:
                return 'B';
            case Piece::Knight:
                return 'N';
            case Piece::Pawn:
                return 'p';
            default:
                return '?';
        }
    }

    constexpr auto piece_equals (ColoredPiece a, ColoredPiece b) -> bool
    {
        return a.color == b.color && a.piece_type == b.piece_type;
    }

    constexpr bool operator== (ColoredPiece a, ColoredPiece b)
    {
        return piece_equals (a, b);
    }

    constexpr bool operator!= (ColoredPiece a, ColoredPiece b)
    {
        return !piece_equals (a, b);
    }

    auto to_string (Color who) -> string;
    auto to_string (ColoredPiece piece) -> string;
    auto to_string (Piece piece) -> string;

    void play (Color human_player);

    std::ostream &operator<< (std::ostream &os, const ColoredPiece &value);
}

#endif // WISDOM_CHESS_PIECE_HPP
