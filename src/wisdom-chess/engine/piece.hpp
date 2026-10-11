#pragma once

#include "wisdom-chess/engine/global.hpp"

namespace wisdom
{
    // Starts at one, so an empty optional<Piece> or optional<ColoredPiece>
    // is zero.
    enum class Piece : int8_t
    {
        Pawn = 1,
        Knight,
        Bishop,
        Rook,
        Queen,
        King
    };

    [[nodiscard]] constexpr auto
    nullRepresentation (std::type_identity<Piece>) noexcept
        -> Piece
    {
        return Piece {};
    }
    static_assert (sizeof (optional<Piece>) == sizeof (Piece));

    inline constexpr int Num_Piece_Types = to_int (Piece::King) + 1;

    // Starts at one, so an empty optional<Color> is zero.
    enum class Color : int8_t
    {
        White = 1,
        Black = 2,
    };

    [[nodiscard]] constexpr auto
    nullRepresentation (std::type_identity<Color>) noexcept
        -> Color
    {
        return Color {};
    }
    static_assert (sizeof (optional<Color>) == sizeof (Color));

    inline constexpr int Color_Index_White = 0;
    inline constexpr int Color_Index_Black = 1;

    using ColorIndex = int8_t;

    [[nodiscard]] constexpr auto
    pieceFromInt8 (int8_t integer) noexcept
        -> Piece
    {
        ASSERT( integer >= to_int (Piece::Pawn) && integer <= to_int (Piece::King) );
        return to_enum_debug<Piece> (integer);
    }

    [[nodiscard]] constexpr auto
    pieceFromInt (int integer) noexcept
        -> Piece
    {
        return to_enum_debug<Piece> (integer);
    }

    [[nodiscard]] constexpr auto
    colorFromInt8 (int8_t integer) noexcept
        -> Color
    {
        ASSERT( integer >= to_int (Color::White) && integer <= to_int (Color::Black) );
        return to_enum_debug<Color> (integer);
    }

    [[nodiscard]] constexpr auto
    colorFromInt (int integer) noexcept
        -> Color
    {
        return to_enum_debug<Color> (integer);
    }

    [[nodiscard]] constexpr auto
    colorFromColorIndex (ColorIndex index) noexcept
        -> Color
    {
        ASSERT( index == Color_Index_White || index == Color_Index_Black );
        return to_enum_debug<Color> (index + 1);
    }

    [[nodiscard]] constexpr auto
    isColorValid (Color who) noexcept
        -> bool
    {
        return (who == Color::White || who == Color::Black);
    }

    [[nodiscard]] constexpr auto
    colorIndex (Color who) noexcept
        -> ColorIndex
    {
        ASSERT( who == Color::White || who == Color::Black );
        return narrow_debug<int8_t> (to_int (who) - 1);
    }

    [[nodiscard]] constexpr auto
    colorInvert (Color who) noexcept
        -> Color
    {
        ASSERT( isColorValid (who) );
        uint8_t inverted = !colorIndex (who);
        return colorFromColorIndex (narrow_debug<int8_t> (inverted));
    }

    [[nodiscard]] constexpr auto
    pieceIndex (Piece piece) noexcept
        -> int
    {
        auto piece_as_int = to_underlying (piece);
        ASSERT( piece_as_int >= to_int (Piece::Pawn) && piece_as_int <= to_int (Piece::King) );
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

        // Every piece has a color and type, so zero marks an empty
        // optional<ColoredPiece>, such as an empty square.
        struct NullRepresentation
        {
            [[nodiscard]] static constexpr auto
            make() noexcept
                -> ColoredPiece
            {
                return ColoredPiece { .piece_type_and_color = 0 };
            }

            [[nodiscard]] static constexpr auto
            isNull (ColoredPiece piece) noexcept
                -> bool
            {
                return piece.piece_type_and_color == 0;
            }
        };

        // The packed type and color, for to_int(). Explicit so the packed
        // value does not leak into arithmetic or a bool conversion.
        template <std::signed_integral Target>
        [[nodiscard]] constexpr explicit
        operator Target() const noexcept
        {
            return widen<Target> (piece_type_and_color);
        }

        [[nodiscard]] static constexpr auto
        make (Color color, Piece piece_type) noexcept
            -> ColoredPiece
        {
            ASSERT( isColorValid (color) );
            ASSERT( piece_type >= Piece::Pawn && piece_type <= Piece::King );
            auto color_as_int = to_int (color);
            auto piece_as_int = to_int (piece_type);
            auto result = narrow_debug<int8_t>(
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
            auto result = narrow_debug<int8_t> (
                (piece_type_and_color & Piece_Color_Mask) >> Piece_Color_Shift
            );
            return colorFromInt8 (result);
        }

        [[nodiscard]] constexpr auto
        type() const noexcept
            -> Piece
        {
            auto result = narrow_debug<int8_t>(
                (piece_type_and_color & Piece_Type_Mask)
            );
            return pieceFromInt8 (result);
        }

        [[nodiscard]] friend constexpr auto
        operator== (ColoredPiece first, ColoredPiece second) noexcept
            -> bool
        {
            return first.piece_type_and_color == second.piece_type_and_color;
        }

        [[nodiscard]] friend constexpr auto
        operator!= (ColoredPiece first, ColoredPiece second) noexcept
            -> bool
        {
            return !operator== (first, second);
        }
    };
    static_assert (std::is_trivial_v<ColoredPiece>);
    static_assert (sizeof (optional<ColoredPiece>) == sizeof (ColoredPiece));

    // Order here is significant - it means computer will prefer the piece at the top
    // all else being equal, such as if the promoted piece cannot be saved from capture.
    inline constexpr Piece All_Promotable_Piece_Types[] = {
        Piece::Queen,
        Piece::Rook,
        Piece::Bishop,
        Piece::Knight,
    };

    [[nodiscard]] constexpr auto
    pieceType (ColoredPiece piece) noexcept
        -> Piece
    {
        return piece.type();
    }

    [[nodiscard]] constexpr auto
    pieceColor (ColoredPiece piece) noexcept
        -> Color
    {
        return piece.color();
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
                PRECONDITION_FAILED( "a piece character" );
        }
    }

    [[nodiscard]] constexpr auto
    pieceToChar (Piece type) noexcept
        -> char
    {
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

    [[nodiscard]] constexpr auto
    pieceToChar (ColoredPiece piece) noexcept
        -> char
    {
        return pieceToChar (pieceType (piece));
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
