#pragma once

#include "wisdom-chess/engine/global.hpp"

namespace wisdom
{
    class ColoredPiece;

    // A piece type. A class rather than an enum, like Color, so that a
    // Piece cannot be made from zero or another integer: its constructors
    // are private, and only the constants, NullRepresentation and
    // ColoredPiece, which decodes its type bits, use them.
    class Piece
    {
    public:
        // Starts at one, so an empty optional<Piece> or optional<ColoredPiece>
        // is zero.
        enum class Value : int8_t
        {
            Pawn = 1,
            Knight,
            Bishop,
            Rook,
            Queen,
            King
        };

        static const Piece Pawn;
        static const Piece Knight;
        static const Piece Bishop;
        static const Piece Rook;
        static const Piece Queen;
        static const Piece King;

        // For a switch, and comparisons, over the piece types.
        constexpr operator Value() const noexcept
        {
            return my_value;
        }

        // The value, for to_int(). Explicit so it does not leak into
        // arithmetic.
        template <std::signed_integral Target>
        [[nodiscard]] constexpr explicit
        operator Target() const noexcept
        {
            return widen<Target> (to_underlying (my_value));
        }

        // Zero, which no piece type has, marks an empty optional<Piece>.
        struct NullRepresentation
        {
            [[nodiscard]] static constexpr auto
            make() noexcept
                -> Piece
            {
                return Piece {};
            }

            [[nodiscard]] static constexpr auto
            isNull (Piece piece) noexcept
                -> bool
            {
                return piece.my_value == Value {};
            }
        };

    private:
        constexpr Piece() noexcept
            : my_value {}
        {
        }

        constexpr explicit Piece (Value value) noexcept
            : my_value { value }
        {
        }

        friend class ColoredPiece;

        Value my_value;
    };

    inline constexpr Piece Piece::Pawn { Piece::Value::Pawn };
    inline constexpr Piece Piece::Knight { Piece::Value::Knight };
    inline constexpr Piece Piece::Bishop { Piece::Value::Bishop };
    inline constexpr Piece Piece::Rook { Piece::Value::Rook };
    inline constexpr Piece Piece::Queen { Piece::Value::Queen };
    inline constexpr Piece Piece::King { Piece::Value::King };

    static_assert (sizeof (Piece) == 1);
    static_assert (sizeof (optional<Piece>) == sizeof (Piece));
    static_assert (!std::is_default_constructible_v<Piece>);
    static_assert (!std::is_constructible_v<Piece, Piece::Value>);

    inline constexpr int Num_Piece_Types = to_int (Piece::King) - to_int (Piece::Pawn) + 1;

    using ColorIndex = int8_t;

    // White or Black. A class rather than an enum, so that a Color cannot
    // be made from zero or another integer: its constructors are private,
    // and only the White and Black constants and NullRepresentation use
    // them. Code that stores a color as bits decodes it by choosing between
    // the constants.
    class Color
    {
    public:
        enum class Value : int8_t
        {
            White = 1,
            Black = 2,
        };

        static const Color White;
        static const Color Black;

        // For a switch, and comparisons, over the colors.
        constexpr operator Value() const noexcept
        {
            return my_value;
        }

        // The value, for to_int(). Explicit so it does not leak into
        // arithmetic.
        template <std::signed_integral Target>
        [[nodiscard]] constexpr explicit
        operator Target() const noexcept
        {
            return widen<Target> (to_underlying (my_value));
        }

        // Zero, which neither color has, marks an empty optional<Color>.
        struct NullRepresentation
        {
            [[nodiscard]] static constexpr auto
            make() noexcept
                -> Color
            {
                return Color {};
            }

            [[nodiscard]] static constexpr auto
            isNull (Color color) noexcept
                -> bool
            {
                return color.my_value == Value {};
            }
        };

    private:
        constexpr Color() noexcept
            : my_value {}
        {
        }

        constexpr explicit Color (Value value) noexcept
            : my_value { value }
        {
        }

        Value my_value;
    };

    inline constexpr Color Color::White { Color::Value::White };
    inline constexpr Color Color::Black { Color::Value::Black };

