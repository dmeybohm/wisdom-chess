#include <sstream>

#include "wisdom-chess/engine/castling.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "CastlingEligibility - Default construction" )
{
    CastlingEligibility eligibility {};

    SUBCASE( "Default is eligible for neither side" )
    {
        CHECK( !eligibility.isSet (CastlingRights::Kingside) );
        CHECK( !eligibility.isSet (CastlingRights::Queenside) );
        CHECK_FALSE( to_bool (eligibility) );
    }

    SUBCASE( "to_uint returns 0 for default" )
    {
        CHECK( to_uint<uint8_t> (eligibility) == 0 );
    }
}

TEST_CASE( "CastlingEligibility - Construction from flags" )
{
    SUBCASE( "Kingside eligible" )
    {
        CastlingEligibility eligibility{ 1 };
        CHECK( eligibility.isSet (CastlingRights::Kingside) );
        CHECK( !eligibility.isSet (CastlingRights::Queenside) );
        CHECK( to_bool (eligibility) );
        CHECK( to_uint<uint8_t> (eligibility) == 1 );
    }

    SUBCASE( "Queenside eligible" )
    {
        CastlingEligibility eligibility{ 2 };
        CHECK( !eligibility.isSet (CastlingRights::Kingside) );
        CHECK( eligibility.isSet (CastlingRights::Queenside) );
        CHECK( to_bool (eligibility) );
        CHECK( to_uint<uint8_t> (eligibility) == 2 );
    }

    SUBCASE( "Both sides eligible" )
    {
        CastlingEligibility eligibility { 3 };
        CHECK( eligibility.isSet (CastlingRights::Kingside) );
        CHECK( eligibility.isSet (CastlingRights::Queenside) );
        CHECK( to_bool (eligibility) );
        CHECK( to_uint<uint8_t> (eligibility) == 3 );
    }
}

TEST_CASE( "CastlingEligibility - makeCastlingEligibilityFromInt" )
{
    SUBCASE( "From various integer values" )
    {
        auto zero = makeCastlingEligibilityFromInt (0);
        CHECK( to_uint<uint8_t> (zero) == 0 );

        auto one = makeCastlingEligibilityFromInt (1);
        CHECK( to_uint<uint8_t> (one) == 1 );
        CHECK( one.isSet (CastlingRights::Kingside) );

        auto two = makeCastlingEligibilityFromInt (2);
        CHECK( to_uint<uint8_t> (two) == 2 );
        CHECK( two.isSet (CastlingRights::Queenside) );

        auto three = makeCastlingEligibilityFromInt (3);
        CHECK( to_uint<uint8_t> (three) == 3 );
        CHECK( three.isSet (CastlingRights::Kingside) );
        CHECK( three.isSet (CastlingRights::Queenside) );
    }
}

TEST_CASE( "CastlingEligibility - set and clear operations" )
{
    CastlingEligibility eligibility {};

    SUBCASE( "Set kingside eligible" )
    {
        eligibility.set (CastlingRights::Kingside);
        CHECK( eligibility.isSet (CastlingRights::Kingside) );
        CHECK( !eligibility.isSet (CastlingRights::Queenside) );
        CHECK( to_uint<uint8_t> (eligibility) == 1 );
    }

    SUBCASE( "Set queenside eligible" )
    {
        eligibility.set (CastlingRights::Queenside);
        CHECK( !eligibility.isSet (CastlingRights::Kingside) );
        CHECK( eligibility.isSet (CastlingRights::Queenside) );
        CHECK( to_uint<uint8_t> (eligibility) == 2 );
    }

    SUBCASE( "Set both sides eligible" )
    {
        eligibility.set (CastlingRights::Kingside);
        eligibility.set (CastlingRights::Queenside);
        CHECK( eligibility.isSet (CastlingRights::Kingside) );
        CHECK( eligibility.isSet (CastlingRights::Queenside) );
        CHECK( to_uint<uint8_t> (eligibility) == 3 );
    }

    SUBCASE( "Clear operations" )
    {
        eligibility.set (CastlingRights::Kingside | CastlingRights::Queenside);
        CHECK( to_uint<uint8_t> (eligibility) == 3 );

        eligibility.clear (CastlingRights::Kingside);
        CHECK( !eligibility.isSet (CastlingRights::Kingside) );
        CHECK( eligibility.isSet (CastlingRights::Queenside) );
        CHECK( to_uint<uint8_t> (eligibility) == 2 );

        eligibility.clear (CastlingRights::Queenside);
        CHECK( !eligibility.isSet (CastlingRights::Kingside) );
        CHECK( !eligibility.isSet (CastlingRights::Queenside) );
        CHECK( to_uint<uint8_t> (eligibility) == 0 );
    }
}

