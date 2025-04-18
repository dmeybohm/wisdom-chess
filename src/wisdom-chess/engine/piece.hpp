#pragma once

#include "wisdom-chess/engine/global.hpp"

namespace wisdom
{
    class PieceError : public Error
    {
    public:
        explicit PieceError (string extra_info)
            : Error ("Piece error", std::move (extra_info))
        {
        }
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

    inline constexpr std::size_t Num_Piece_Types = 
        static_cast<std::size_t> (Piece::King) + 1;

    enum class Color : int8_t
    {
        None = 0,
        White = 1,
        Black = 2,
    };

    inline constexpr int Color_Index_White = 0;
    inline constexpr int Color_Index_Black = 1;

    using ColorIndex = int8_t;

    [[nodiscard]] constexpr auto
    pieceFromInt8 (int8_t integer) 
        -> Piece
    {
        return static_cast<Piece> (integer);
    }

    [[nodiscard]] constexpr auto
    pieceFromInt (int integer) 
        -> Piece
    {
        return static_cast<Piece> (integer);
    }

    [[nodiscard]] constexpr auto
    colorFromInt8 (int8_t integer) 
        -> Color
    {
        return static_cast<Color> (integer);
    }

    [[nodiscard]] constexpr auto
    colorFromInt (int integer) 
        -> Color
    {
        return static_cast<Color> (integer);
    }

    [[nodiscard]] constexpr auto
    colorFromColorIndex (ColorIndex index) 
        -> Color
    {
        assert (index == Color_Index_White || index == Color_Index_Black);
        return static_cast<Color> (index + 1);
    }

    [[nodiscard]] constexpr auto
    toInt8 (Piece piece) 
        -> int8_t
    {
        return static_cast<int8_t> (piece);
    }

    [[nodiscard]] constexpr auto
    toInt (Piece piece) 
        -> int
    {
        return static_cast<int> (piece);
    }

    [[nodiscard]] constexpr auto
    toInt (Color color) 
        -> int
    {
        return static_cast<int> (color);
    }

    [[nodiscard]] constexpr auto
    toInt8 (Color color) 
        -> int8_t
    {
        return static_cast<int8_t> (color);
    }

    [[nodiscard]] constexpr auto
    isColorValid (Color who) 
        -> bool
    {
        return (who == Color::White || who == Color::Black);
    }

    [[nodiscard]] constexpr auto
    colorIndex (Color who) 
        -> ColorIndex
    {
        assert (who == Color::White || who == Color::Black);
        return narrow_cast<int8_t> (toInt8 (who) - 1);
    }

    [[nodiscard]] constexpr auto
    colorInvert (Color who)
        -> Color
    {
        assert (isColorValid (who));
        uint8_t inverted = !colorIndex (who);
        return colorFromColorIndex (narrow_cast<int8_t> (inverted));
    }

    [[nodiscard]] constexpr auto
    pieceIndex (Piece piece)
        -> int
    {
        auto piece_as_int = static_cast<int8_t> (piece);
        assert (piece_as_int >= toInt8 (Piece::None) && piece_as_int <= toInt8 (Piece::King));
        return piece_as_int;
    }

    // 3 bits for type of piece
    inline constexpr int8_t Piece_Type_Mask = 0x08 - 1;

    // 2 bits for color
    inline constexpr int8_t Piece_Color_Mask = 0x18;
    inline constexpr int8_t Piece_Color_Shift = 3;

    struct ColoredPiece
    {
        int8_t piece_type_and_color;

        static constexpr auto 
        make (Color color, Piece piece_type) noexcept
            -> ColoredPiece
        {
            assert ((piece_type == Piece::None && color == Color::None) ||
                (piece_type != Piece::None && color != Color::None));
            auto color_as_int = toInt8 (color);
            auto piece_as_int = toInt8 (piece_type);
            auto result = narrow_cast<int8_t>(
                (color_as_int << Piece_Color_Shift) |
                    (piece_as_int & Piece_Type_Mask)
            );
            ColoredPiece piece_with_color = { .piece_type_and_color = result };
            return piece_with_color;
        }

        [[nodiscard]] constexpr auto 
        color() const noexcept 
            -> Color
        {
            auto result = narrow_cast<int8_t> (
                (piece_type_and_color & Piece_Color_Mask) >> Piece_Color_Shift
            );
            return colorFromInt8 (result);
        }

        [[nodiscard]] constexpr auto 
        type() const noexcept 
            -> Piece
        {
            auto result = narrow_cast<int8_t>(
                (piece_type_and_color & Piece_Type_Mask)
            );
            return pieceFromInt8 (result);
        }

        [[nodiscard]] friend constexpr auto
        operator== (ColoredPiece first, ColoredPiece second) 
            -> bool
        {
            return first.piece_type_and_color == second.piece_type_and_color;
        }

        [[nodiscard]] friend constexpr auto
        operator!= (ColoredPiece first, ColoredPiece second)
            -> bool
        {
            return !operator== (first, second);
        }
    };
    static_assert (std::is_trivial_v<ColoredPiece>);

    // Order here is significant - it means computer will prefer the piece at the top
    // all else being equal, such as if the promoted piece cannot be saved from capture.
    inline constexpr Piece All_Promotable_Piece_Types[] = {
        Piece::Queen,
        Piece::Rook,
        Piece::Bishop,
        Piece::Knight,
    };

    [[nodiscard]] constexpr auto
    pieceType (ColoredPiece piece) 
        -> Piece
    {
        return piece.type();
    }

    [[nodiscard]] constexpr auto
    pieceColor (ColoredPiece piece) 
        -> Color
    {
        return piece.color();
    }

    inline constexpr ColoredPiece Piece_And_Color_None = ColoredPiece::make (
        Color::None,
        Piece::None
    );

    [[nodiscard]] constexpr auto
    toInt8 (ColoredPiece piece) 
        -> int8_t
    {
        return piece.piece_type_and_color;
    }

    [[nodiscard]] constexpr auto
    pieceFromChar (char p) 
        -> Piece
    {
        switch (p)
        {
            case 'k':
            case 'K':
                return Piece::King;
            case 'q':
            case 'Q':
                return Piece::Queen;
            case 'r':
            case 'R':
                return Piece::Rook;
            case 'b':
            case 'B':
                return Piece::Bishop;
            case 'n':
            case 'N':
                return Piece::Knight;
            case 'p':
            case 'P':
                return Piece::Pawn;
            default:
                throw Error { "Invalid piece character" };
        }
    }

    [[nodiscard]] constexpr auto
    pieceToChar (ColoredPiece piece)
        -> char
    {
        Piece type = pieceType (piece);

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

    [[nodiscard]] auto
    asString (Color who)
        -> string;

    [[nodiscard]] auto
    asString (ColoredPiece piece)
        -> string;

    [[nodiscard]] auto
    asString (Piece piece)
        -> string;

    auto 
    operator<< (std::ostream& ostream, const ColoredPiece& value) 
        -> std::ostream&;
}
