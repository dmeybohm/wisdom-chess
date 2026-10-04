#pragma once

#include <tl/expected.hpp>

#include "wisdom-chess/engine/types.hpp"

namespace wisdom
{
    // tl::expected stands in for C++23's std::expected, which has the same
    // interface, until every toolchain the project builds with has it.
    using tl::expected;
    using tl::unexpected;

    // Why some text could not be parsed, in words for the user.
    struct ParseError
    {
        string message;
    };
}
