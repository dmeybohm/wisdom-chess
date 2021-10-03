#include "piece.hpp"

#include <iostream>

namespace wisdom
{
    auto to_string (Color who) -> string
    {
        switch (who)
        {
            case Color::White: return "White";
            case Color::Black: return "Black";
            case Color::None: return "None";
        }
        std::terminate ();
    }

    auto to_string (Piece piece) -> string
    {
        switch (piece)
        {
            case Piece::King:
                return "King";
            case Piece::Queen:
                return "Queen";
            case Piece::Rook:
                return "Rook";
            case Piece::Bishop:
                return "Bishop";
            case Piece::Knight:
                return "Knight";
            case Piece::Pawn:
                return "Pawn";
            case Piece::None:
                return "None";
        }
        std::terminate ();
    }

    auto to_string (const ColoredPiece piece) -> string
    {
        string result;

        result += to_string (piece_color (piece));
        result += to_string (piece_type (piece));

        return result;
    }

    auto operator<< (std::ostream &os, const ColoredPiece &piece) -> std::ostream&
    {
        os << to_string (piece);
        return os;
    }
}
