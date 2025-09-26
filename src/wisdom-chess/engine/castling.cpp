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

        result += !value.isSet (CastlingIneligible::Kingside)
            ? "eligible, "
            : "not eligible, ";
        result += "Queenside: ";
        result += !value.isSet (CastlingIneligible::Queenside)
            ? "eligible"
            : "not eligible";
        result += " }";

        os << result;
        return os;
    }
}