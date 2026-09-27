#include "wisdom-chess/engine/str.hpp"

#include "wisdom-chess-tests.hpp"

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

    SUBCASE( "Empty" )
    {
        auto result = wisdom::toInt ("");
        REQUIRE( !result.has_value() );
    }

    SUBCASE( "Too large or too small to fit in an int" )
    {
        CHECK( !wisdom::toInt ("99999999999999999999").has_value() );
        CHECK( !wisdom::toInt ("-99999999999999999999").has_value() );
        CHECK( !wisdom::toInt ("2147483648").has_value() );
        CHECK( wisdom::toInt ("2147483647") == std::numeric_limits<int>::max() );
    }
}

using wisdom::toLower;
using wisdom::toUpper;
using wisdom::isAlpha;
using wisdom::isDigit;
using wisdom::isLower;
using wisdom::isSpace;

TEST_CASE( "Character classification" )
{
    SUBCASE( "ASCII letters, digits and whitespace" )
    {
        CHECK( toLower ('A') == 'a' );
        CHECK( toLower ('z') == 'z' );
        CHECK( toUpper ('q') == 'Q' );
        CHECK( toUpper ('7') == '7' );
        CHECK( isAlpha ('k') );
        CHECK( isAlpha ('K') );
        CHECK( !isAlpha ('1') );
        CHECK( isDigit ('0') );
        CHECK( !isDigit ('a') );
        CHECK( isLower ('a') );
        CHECK( !isLower ('A') );
        CHECK( isSpace (' ') );
        CHECK( isSpace ('\t') );
        CHECK( isSpace ('\n') );
        CHECK( !isSpace ('_') );
    }

    SUBCASE( "Bytes above 0x7f are neither letters, digits nor spaces" )
    {
        for (int byte = 0x80; byte <= 0xff; byte++)
        {
            char ch = static_cast<char> (byte);
            INFO( "byte ", byte );
            CHECK( toLower (ch) == ch );
            CHECK( toUpper (ch) == ch );
            CHECK( !isAlpha (ch) );
            CHECK( !isDigit (ch) );
            CHECK( !isLower (ch) );
            CHECK( !isSpace (ch) );
        }
    }

    SUBCASE( "chomp only strips ASCII whitespace" )
    {
        CHECK( wisdom::chomp ("abc\xa0") == "abc\xa0" );
        CHECK( wisdom::chomp ("abc \n") == "abc" );
    }
}
