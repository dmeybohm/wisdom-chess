#include "piece.hpp"

#include <iostream>

namespace wisdom
{
    std::string to_string (Color who)
    {
        switch (who)
        {
            case Color::White: return "White";
            case Color::Black: return "Black";
            case Color::None: return "None";
        }
    }

    std::string to_string (Piece piece)
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
    }

    std::string to_string (ColoredPiece piece)
    {
        std::string result;

        result += to_string (piece_color (piece));
        result += to_string (piece_type (piece));

        return result;
    }

    std::ostream &operator<< (std::ostream &os, const ColoredPiece &piece)
    {
        os << to_string (piece);
        return os;
    }

}