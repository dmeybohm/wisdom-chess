#include "wisdom-chess/engine/coord.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/piece.hpp"
#include "wisdom-chess/lib/optional.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

namespace
{
    // Whether code outside the optional can reach a type's empty value
    // through the optional's access to it. A type's own make() needs a
    // NullRepresentationKey, which cannot be created here either.
    template <typename T>
    concept EmptyValueReachable = requires { NullRepresentationOf<T>::make(); };
}

static_assert (std::is_same_v<optional<Move>, compact_optional<Move>>);
static_assert (std::is_same_v<optional<Coord>, compact_optional<Coord>>);
static_assert (std::is_same_v<optional<ColoredPiece>, compact_optional<ColoredPiece>>);
static_assert (std::is_same_v<optional<Piece>, compact_optional<Piece>>);
static_assert (std::is_same_v<optional<Color>, compact_optional<Color>>);
static_assert (!EmptyValueReachable<Move>);
static_assert (!EmptyValueReachable<Coord>);
static_assert (!EmptyValueReachable<ColoredPiece>);
static_assert (!EmptyValueReachable<Piece>);
static_assert (!EmptyValueReachable<Color>);
static_assert (std::is_trivially_copyable_v<optional<Move>>);
static_assert (std::is_trivially_copyable_v<optional<Coord>>);

TEST_CASE( "A compact optional of a chess type" )
{
    SUBCASE( "holds a move" )
    {
        Move move = Move::make (1, 2, 3, 4);
        optional<Move> holder = move;
        REQUIRE( holder.has_value() );
        CHECK( *holder == move );
        CHECK( holder->getSrc() == move.getSrc() );

        holder = nullopt;
        CHECK( !holder.has_value() );
    }

    SUBCASE( "holds the first square" )
    {
        Coord a8 = Coord::make (0, 0);
        optional<Coord> holder = a8;
        REQUIRE( holder.has_value() );
        CHECK( *holder == a8 );
    }
}
