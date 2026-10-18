#include "wisdom-chess/lib/error.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "EXPECTS and ENSURES pass a true condition" )
{
    auto checked = []() noexcept
    {
        EXPECTS( 1 + 1 == 2 );
        ENSURES( 1 + 1 == 2 );
        return true;
    };
    CHECK( checked() );
}

TEST_CASE( "ASSERT" )
{
    SUBCASE( "A true condition passes" )
    {
        ASSERT( 1 + 1 == 2 );
    }

    SUBCASE( "Evaluates the condition only when Debugging is on" )
    {
        int evaluations = 0;
        ASSERT( ++evaluations > 0 );
        CHECK( evaluations == (Debugging ? 1 : 0) );
    }

    SUBCASE( "A true condition works in a constant expression" )
    {
        constexpr auto checked = []
        {
            ASSERT( 1 + 1 == 2 );
            return true;
        }();
        static_assert (checked);
    }
}
