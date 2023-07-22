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

    static constexpr std::size_t Num_Piece_Types = static_cast<std::size_t> (Piece::King) + 1;

    enum class Color : int8_t
    {
        None = 0,
        White = 1,
        Black = 2,
    };

    static constexpr int Color_Index_White = 0;
    static constexpr int Color_Index_Black = 1;

    using ColorIndex = int8_t;

    constexpr auto pieceFromInt8 (int8_t integer) -> Piece
    {
        return static_cast<Piece>(integer);
    }

    constexpr auto pieceFromInt (int integer) -> Piece
    {
        return static_cast<Piece>(integer);
    }

    constexpr auto colorFromInt8 (int8_t integer) -> Color
    {
        return static_cast<Color>(integer);
    }

    constexpr auto colorFromInt (int integer) -> Color
    {
        return static_cast<Color>(integer);
    }

    constexpr auto colorFromColorIndex (ColorIndex index) -> Color
    {
        assert (index == Color_Index_White || index == Color_Index_Black);
        return static_cast<Color>(index + 1);
    }

    constexpr auto toInt8 (Piece piece) -> int8_t
    {
        return static_cast<int8_t>(piece);
    }

    constexpr auto toInt (Piece piece) -> int
    {
        return static_cast<int>(piece);
    }

    constexpr auto toInt (Color color) -> int
    {
        return static_cast<int>(color);
    }

    constexpr auto toInt8 (Color color) -> int8_t
    {
        return static_cast<int8_t>(color);
    }

    constexpr auto isColorValid (Color who) -> bool
    {
        return (who == Color::White || who == Color::Black);
    }

    constexpr auto colorIndex (Color who) -> ColorIndex
    {
        assert (who == Color::White || who == Color::Black);
        return gsl::narrow_cast<int8_t>(toInt8 (who) - 1);
    }

    constexpr auto colorInvert (Color who) -> Color
    {
        assert (isColorValid (who));
        return colorFromColorIndex (gsl::narrow_cast<int8_t> (!colorIndex (who)));
    }

    constexpr auto pieceIndex (Piece piece) -> int
    {
        auto piece_as_int = static_cast<int8_t>(piece);
        assert (piece_as_int >= toInt8 (Piece::None) && piece_as_int <= toInt8 (Piece::King));
        return piece_as_int;
    }

    // 3 bits for type of piece
    static constexpr int8_t Piece_Type_Mask = 0x08-1;

    // 2 bits for color
    static constexpr int8_t Piece_Color_Mask = 0x18;
    static constexpr int8_t Piece_Color_Shift = 3;

    struct ColoredPiece
    {
        int8_t piece_type_and_color;

        static constexpr auto make (Color color, Piece piece_type) noexcept
            -> ColoredPiece
        {
            assert ((piece_type == Piece::None && color == Color::None) ||
                (piece_type != Piece::None && color != Color::None));
            auto color_as_int = toInt8 (color);
            auto piece_as_int = toInt8 (piece_type);
            auto result = gsl::narrow_cast<int8_t>(
                (color_as_int << Piece_Color_Shift) |
                    (piece_as_int & Piece_Type_Mask)
            );
            ColoredPiece piece_with_color = { .piece_type_and_color = result };
            return piece_with_color;
        }

        [[nodiscard]] constexpr auto color () const noexcept -> Color
        {
            auto result = gsl::narrow_cast<int8_t>(
                (piece_type_and_color & Piece_Color_Mask) >> Piece_Color_Shift
            );
            return colorFromInt8 (result);
        }

        [[nodiscard]] constexpr auto type () const noexcept -> Piece
        {
            auto result = gsl::narrow_cast<int8_t>(
                (piece_type_and_color & Piece_Type_Mask)
            );
            return pieceFromInt8 (result);
        }

        friend constexpr auto operator== (ColoredPiece first, ColoredPiece second) -> bool
        {
            return first.piece_type_and_color == second.piece_type_and_color;
        }

        friend constexpr bool operator!= (ColoredPiece first, ColoredPiece second)
        {
            return !operator== (first, second);
        }
    };
    static_assert(std::is_trivial_v<ColoredPiece>);

    // Order here is significant - it means computer will prefer the piece at the top
    // all else being equal, such as if the promoted piece cannot be saved from capture.
    static constexpr Piece All_Promotable_Piece_Types[] = {
            Piece::Queen,
            Piece::Rook,
            Piece::Bishop,
            Piece::Knight,
    };

    constexpr auto pieceType (ColoredPiece piece) -> Piece
    {
        return piece.type ();
    }

    constexpr auto pieceColor (ColoredPiece piece) -> Color
    {
        return piece.color ();
    }

    static constexpr ColoredPiece Piece_And_Color_None = ColoredPiece::make (
        Color::None,
        Piece::None
    );

    constexpr auto toInt8 (ColoredPiece piece) -> int8_t
    {
        return piece.piece_type_and_color;
    }

    constexpr auto pieceFromChar (char p) -> Piece
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

    constexpr auto pieceChar (ColoredPiece piece) -> char
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

    auto asString (Color who) -> string;
    auto asString (ColoredPiece piece) -> string;
    auto asString (Piece piece) -> string;

    auto operator<< (std::ostream& ostream, const ColoredPiece& value) -> std::ostream&;
}

#endif // WISDOM_CHESS_PIECE_HPP
