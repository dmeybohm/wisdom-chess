#pragma once

#include <doctest/doctest.h>

// Workaround some build problems on older versions of MacOS with doctest
#ifndef DEBUG
#include <iostream>
#endif
