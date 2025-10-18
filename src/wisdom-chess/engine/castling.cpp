#include <iostream>
#include <string>

#include "wisdom-chess/engine/castling.hpp"

namespace wisdom
{
    auto
    operator<< (std::ostream& os, const CastlingEligibility& value)
        -> std::ostream&
    {
        std::string result = "{ Kingside: ";

        result += value.isSet (CastlingRights::Kingside)
            ? "eligible, "
            : "not eligible, ";
        result += "Queenside: ";
        result += value.isSet (CastlingRights::Queenside);

        os << result;
        return os;
    }
}
