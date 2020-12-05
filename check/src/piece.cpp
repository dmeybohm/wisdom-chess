#include "piece.h"

std::string to_string (Color who)
{
    switch (who)
    {
        case Color::White:
            return "White";
        case Color::Black:
            return "Black";
        case Color::None:
            return "None";
        default:
            abort();
    }
}