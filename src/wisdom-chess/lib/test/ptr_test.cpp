#include "wisdom-chess/lib/ptr.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

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
