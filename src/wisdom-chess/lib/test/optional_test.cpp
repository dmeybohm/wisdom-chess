#include "wisdom-chess/lib/optional.hpp"
#include "wisdom-chess/lib/ptr.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

namespace
{
    // A class opting in through a nested NullRepresentation. Zero is the
    // value its API never produces.
    class Tag
    {
    public:
        struct NullRepresentation
        {
            [[nodiscard]] static constexpr auto
            make (NullRepresentationKey) noexcept
                -> Tag
            {
                return Tag { 0 };
            }

            [[nodiscard]] static constexpr auto
            isNull (const Tag& tag) noexcept
                -> bool
            {
                return tag.my_value == 0;
            }
        };

        [[nodiscard]] static constexpr auto
        make (int value) noexcept
            -> Tag
        {
            EXPECTS( value != 0 );
            return Tag { value };
        }

        [[nodiscard]] constexpr auto
        value() const noexcept
            -> int
        {
            return my_value;
        }

        [[nodiscard]] friend constexpr auto
        operator== (const Tag&, const Tag&) noexcept
            -> bool = default;

    private:
        constexpr explicit Tag (int value) noexcept
            : my_value { value }
        {
        }

        int my_value;
    };

    // An enum, which cannot nest a NullRepresentation, opting in through
    // the function instead.
    enum class Shade : int8_t
    {
        Light = 1,
        Dark = 2,
    };

    [[nodiscard]] constexpr auto
    nullRepresentation (std::type_identity<Shade>, NullRepresentationKey) noexcept
        -> Shade
    {
        return Shade {};
    }

    struct WithoutNullRepresentation
    {
        int8_t value;
    };

    class WithPrivateNullRepresentation
    {
        struct NullRepresentation
        {
            static constexpr auto
            make (NullRepresentationKey) noexcept
                -> WithPrivateNullRepresentation
            {
                return {};
            }

            static constexpr auto
            isNull (const WithPrivateNullRepresentation&) noexcept
                -> bool
            {
                return true;
            }
        };
    };

    // Whether code outside the optional can reach a type's empty value
    // through the optional's access to it. A type's own make() needs a
    // NullRepresentationKey, which cannot be created here either.
    template <typename T>
    concept EmptyValueReachable = requires { NullRepresentationOf<T>::make(); };
}

static_assert (std::is_same_v<optional<Tag>, compact_optional<Tag>>);
static_assert (sizeof (optional<Tag>) == sizeof (Tag));
static_assert (std::is_same_v<optional<Shade>, compact_optional<Shade>>);
static_assert (sizeof (optional<Shade>) == sizeof (Shade));
static_assert (std::is_same_v<optional<int>, std::optional<int>>);
static_assert (std::is_same_v<optional<nonnull<int>>, compact_optional<nonnull<int>>>);
static_assert (sizeof (optional<nonnull<int>>) == sizeof (nonnull<int>));
static_assert (std::is_same_v<
    optional<WithoutNullRepresentation>,
    std::optional<WithoutNullRepresentation>
>);
static_assert (!HasNullRepresentation<WithPrivateNullRepresentation>);
static_assert (!std::is_default_constructible_v<NullRepresentationKey>);
static_assert (!EmptyValueReachable<Tag>);
static_assert (!EmptyValueReachable<Shade>);
static_assert (std::is_trivially_copyable_v<optional<Tag>>);

TEST_CASE( "A compact optional" )
{
    Tag tag = Tag::make (7);

    SUBCASE( "is empty by default" )
    {
        optional<Tag> empty;
        CHECK( !empty.has_value() );
        CHECK( !empty );
        CHECK( empty == nullopt );
        CHECK( empty.value_or (tag) == tag );
    }

    SUBCASE( "holds a value" )
    {
        optional<Tag> holder = tag;
        REQUIRE( holder.has_value() );
        CHECK( *holder == tag );
        CHECK( holder.value() == tag );
        CHECK( holder->value() == 7 );
        CHECK( holder == tag );
        CHECK( holder != nullopt );
    }

    SUBCASE( "can be reset" )
    {
        optional<Tag> holder = tag;
        holder = nullopt;
        CHECK( !holder.has_value() );

        holder.emplace (tag);
        CHECK( holder == tag );

        holder.reset();
        CHECK( holder == optional<Tag> {} );
    }

    SUBCASE( "compares values" )
    {
        optional<Tag> first = tag;
        optional<Tag> second = Tag::make (8);
        optional<Tag> empty;

        CHECK( first == optional<Tag> { tag } );
        CHECK( first != second );
        CHECK( first != empty );
        CHECK( empty == optional<Tag> {} );
        CHECK( first != Tag::make (8) );
    }

    SUBCASE( "of a nonnull holds a pointer" )
    {
        int value = 0;
        optional<nonnull<int>> holder;
        CHECK( !holder.has_value() );

        holder = nonnull<int> { &value };
        REQUIRE( holder.has_value() );
        CHECK( holder->get() == &value );
    }

    SUBCASE( "of an enum uses its nullRepresentation() function" )
    {
        optional<Shade> empty;
        CHECK( !empty.has_value() );

        optional<Shade> dark = Shade::Dark;
        REQUIRE( dark.has_value() );
        CHECK( *dark == Shade::Dark );
    }
}
