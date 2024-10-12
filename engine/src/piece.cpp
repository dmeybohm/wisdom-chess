#include <iostream>

#include "wisdom-chess/engine/piece.hpp"

namespace wisdom
{
    auto asString (Color who) -> string
    {
        switch (who)
        {
            case Color::White:
                return "White";
            case Color::Black:
                return "Black";
            case Color::None:
                return "None";
        }
        std::terminate();
    }

    auto asString (Piece piece) -> string
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
        std::terminate();
    }

    auto asString (ColoredPiece piece) -> string
    {
        string result;

        result += asString (pieceColor (piece));
        result += asString (pieceType (piece));

        return result;
    }

    auto 
    operator<< (std::ostream& ostream, const ColoredPiece& piece) 
        -> std::ostream&
    {
        ostream << asString (piece);
        return ostream;
    }
}