    static_assert (sizeof (Color) == 1);
    static_assert (sizeof (optional<Color>) == sizeof (Color));
    static_assert (!std::is_default_constructible_v<Color>);
    static_assert (!std::is_constructible_v<Color, Color::Value>);

    inline constexpr int Color_Index_White = 0;
    inline constexpr int Color_Index_Black = 1;

    [[nodiscard]] constexpr auto
    colorIndex (Color who) noexcept
        -> ColorIndex
    {
        EXPECTS( who == Color::White || who == Color::Black );
        return narrow_debug<int8_t> (to_int (who) - 1);
    }

    [[nodiscard]] constexpr auto
    colorInvert (Color who) noexcept
        -> Color
    {
        return who == Color::White ? Color::Black : Color::White;
    }

    [[nodiscard]] constexpr auto
    pieceIndex (Piece piece) noexcept
        -> int
    {
        return to_int (piece) - to_int (Piece::Pawn);
    }

    // 3 bits for type of piece
    inline constexpr int8_t Piece_Type_Mask = 0x08 - 1;

    // 2 bits for color, above the type
    inline constexpr int8_t Piece_Color_Shift = 3;

    // The bits a color occupies in a ColoredPiece.
    [[nodiscard]] constexpr auto
    colorBits (Color who) noexcept
        -> int8_t
    {
        return narrow_debug<int8_t> (to_int (who) << Piece_Color_Shift);
    }

    // A piece of either color. Built only by make(), so it always holds a
    // valid color and type, and color() and type() need no check.
    class ColoredPiece
    {
    public:
        // Every piece has a color and type, so zero marks an empty
        // optional<ColoredPiece>, such as an empty square.
        struct NullRepresentation
        {
            [[nodiscard]] static constexpr auto
            make() noexcept
                -> ColoredPiece
            {
                return ColoredPiece { 0 };
            }

            [[nodiscard]] static constexpr auto
            isNull (ColoredPiece piece) noexcept
                -> bool
            {
                return piece.my_piece_type_and_color == 0;
            }
        };

        // The packed type and color, for to_int(). Explicit so the packed
        // value does not leak into arithmetic or a bool conversion.
        template <std::signed_integral Target>
        [[nodiscard]] constexpr explicit
        operator Target() const noexcept
        {
            return widen<Target> (my_piece_type_and_color);
        }

        [[nodiscard]] static constexpr auto
        make (Color color, Piece piece_type) noexcept
            -> ColoredPiece
        {
            EXPECTS( color == Color::White || color == Color::Black );
            ASSERT( piece_type >= Piece::Pawn && piece_type <= Piece::King );
            auto piece_as_int = to_int (piece_type);
            return ColoredPiece {
                narrow_debug<int8_t> (colorBits (color) | (piece_as_int & Piece_Type_Mask))
            };
        }

        [[nodiscard]] constexpr auto
        color() const noexcept
            -> Color
        {
            return (my_piece_type_and_color & colorBits (Color::Black)) ? Color::Black : Color::White;
        }

        [[nodiscard]] constexpr auto
        type() const noexcept
            -> Piece
        {
            auto bits = narrow_debug<int8_t> (my_piece_type_and_color & Piece_Type_Mask);
            return Piece { to_enum_debug<Piece::Value> (bits) };
        }

        [[nodiscard]] friend constexpr auto
        operator== (ColoredPiece first, ColoredPiece second) noexcept
            -> bool
        {
            return first.my_piece_type_and_color == second.my_piece_type_and_color;
        }

        [[nodiscard]] friend constexpr auto
        operator!= (ColoredPiece first, ColoredPiece second) noexcept
            -> bool
        {
            return !operator== (first, second);
        }

    private:
        constexpr explicit ColoredPiece (int8_t piece_type_and_color) noexcept
            : my_piece_type_and_color { piece_type_and_color }
        {
        }

        int8_t my_piece_type_and_color;
    };
    static_assert (std::is_trivially_copyable_v<ColoredPiece>);
    static_assert (!std::is_default_constructible_v<ColoredPiece>);
    static_assert (!std::is_constructible_v<ColoredPiece, int8_t>);
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
        }
        PRECONDITION_FAILED( "a piece type" );
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
