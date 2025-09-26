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
        CHECK( !static_cast<bool> (eligibility) );
    }
    
    SUBCASE( "toInt returns 0 for default" )
    {
        CHECK( eligibility.toInt<uint8_t>() == 0 );
        CHECK( toInt<uint8_t> (eligibility) == 0 );
    }
}

TEST_CASE( "CastlingEligibility - Construction from flags" )
{
    SUBCASE( "Kingside eligible" )
    {
        CastlingEligibility eligibility{ 1 };
        CHECK( eligibility.isSet (CastlingRights::Kingside) );
        CHECK( !eligibility.isSet (CastlingRights::Queenside) );
        CHECK( static_cast<bool> (eligibility) );
        CHECK( eligibility.toInt<uint8_t>() == 1 );
    }
    
    SUBCASE( "Queenside eligible" )
    {
        CastlingEligibility eligibility{ 2 };
        CHECK( !eligibility.isSet (CastlingRights::Kingside) );
        CHECK( eligibility.isSet (CastlingRights::Queenside) );
        CHECK( static_cast<bool> (eligibility) );
        CHECK( eligibility.toInt<uint8_t>() == 2 );
    }
    
    SUBCASE( "Both sides eligible" )
    {
        CastlingEligibility eligibility { 3 };
        CHECK( eligibility.isSet (CastlingRights::Kingside) );
        CHECK( eligibility.isSet (CastlingRights::Queenside) );
        CHECK( static_cast<bool> (eligibility) );
        CHECK( eligibility.toInt<uint8_t>() == 3 );
    }
}

TEST_CASE( "CastlingEligibility - makeCastlingEligibilityFromInt" )
{
    SUBCASE( "From various integer values" )
    {
        auto zero = makeCastlingEligibilityFromInt (0);
        CHECK( zero.toInt<uint8_t>() == 0 );
        
        auto one = makeCastlingEligibilityFromInt (1);
        CHECK( one.toInt<uint8_t>() == 1 );
        CHECK( one.isSet (CastlingRights::Kingside) );
        
        auto two = makeCastlingEligibilityFromInt (2);
        CHECK( two.toInt<uint8_t>() == 2 );
        CHECK( two.isSet (CastlingRights::Queenside) );
        
        auto three = makeCastlingEligibilityFromInt (3);
        CHECK( three.toInt<uint8_t>() == 3 );
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
        CHECK( eligibility.toInt<uint8_t>() == 1 );
    }
    
    SUBCASE( "Set queenside eligible" )
    {
        eligibility.set (CastlingRights::Queenside);
        CHECK( !eligibility.isSet (CastlingRights::Kingside) );
        CHECK( eligibility.isSet (CastlingRights::Queenside) );
        CHECK( eligibility.toInt<uint8_t>() == 2 );
    }
    
    SUBCASE( "Set both sides eligible" )
    {
        eligibility.set (CastlingRights::Kingside);
        eligibility.set (CastlingRights::Queenside);
        CHECK( eligibility.isSet (CastlingRights::Kingside) );
        CHECK( eligibility.isSet (CastlingRights::Queenside) );
        CHECK( eligibility.toInt<uint8_t>() == 3 );
    }
    
    SUBCASE( "Clear operations" )
    {
        eligibility.set (CastlingRights::Kingside | CastlingRights::Queenside);
        CHECK( eligibility.toInt<uint8_t>() == 3 );
        
        eligibility.clear (CastlingRights::Kingside);
        CHECK( !eligibility.isSet (CastlingRights::Kingside) );
        CHECK( eligibility.isSet (CastlingRights::Queenside) );
        CHECK( eligibility.toInt<uint8_t>() == 2 );
        
        eligibility.clear (CastlingRights::Queenside);
        CHECK( !eligibility.isSet (CastlingRights::Kingside) );
        CHECK( !eligibility.isSet (CastlingRights::Queenside) );
        CHECK( eligibility.toInt<uint8_t>() == 0 );
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
        CHECK( both.toInt<uint8_t>() == 3 );
    }
    
    SUBCASE( "AND operator" )
    {
        auto both = kingside | queenside;
        auto result_king = both & kingside;
        auto result_queen = both & queenside;
        
        CHECK( result_king.toInt<uint8_t>() == 1 );
        CHECK( result_queen.toInt<uint8_t>() == 2 );
    }
    
    SUBCASE( "XOR operator" )
    {
        auto both = kingside | queenside;
        auto result = both ^ kingside;
        
        CHECK( !result.isSet (CastlingRights::Kingside) );
        CHECK( result.isSet (CastlingRights::Queenside) );
        CHECK( result.toInt<uint8_t>() == 2 );
    }
}

