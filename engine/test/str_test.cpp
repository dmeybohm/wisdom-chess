#include "tests.hpp"
#include "str.hpp"

using std::string;
using std::vector;

TEST_CASE( "chomp" )
{
    SUBCASE( "With text and carriage return and linefeed" )
    {
        string s { "Hello\r\n" };
        string result = wisdom::chomp (s);

        REQUIRE( result == "Hello" );
    }

    SUBCASE( "with newline" )
    {
        string s { "\n" };
        string result = wisdom::chomp (s);

        REQUIRE( result == "" );
    }

    SUBCASE( "with carriage return and newline" )
    {
        string s { "\r\n" };
        string result = wisdom::chomp (s);

        REQUIRE( result == "" );
    }

    SUBCASE( "with space at the end" )
    {
        string s { "Hello   \r\n" };
        string result = wisdom::chomp (s);

        REQUIRE( result == "Hello" );
    }
}

TEST_CASE( "split" )
{
    SUBCASE( "Empty" )
    {
        auto result = wisdom::split ("", ",");
        REQUIRE( result.size() == 1 );
    }

    SUBCASE( "Three strings" )
    {
        string input = "A string, to split, or not.";
        auto result = wisdom::split (input, ",");

        REQUIRE( result.size() == 3 );
        REQUIRE( result[0] == "A string" );
        REQUIRE( result[1] == " to split" );
        REQUIRE( result[2] == " or not." );
    }

    SUBCASE( "No separators" )
    {
        string input = "A string";
        auto result = wisdom::split (input, ",");

        REQUIRE( result.size() == 1 );
        REQUIRE( result[0] == "A string" );
    }

    SUBCASE( "Empty" )
    {
        string input = "";
        auto result = wisdom::split (input, ",");

        REQUIRE( result.size() == 1 );
        REQUIRE( result[0] == "" );
    }
}

TEST_CASE( "join" )
{
    SUBCASE( "Empty" )
    {
        auto result = wisdom::join ({}, ",");
        REQUIRE( result == "" );
    }

    SUBCASE( "Non-empty" )
    {
        auto result = wisdom::join ({"one", "two"}, ", ");
        REQUIRE( result == "one, two" );
    }
}

TEST_CASE( "toInt" )
{
    SUBCASE( "When successful" )
    {
        auto result = wisdom::toInt ("10");
        REQUIRE( result.has_value() );
        REQUIRE( *result == 10 );
    }

    SUBCASE( "Invalid" )
    {
        auto result = wisdom::toInt ("invalid");
        REQUIRE( !result.has_value() );
    }
}