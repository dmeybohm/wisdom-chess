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

TEST_CASE( "narrow converts a value that fits" )
{
    static_assert (narrow<int8_t> (100) == 100);

    int fits = 127;
    std::size_t zero = 0;
    CHECK( narrow<int8_t> (fits) == 127 );
    CHECK( narrow<int> (zero) == 0 );
}

TEST_CASE( "narrow_debug converts a value that fits" )
{
    static_assert (narrow_debug<int8_t> (100) == 100);

    if constexpr (!Debugging)
    {
        int too_big = 300;
        CHECK( narrow_debug<uint8_t> (too_big) == 44 );
    }
}

TEST_CASE( "widen converts to a type that holds every value" )
{
    static_assert (widen<int64_t> (int32_t { -1 }) == -1);
    static_assert (widen<int64_t> (uint32_t { 0xffff'ffffU }) == 0xffff'ffffLL);
    static_assert (widen<uint64_t> (uint32_t { 0xffff'ffffU }) == 0xffff'ffffULL);
}

TEST_CASE( "to_underlying gives the enum's underlying type" )
{
    enum class Small : int8_t
    {
        Value = -3
    };
    enum class Default
    {
        Value = 7
    };

    static_assert (std::is_same_v<decltype (to_underlying (Small::Value)), int8_t>);
    static_assert (std::is_same_v<decltype (to_underlying (Default::Value)), int>);
    static_assert (to_underlying (Small::Value) == -3);
    static_assert (to_underlying (Default::Value) == 7);
}

TEST_CASE( "to_enum converts a value that fits the underlying type" )
{
    enum class Small : int8_t
    {
        Value = -3
    };

    static_assert (to_enum<Small> (-3) == Small::Value);
    static_assert (to_enum_debug<Small> (int64_t { -3 }) == Small::Value);

    int runtime_value = -3;
    CHECK( to_enum<Small> (runtime_value) == Small::Value );
    CHECK( to_enum_debug<Small> (runtime_value) == Small::Value );
}

TEST_CASE( "to_double converts a number" )
{
    static_assert (to_double (3) == 3.0);
    static_assert (to_double (uint64_t { 1 } << 53) == 9007199254740992.0);
    static_assert (to_double (1.5f) == 1.5);
}

TEST_CASE( "to_bool converts through an explicit operator bool" )
{
    struct Flag
    {
        bool set;

        constexpr explicit operator bool() const noexcept
        {
            return set;
        }
    };

    static_assert (to_bool (Flag { true }));
    static_assert (!to_bool (Flag { false }));
    static_assert (to_bool (2));
    static_assert (!to_bool (0));
}

TEST_CASE( "to_unsigned converts a nonnegative value" )
{
    static_assert (to_unsigned<uint32_t> (int32_t { 0 }) == 0);
    static_assert (to_unsigned<uint32_t> (std::numeric_limits<int32_t>::max()) == 0x7fff'ffffU);
    static_assert (to_unsigned<std::size_t> (int8_t { 1 }) == 1);

    int ply = 63;
    CHECK( to_unsigned<std::size_t> (ply) == 63 );
}

TEST_CASE( "to_unsigned_debug converts a nonnegative value" )
{
    static_assert (to_unsigned_debug<uint32_t> (int32_t { 42 }) == 42);

    if constexpr (!Debugging)
    {
        int32_t negative = -1;
        CHECK( to_unsigned_debug<uint32_t> (negative) == 0xffff'ffffU );
    }
}

TEST_CASE( "CompileTimeRandom reports the full range of its result type" )
{
    static_assert (CompileTimeRandom::min() == 0);
    static_assert (CompileTimeRandom::max() == std::numeric_limits<uint32_t>::max());
    static_assert (CompileTimeRandom::min() < CompileTimeRandom::max());
}

namespace
{
    constexpr auto
    readThroughNonnull()
        -> int
    {
        int value = 42;
        nonnull<int> ptr = &value;
        return *ptr;
    }

    template <typename P>
    concept Dereferenceable = requires (P p) { *p; } || requires (P p) { p.operator->(); };

    struct Base
    {
    };

    struct Derived : Base
    {
    };
}

TEST_CASE( "nonnull" )
{
    static_assert (Dereferenceable<nonnull<int>>);
    static_assert (!std::is_default_constructible_v<nonnull<int>>);
    static_assert (!std::is_constructible_v<nonnull<int>, std::nullptr_t>);
    static_assert (!std::is_assignable_v<nonnull<int>&, std::nullptr_t>);
    static_assert (!std::is_constructible_v<nonnull<int>, nullable<int>>);
    static_assert (!std::is_convertible_v<nonnull<int>, int*>); // lint-allow(raw-pointer)
    static_assert (!std::is_convertible_v<nonnull<int>, bool>);
    static_assert (std::is_trivially_copyable_v<nonnull<int>>);
    static_assert (sizeof (nonnull<int>) == sizeof (int*)); // lint-allow(raw-pointer)

    SUBCASE( "Dereferencing reaches the pointed-to object" )
    {
        std::string text = "abc";
        nonnull<std::string> ptr = &text;

        CHECK( ptr.get() == &text );
        CHECK( ptr->size() == 3 );
        *ptr += "d";
        CHECK( text == "abcd" );
    }

    SUBCASE( "A moved-from pointer keeps its value" )
    {
        int value = 7;
        nonnull<int> original = &value;
        nonnull<int> moved_to = std::move (original);

        CHECK( moved_to.get() == &value );
        CHECK( original.get() == &value );
    }

    SUBCASE( "Converts to a pointer to a base class or to const" )
    {
        Derived derived;
        nonnull<Derived> derived_ptr = &derived;
        nonnull<Base> base_ptr = derived_ptr;
        nonnull<const Derived> const_ptr = derived_ptr;

        CHECK( base_ptr.get() == &derived );
        CHECK( const_ptr.get() == &derived );
        static_assert (!std::is_constructible_v<nonnull<Derived>, nonnull<Base>>);
        static_assert (!std::is_constructible_v<nonnull<int>, nonnull<const int>>);
    }

    SUBCASE( "Compares by address" )
    {
        int first = 1;
        int second = 1;
        nonnull<int> first_ptr = &first;

        CHECK( first_ptr == nonnull<int> { &first } );
        CHECK( first_ptr != nonnull<int> { &second } );
    }

    SUBCASE( "Works in a constant expression" )
    {
        static_assert (readThroughNonnull() == 42);
    }
}

TEST_CASE( "nullable" )
{
    static_assert (!Dereferenceable<nullable<int>>);
    static_assert (!std::is_convertible_v<nullable<int>, int*>); // lint-allow(raw-pointer)
    static_assert (!std::is_convertible_v<nullable<int>, bool>);
    static_assert (std::is_trivially_copyable_v<nullable<int>>);
    static_assert (sizeof (nullable<int>) == sizeof (int*)); // lint-allow(raw-pointer)

    SUBCASE( "A default-constructed pointer is null" )
    {
        nullable<int> ptr;

        CHECK( !ptr );
        CHECK( ptr == nullptr );
    }

    SUBCASE( "value() returns the pointer as nonnull" )
    {
        int value = 3;
        nullable<int> ptr = &value;

        REQUIRE( ptr );
        nonnull<int> checked = ptr.value();
        CHECK( checked.get() == &value );
        CHECK( ptr.unsafeGet() == &value );
    }

    SUBCASE( "Converts from nonnull" )
    {
        int value = 3;
        nonnull<int> checked = &value;
        nullable<int> ptr = checked;

        CHECK( ptr.unsafeGet() == &value );
    }

    SUBCASE( "Converts to a pointer to a base class or to const" )
    {
        Derived derived;
        nullable<Derived> derived_ptr = &derived;
        nullable<Base> base_ptr = derived_ptr;
        nullable<const Derived> const_ptr = derived_ptr;

        CHECK( base_ptr.unsafeGet() == &derived );
        CHECK( const_ptr.unsafeGet() == &derived );
        static_assert (!std::is_constructible_v<nullable<Derived>, nullable<Base>>);
    }

    SUBCASE( "Compares by address" )
    {
        int first = 1;
        int second = 2;
        nullable<int> first_ptr = &first;

        CHECK( first_ptr == nullable<int> { &first } );
        CHECK( first_ptr != nullable<int> { &second } );
        CHECK( first_ptr != nullptr );
    }
}

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
            ASSERT( Num_Rows == 8 );
            return true;
        }();
        static_assert (checked);
    }
}
