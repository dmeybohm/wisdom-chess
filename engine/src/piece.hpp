#ifndef WISDOM_CHESS_PIECE_HPP
#define WISDOM_CHESS_PIECE_HPP

#include "global.hpp"

namespace wisdom
{
    class PieceError : public Error
    {
    public:
        explicit PieceError (std::string extra_info) :
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

    constexpr ColoredPiece make_piece (Color color, Piece piece_type)
    {
        ColoredPiece piece_with_color = { .piece_type = piece_type, .color = color };
        return piece_with_color;
    }

    constexpr ColoredPiece Piece_And_Color_None = make_piece (Color::None, Piece::None);

    constexpr int to_int (Piece piece)
    {
        return static_cast<int> (piece);
    }

    constexpr int piece_index (Piece piece)
    {
        switch (piece)
        {
            case Piece::None: return 0;
            case Piece::King: return 1;
            case Piece::Queen: return 2;
            case Piece::Rook: return 3;
            case Piece::Bishop: return 4;
            case Piece::Knight: return 5;
            case Piece::Pawn: return 6;
            default: throw PieceError {
                "Invalid piece type: " + std::to_string (to_int (piece))
            };
        }
    }

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
        assert (is_color_valid (who));
        return who == Color::White ? Color::Black : Color::White;
    }

    constexpr int to_int (Color who)
    {
        return static_cast<int>(who);
    }

    constexpr ColorIndex color_index (Color who)
    {
        switch (who)
        {
            case Color::White: return Color_Index_White;
            case Color::Black: return Color_Index_Black;
            default: throw PieceError {
                    "Invalid color index: " + std::to_string (to_int (who))
            };
        }
    }

    constexpr auto piece_from_char (char p) -> Piece
    {
        switch (toupper (p))
        {
            case 'K': return Piece::King;
            case 'Q': return Piece::Queen;
            case 'R': return Piece::Rook;
            case 'B': return Piece::Bishop;
            case 'N': return Piece::Knight;
            case 'P': return Piece::Pawn;
        }
        abort();
    }

    constexpr char piece_char (ColoredPiece piece)
    {
        Piece p = piece_type (piece);

        switch (p)
        {
            case Piece::King: return 'K';
            case Piece::Queen: return 'Q';
            case Piece::Rook: return 'R';
            case Piece::Bishop: return 'B';
            case Piece::Knight: return 'N';
            case Piece::Pawn: return 'p';
            default: return '?';
        }
    }

    constexpr bool piece_equals (ColoredPiece a, ColoredPiece b)
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

    std::string to_string (Color who);
    std::string to_string (ColoredPiece piece);
    std::string to_string (Piece piece);

    void play (Color human_player);

    std::ostream &operator<< (std::ostream &os, const ColoredPiece &value);
}

#endif // WISDOM_CHESS_PIECE_HPP
