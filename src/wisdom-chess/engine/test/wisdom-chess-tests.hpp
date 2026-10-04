#pragma once

#include <doctest/doctest.h>

// Workaround some build problems on older versions of MacOS with doctest
#ifndef DEBUG
#include <iostream>
#endif

namespace wisdom::test
{
    // Emscripten builds have no exception catching, so a throw ends the
    // process there.
#ifdef __EMSCRIPTEN__
    constexpr bool Can_Catch_Exceptions = false;
#else
    constexpr bool Can_Catch_Exceptions = true;
#endif
}
