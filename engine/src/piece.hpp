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
        King
    };

    static constexpr std::size_t Num_Pieces = static_cast<std::size_t> (Piece::King) + 1;

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

    constexpr auto to_int (Piece piece) -> int
    {
        return static_cast<int>(piece);
    }

    constexpr auto to_int (Color color) -> int
    {
        return static_cast<int>(color);
    }

    constexpr auto to_int8 (Color color) -> int8_t
    {
        return static_cast<int8_t>(color);
    }

    constexpr auto make_piece (Color color, Piece piece_type) noexcept
        -> ColoredPiece
    {
        assert ((piece_type == Piece::None && color == Color::None) ||
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
        assert (piece_as_int >= to_int8 (Piece::None) && piece_as_int <= to_int8 (Piece::King));
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

    constexpr auto color_from_int (int integer) -> Color
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
        return gsl::narrow_cast<int8_t>(to_int8 (who) - 1);
    }

    constexpr auto color_invert (Color who) -> Color
    {
        assert (is_color_valid (who));
        return color_from_color_index (
            gsl::narrow_cast<int8_t>(
                !color_index (who)
            )
        );
    }

    constexpr auto to_int8 (ColoredPiece piece) -> int8_t
    {
        return piece.piece_type_and_color;
    }

    constexpr auto to_colored_piece_shifted (Color who) -> int8_t
    {
       return gsl::narrow_cast<int8_t>(to_int8 (who) << Piece_Color_Shift);
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
        Piece type = piece_type (piece);

        switch (type)
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

    constexpr auto operator== (ColoredPiece first, ColoredPiece second) -> bool
    {
        return first.piece_type_and_color == second.piece_type_and_color;
    }

    constexpr bool operator!= (ColoredPiece first, ColoredPiece second)
    {
        return !operator== (first, second);
    }

    auto to_string (Color who) -> string;
    auto to_string (ColoredPiece piece) -> string;
    auto to_string (Piece piece) -> string;

    auto operator<< (std::ostream& ostream, const ColoredPiece& value) -> std::ostream&;
}

#endif // WISDOM_CHESS_PIECE_HPP
