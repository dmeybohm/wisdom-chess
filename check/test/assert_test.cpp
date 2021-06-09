#include <doctest/doctest.h>
#include "global.hpp"

using namespace wisdom;

TEST_CASE("Assertion failure throws an error")
{
    bool thrown = false;
    try
    {
        assert (0);
    }
    catch (AssertionError &e)
    {
        thrown = true;
        REQUIRE( e.file().find("assert_test.cpp") != std::string::npos );
    }
    REQUIRE( thrown );
}