TEST_CASE( "CastlingEligibility - assignment operators" )
{
    CastlingEligibility eligibility {};
    
    SUBCASE( "OR assignment" )
    {
        eligibility |= CastlingRights::Kingside;
        CHECK( eligibility.toInt<uint8_t>() == 1 );
        
        eligibility |= CastlingRights::Queenside;
        CHECK( eligibility.toInt<uint8_t>() == 3 );
    }
    
    SUBCASE( "AND assignment" )
    {
        eligibility = CastlingRights::Kingside | CastlingRights::Queenside;
        eligibility &= CastlingRights::Kingside;
        CHECK( eligibility.toInt<uint8_t>() == 1 );
    }
    
    SUBCASE( "XOR assignment" )
    {
        eligibility = CastlingRights::Kingside | CastlingRights::Queenside;
        eligibility ^= CastlingRights::Kingside;
        CHECK( eligibility.toInt<uint8_t>() == 2 );
    }
    
    SUBCASE( "Regular assignment" )
    {
        eligibility = CastlingRights::Queenside;
        CHECK( eligibility.toInt<uint8_t>() == 2 );
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
        CHECK( !static_cast<bool> (empty) );
    }
    
    SUBCASE( "Non-empty eligibility is true" )
    {
        CHECK( static_cast<bool> (CastlingRights::Kingside) );
        CHECK( static_cast<bool> (CastlingRights::Queenside) );
        CHECK( static_cast<bool> (CastlingRights::Kingside | CastlingRights::Queenside) );
    }
}

TEST_CASE( "CastlingRights - static constants" )
{
    SUBCASE( "Kingside constant" )
    {
        CHECK( CastlingRights::Kingside.toInt<uint8_t>() == 1 );
        CHECK( CastlingRights::Kingside.isSet (CastlingRights::Kingside) );
        CHECK( !CastlingRights::Kingside.isSet (CastlingRights::Queenside) );
    }
    
    SUBCASE( "Queenside constant" )
    {
        CHECK( CastlingRights::Queenside.toInt<uint8_t>() == 2 );
        CHECK( !CastlingRights::Queenside.isSet (CastlingRights::Kingside) );
        CHECK( CastlingRights::Queenside.isSet (CastlingRights::Queenside) );
    }
}

TEST_CASE( "Global constants" )
{
    SUBCASE( "CastlingEligibility::Either_Side" )
    {
        CHECK( CastlingEligibility::Either_Side.toInt<uint8_t>() == 3 );
        CHECK( static_cast<bool> (CastlingEligibility::Either_Side) );
    }
    
    SUBCASE( "CastlingEligibility::Neither_Side" )
    {
        CHECK( CastlingEligibility::Neither_Side.toInt<uint8_t>() == 0 );
        CHECK( !static_cast<bool> (CastlingEligibility::Neither_Side) );
        CHECK( !CastlingEligibility::Neither_Side.isSet (CastlingRights::Kingside) );
        CHECK( !CastlingEligibility::Neither_Side.isSet (CastlingRights::Queenside) );
    }
}

TEST_CASE( "toInt template function" )
{
    auto eligibility = CastlingRights::Kingside | CastlingRights::Queenside;
    
    SUBCASE( "Different integer types" )
    {
        CHECK( toInt<uint8_t> (eligibility) == 3 );
        CHECK( toInt<uint16_t> (eligibility) == 3 );
        CHECK( toInt<uint32_t> (eligibility) == 3 );
        CHECK( toInt<int> (eligibility) == 3 );
    }
    
    SUBCASE( "Default template parameter" )
    {
        CHECK( toInt (eligibility) == 3 );
    }
}