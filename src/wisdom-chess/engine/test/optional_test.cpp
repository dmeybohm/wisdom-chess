#include "wisdom-chess/engine/coord.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/optional.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

namespace
{
    struct WithoutNullRepresentation
    {
        int8_t value;
    };

    class WithPrivateNullRepresentation
    {
        struct NullRepresentation
        {
            static constexpr auto
            make() noexcept
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
}

static_assert (std::is_same_v<optional<Move>, compact_optional<Move>>);
static_assert (std::is_same_v<optional<Coord>, compact_optional<Coord>>);
static_assert (std::is_same_v<optional<int>, std::optional<int>>);
static_assert (std::is_same_v<
    optional<WithoutNullRepresentation>,
    std::optional<WithoutNullRepresentation>
>);
static_assert (!HasNullRepresentation<WithPrivateNullRepresentation>);
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
}
