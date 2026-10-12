#include "wisdom-chess/engine/coord.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/optional.hpp"
#include "wisdom-chess/engine/piece.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

namespace
{
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

    // Whether code outside the optional can make a type's empty value.
    template <typename T>
    concept EmptyValueReachable = requires {
        NullRepresentationOf<T>::make();
    } || requires {
        T::NullRepresentation::make (NullRepresentationKey {});
    };
}

static_assert (std::is_same_v<optional<Move>, compact_optional<Move>>);
static_assert (std::is_same_v<optional<Coord>, compact_optional<Coord>>);
static_assert (std::is_same_v<optional<ColoredPiece>, compact_optional<ColoredPiece>>);
static_assert (std::is_same_v<optional<Piece>, compact_optional<Piece>>);
static_assert (std::is_same_v<optional<Shade>, compact_optional<Shade>>);
static_assert (sizeof (optional<Shade>) == sizeof (Shade));
static_assert (std::is_same_v<optional<Color>, compact_optional<Color>>);
static_assert (std::is_same_v<optional<int>, std::optional<int>>);
static_assert (std::is_same_v<
    optional<WithoutNullRepresentation>,
    std::optional<WithoutNullRepresentation>
>);
static_assert (!HasNullRepresentation<WithPrivateNullRepresentation>);
static_assert (!std::is_default_constructible_v<NullRepresentationKey>);
static_assert (!EmptyValueReachable<Move>);
static_assert (!EmptyValueReachable<Coord>);
static_assert (!EmptyValueReachable<ColoredPiece>);
static_assert (!EmptyValueReachable<Piece>);
static_assert (!EmptyValueReachable<Color>);
static_assert (std::is_trivially_copyable_v<optional<Move>>);
static_assert (std::is_trivially_copyable_v<optional<Coord>>);

TEST_CASE( "A compact optional" )
{
    Move move = Move::make (1, 2, 3, 4);

    SUBCASE( "is empty by default" )
    {
        optional<Move> empty;
        CHECK( !empty.has_value() );
        CHECK( !empty );
        CHECK( empty == nullopt );
        CHECK( empty.value_or (move) == move );
    }

    SUBCASE( "holds a value" )
    {
        optional<Move> holder = move;
        REQUIRE( holder.has_value() );
        CHECK( *holder == move );
        CHECK( holder.value() == move );
        CHECK( holder->getSrc() == move.getSrc() );
        CHECK( holder == move );
        CHECK( holder != nullopt );
    }

    SUBCASE( "can be reset" )
    {
        optional<Move> holder = move;
        holder = nullopt;
        CHECK( !holder.has_value() );

        holder.emplace (move);
        CHECK( holder == move );

        holder.reset();
        CHECK( holder == optional<Move> {} );
    }

    SUBCASE( "compares values" )
    {
        optional<Move> first = move;
        optional<Move> second = Move::make (5, 6, 7, 0);
        optional<Move> empty;

        CHECK( first == optional<Move> { move } );
        CHECK( first != second );
        CHECK( first != empty );
        CHECK( empty == optional<Move> {} );
        CHECK( first != Move::make (5, 6, 7, 0) );
    }

    SUBCASE( "holds the first square" )
    {
        Coord a8 = Coord::make (0, 0);
        optional<Coord> holder = a8;
        REQUIRE( holder.has_value() );
        CHECK( *holder == a8 );
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
