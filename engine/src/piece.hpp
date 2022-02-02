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

    enum class Piece : int8_t
    {
        None = 0,
        Pawn,
        Knight,
        Bishop,
        Rook,
        Queen,
        King,
    };

    enum class Color : int8_t
    {
        None = 0,
        White = 1,
        Black = 2,
    };

    struct ColoredPiece
    {
        int8_t piece_type_and_color;
    };

    static constexpr int Color_Index_White = 0;
    static constexpr int Color_Index_Black = 1;

    using ColorIndex = int8_t;

    static_assert(std::is_trivial<ColoredPiece>::value);

    // Order here is significant - it means computer will prefer the piece at the top
    // all else being equal, such as if the promoted piece cannot be saved from capture.
    static constexpr Piece All_Promotable_Piece_Types[] = {
            Piece::Queen,
            Piece::Rook,
            Piece::Bishop,
            Piece::Knight,
    };

    // 3 bits for type of piece
    static constexpr int8_t Piece_Type_Mask = 0x08-1;

    // 2 bits for color
    static constexpr int8_t Piece_Color_Mask = 0x18;
    static constexpr int8_t Piece_Color_Shift = 3;

    constexpr auto to_int8 (Piece piece) -> int8_t
    {
        return static_cast<int8_t>(piece);
    }

    constexpr auto to_int8 (Color color) -> int8_t
    {
        return static_cast<int8_t>(color);
    }

    constexpr auto make_piece (Color color, Piece piece_type) noexcept
        -> ColoredPiece
    {
        assert ((piece_type == Piece::None && color == Piece::None) ||
                (piece_type != Piece::None && color != Color::None));
        auto color_as_int = to_int8 (color);
        auto piece_as_int = to_int8 (piece_type);
        auto result = gsl::narrow_cast<int8_t>(
            (color_as_int << Piece_Color_Shift) |
            (piece_as_int & Piece_Type_Mask)
        );
        ColoredPiece piece_with_color = { .piece_type_and_color = result };
        return piece_with_color;
    }

    constexpr ColoredPiece Piece_And_Color_None = make_piece (Color::None, Piece::None);

    constexpr auto piece_index (Piece piece) -> int
    {
        auto piece_as_int = static_cast<int8_t>(piece);
        assert (piece_as_int >= to_int (Piece::None) && piece_as_int <= to_int (Piece::King));
        return piece_as_int;
    }

    constexpr auto piece_from_int8 (int8_t integer) -> Piece
    {
        return static_cast<Piece>(integer);
    }

    constexpr auto color_from_int8 (int8_t integer) -> Color
    {
        return static_cast<Color>(integer);
    }

    constexpr auto color_from_color_index (ColorIndex index) -> Color
    {
        assert (index == Color_Index_White || index == Color_Index_Black);
        return static_cast<Color>(index + 1);
    }

    constexpr auto piece_type (ColoredPiece piece) -> Piece
    {
        auto result = gsl::narrow_cast<int8_t>(
            (piece.piece_type_and_color & Piece_Type_Mask)
        );
        return piece_from_int8 (result);
    }

    constexpr auto piece_color (ColoredPiece piece) -> Color
    {
        auto result = gsl::narrow_cast<int8_t>(
            (piece.piece_type_and_color & Piece_Color_Mask) >> Piece_Color_Shift
        );
        return color_from_int8 (result);
    }

    constexpr auto is_color_valid (Color who) -> bool
    {
        return (who == Color::White || who == Color::Black);
    }

    constexpr auto color_index (Color who) -> ColorIndex
    {
        assert (who == Color::White || who == Color::Black);
        return to_int8 (who) - 1;
    }

    constexpr auto color_invert (Color who) -> Color
    {
        assert (is_color_valid (who));
        return color_from_color_index (!(color_index (who) - 1));
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
        return a.piece_type_and_color == b.piece_type_and_color;
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
