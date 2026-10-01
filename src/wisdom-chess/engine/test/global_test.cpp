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

TEST_CASE( "Copying an Error cannot throw" )
{
    static_assert (std::is_nothrow_copy_constructible_v<Error>);
    static_assert (std::is_nothrow_copy_assignable_v<Error>);
    static_assert (std::is_nothrow_copy_constructible_v<PreconditionError>);

    SUBCASE( "A copy keeps its text after the original is destroyed" )
    {
        auto original = std::make_unique<Error> ("the message", "the extra info");
        Error copy { *original };
        original.reset();

        CHECK( copy.message() == "the message" );
        CHECK( copy.extraInfo() == "the extra info" );
        CHECK( string { copy.what() } == "the message" );
    }

    SUBCASE( "A moved-from error keeps its text" )
    {
        Error original { "the message", "the extra info" };
        Error moved_to { std::move (original) };

        CHECK( moved_to.message() == "the message" );
        CHECK( original.message() == "the message" );
    }

    SUBCASE( "The extra info defaults to empty" )
    {
        Error error { "the message" };

        CHECK( error.extraInfo().empty() );
    }
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

    SUBCASE( "Constructing from null throws" )
    {
        int* null_ptr = nullptr; // lint-allow(raw-pointer)
        CHECK_THROWS_AS( nonnull<int> { null_ptr }, PreconditionError );
    }

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
        CHECK_THROWS_AS( (void)ptr.value(), PreconditionError );
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

TEST_CASE( "EXPECTS quotes the condition and the location in the error" )
{
    try
    {
        EXPECTS( 1 + 1 == 3 );
        FAIL( "EXPECTS did not throw" );
    }
    catch (const PreconditionError& error)
    {
        CHECK( error.message().find ("Precondition failed at ") == 0 );
        CHECK( error.message().find ("global_test.cpp:") != string::npos );
        CHECK( error.message().find (": 1 + 1 == 3") != string::npos );
        CHECK( !error.extraInfo().empty() );
    }
}

TEST_CASE( "ENSURES quotes the condition in the error" )
{
    try
    {
        ENSURES( 2 * 2 == 5 );
        FAIL( "ENSURES did not throw" );
    }
    catch (const PostconditionError& error)
    {
        CHECK( error.message().find ("Postcondition failed at ") == 0 );
        CHECK( error.message().find (": 2 * 2 == 5") != string::npos );
    }
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