TEST_CASE( "CastlingEligibility - bitwise operators" )
{
    auto kingside = CastlingRights::Kingside;
    auto queenside = CastlingRights::Queenside;

    SUBCASE( "OR operator" )
    {
        auto both = kingside | queenside;
        CHECK( both.isSet (CastlingRights::Kingside) );
        CHECK( both.isSet (CastlingRights::Queenside) );
        CHECK( to_uint<uint8_t> (both) == 3 );
    }

    SUBCASE( "AND operator" )
    {
        auto both = kingside | queenside;
        auto result_king = both & kingside;
        auto result_queen = both & queenside;

        CHECK( to_uint<uint8_t> (result_king) == 1 );
        CHECK( to_uint<uint8_t> (result_queen) == 2 );
    }

    SUBCASE( "XOR operator" )
    {
        auto both = kingside | queenside;
        auto result = both ^ kingside;

        CHECK( !result.isSet (CastlingRights::Kingside) );
        CHECK( result.isSet (CastlingRights::Queenside) );
        CHECK( to_uint<uint8_t> (result) == 2 );
    }
}

TEST_CASE( "CastlingEligibility - assignment operators" )
{
    CastlingEligibility eligibility {};

    SUBCASE( "OR assignment" )
    {
        eligibility |= CastlingRights::Kingside;
        CHECK( to_uint<uint8_t> (eligibility) == 1 );

        eligibility |= CastlingRights::Queenside;
        CHECK( to_uint<uint8_t> (eligibility) == 3 );
    }

    SUBCASE( "AND assignment" )
    {
        eligibility = CastlingRights::Kingside | CastlingRights::Queenside;
        eligibility &= CastlingRights::Kingside;
        CHECK( to_uint<uint8_t> (eligibility) == 1 );
    }

    SUBCASE( "XOR assignment" )
    {
        eligibility = CastlingRights::Kingside | CastlingRights::Queenside;
        eligibility ^= CastlingRights::Kingside;
        CHECK( to_uint<uint8_t> (eligibility) == 2 );
    }

    SUBCASE( "Regular assignment" )
    {
        eligibility = CastlingRights::Queenside;
        CHECK( to_uint<uint8_t> (eligibility) == 2 );
    }
}

TEST_CASE( "CastlingEligibility - equality operators" )
{
    auto kingside = CastlingRights::Kingside;
    auto queenside = CastlingRights::Queenside;
    auto both = kingside | queenside;

    SUBCASE( "Equality" )
    {
        CHECK( kingside == CastlingRights::Kingside );
        CHECK( queenside == CastlingRights::Queenside );
        CHECK( both == (CastlingRights::Kingside | CastlingRights::Queenside) );
    }

    SUBCASE( "Inequality" )
    {
        CHECK( kingside != queenside );
        CHECK( kingside != both );
        CHECK( queenside != both );
    }
}

TEST_CASE( "CastlingEligibility - bool conversion" )
{
    SUBCASE( "Empty eligibility is false" )
    {
        CastlingEligibility empty {};
        CHECK_FALSE( to_bool (empty) );
    }

    SUBCASE( "Non-empty eligibility is true" )
    {
        CHECK( to_bool (CastlingRights::Kingside) );
        CHECK( to_bool (CastlingRights::Queenside) );
        CHECK( to_bool (CastlingRights::Kingside | CastlingRights::Queenside) );
    }
}

TEST_CASE( "CastlingRights - static constants" )
{
    SUBCASE( "Kingside constant" )
    {
        CHECK( to_uint<uint8_t> (CastlingRights::Kingside) == 1 );
        CHECK( CastlingRights::Kingside.isSet (CastlingRights::Kingside) );
        CHECK( !CastlingRights::Kingside.isSet (CastlingRights::Queenside) );
    }

    SUBCASE( "Queenside constant" )
    {
        CHECK( to_uint<uint8_t> (CastlingRights::Queenside) == 2 );
        CHECK( !CastlingRights::Queenside.isSet (CastlingRights::Kingside) );
        CHECK( CastlingRights::Queenside.isSet (CastlingRights::Queenside) );
    }
}

