#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/random.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "Lossless conversion checks" )
{
    SUBCASE( "Values inside the target range are lossless" )
    {
        static_assert (isLosslessConversion<int8_t> (127));
        static_assert (isLosslessConversion<int8_t> (-128));
        static_assert (isLosslessConversion<uint8_t> (255));
        static_assert (isLosslessConversion<char> ('a' + 7));
        static_assert (isLosslessConversion<std::size_t> (0));
    }

    SUBCASE( "Values outside the target range are lossy" )
    {
        static_assert (!isLosslessConversion<int8_t> (128));
        static_assert (!isLosslessConversion<int8_t> (-129));
        static_assert (!isLosslessConversion<uint8_t> (256));
    }

    SUBCASE( "Sign changes are lossy even when the bits round-trip" )
    {
        static_assert (!isLosslessConversion<std::size_t> (-1));
        static_assert (!isLosslessConversion<uint8_t> (int8_t { -1 }));
        static_assert (!isLosslessConversion<int> (std::numeric_limits<unsigned>::max()));
    }
}

TEST_CASE( "truncate discards the high bits" )
{
    static_assert (truncate<uint32_t> (uint64_t { 0x1234'5678'9abc'def0ULL }) == 0x9abc'def0U);
    static_assert (truncate<uint8_t> (uint32_t { 0x1ff }) == 0xff);
    static_assert (truncate<uint16_t> (uint16_t { 0xbeef }) == 0xbeef);

    uint64_t runtime_value = 0xffff'ffff'0000'0001ULL;
    CHECK( truncate<uint32_t> (runtime_value) == 1U );
}

TEST_CASE( "narrow throws at runtime when the value does not fit" )
{
    int too_big = 300;
    int negative = -1;

    CHECK( narrow<int8_t> (100) == 100 );
    CHECK_THROWS( (void)narrow<int8_t> (too_big) );
    CHECK_THROWS( (void)narrow<std::size_t> (negative) );
}

TEST_CASE( "CompileTimeRandom reports the full range of its result type" )
{
    static_assert (CompileTimeRandom::min() == 0);
    static_assert (CompileTimeRandom::max() == std::numeric_limits<uint32_t>::max());
    static_assert (CompileTimeRandom::min() < CompileTimeRandom::max());
}
