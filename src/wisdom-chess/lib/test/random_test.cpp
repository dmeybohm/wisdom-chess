#include "wisdom-chess/lib/random.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "CompileTimeRandom reports the full range of its result type" )
{
    static_assert (CompileTimeRandom::min() == 0);
    static_assert (CompileTimeRandom::max() == std::numeric_limits<uint32_t>::max());
    static_assert (CompileTimeRandom::min() < CompileTimeRandom::max());
}