TEST_CASE( "Global constants" )
{
    SUBCASE( "CastlingEligibility::Both_Sides" )
    {
        CHECK( to_uint<uint8_t> (CastlingEligibility::Both_Sides) == 3 );
        CHECK( to_bool (CastlingEligibility::Both_Sides) );
    }

    SUBCASE( "CastlingEligibility::Neither_Side" )
    {
        CHECK( to_uint<uint8_t> (CastlingEligibility::Neither_Side) == 0 );
        CHECK_FALSE( to_bool (CastlingEligibility::Neither_Side) );
        CHECK( !CastlingEligibility::Neither_Side.isSet (CastlingRights::Kingside) );
        CHECK( !CastlingEligibility::Neither_Side.isSet (CastlingRights::Queenside) );
    }
}

TEST_CASE( "CastlingEligibility - Stream output" )
{
    auto as_streamed = [] (CastlingEligibility eligibility) -> std::string
    {
        std::ostringstream out;
        out << eligibility;
        return out.str();
    };

    SUBCASE( "Both sides eligible" )
    {
        CHECK( as_streamed (CastlingEligibility::Both_Sides)
               == "{ Kingside: eligible, Queenside: eligible }" );
    }

    SUBCASE( "Kingside only" )
    {
        CHECK( as_streamed (CastlingRights::Kingside)
               == "{ Kingside: eligible, Queenside: not eligible }" );
    }

    SUBCASE( "Queenside only" )
    {
        CHECK( as_streamed (CastlingRights::Queenside)
               == "{ Kingside: not eligible, Queenside: eligible }" );
    }

    SUBCASE( "Neither side" )
    {
        CHECK( as_streamed (CastlingEligibility::Neither_Side)
               == "{ Kingside: not eligible, Queenside: not eligible }" );
    }
}

namespace
{
    template <typename Target, typename Source>
    concept ConvertsToUint = requires (const Source& value) { to_uint<Target> (value); };
}

TEST_CASE( "to_uint on castling eligibility" )
{
    auto eligibility = CastlingRights::Kingside | CastlingRights::Queenside;

    SUBCASE( "Different unsigned integer types" )
    {
        CHECK( to_uint<uint8_t> (eligibility) == 3 );
        CHECK( to_uint<uint16_t> (eligibility) == 3 );
        CHECK( to_uint<uint32_t> (eligibility) == 3 );
        CHECK( to_uint<uint64_t> (eligibility) == 3 );
    }

    SUBCASE( "Signed types are rejected" )
    {
        CHECK_FALSE( ConvertsToUint<int, CastlingEligibility> );
        CHECK_FALSE( ConvertsToUint<signed char, CastlingEligibility> );
        CHECK_FALSE( std::is_constructible_v<int, CastlingEligibility> );
    }

    SUBCASE( "Type safety - ensure unsigned arithmetic" )
    {
        auto result = to_uint<uint8_t> (eligibility);
        static_assert (std::is_same_v<decltype(result), uint8_t>);

        // Verify bitwise operations work correctly with unsigned types
        uint8_t castle_bits = 3;  // Both sides eligible
        auto check_bits_signed = ~0;  // This is signed int
        auto check_bits_unsigned = static_cast<uint8_t>(~0);  // This is uint8_t

        // Demonstrate the potential issue
        CHECK( check_bits_signed == -1 );  // ~0 as signed int
        CHECK( check_bits_unsigned == 255 );  // ~0 as uint8_t

        // The important test: both should fail the equality check
        CHECK( (castle_bits & check_bits_signed) != check_bits_signed );
        CHECK( (castle_bits & check_bits_unsigned) != check_bits_unsigned );

        // Verify the actual implementation matches our understanding
        using flags_type = uint8_t;
        static_assert (std::is_unsigned_v<flags_type>);

        auto max_flags = std::numeric_limits<flags_type>::max();  // What ableToCastle uses
        CHECK( max_flags == 255 );
        CHECK( (castle_bits & max_flags) == castle_bits );  // 3 & 255 == 3
        CHECK( (castle_bits & max_flags) != max_flags );    // 3 != 255, so equality fails
    }
}
