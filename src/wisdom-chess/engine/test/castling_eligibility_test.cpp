#include "wisdom-chess/engine/castling.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "CastlingEligibility - Default construction" )
{
    CastlingEligibility eligibility {};
    
    SUBCASE( "Default is eligible for both sides" )
    {
        CHECK( !eligibility.isSet (CastlingIneligible::Kingside) );
        CHECK( !eligibility.isSet (CastlingIneligible::Queenside) );
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
    SUBCASE( "Kingside ineligible" )
    {
        CastlingEligibility eligibility{ 1 };
        CHECK( eligibility.isSet (CastlingIneligible::Kingside) );
        CHECK( !eligibility.isSet (CastlingIneligible::Queenside) );
        CHECK( static_cast<bool> (eligibility) );
        CHECK( eligibility.toInt<uint8_t>() == 1 );
    }
    
    SUBCASE( "Queenside ineligible" )
    {
        CastlingEligibility eligibility{ 2 };
        CHECK( !eligibility.isSet (CastlingIneligible::Kingside) );
        CHECK( eligibility.isSet (CastlingIneligible::Queenside) );
        CHECK( static_cast<bool> (eligibility) );
        CHECK( eligibility.toInt<uint8_t>() == 2 );
    }
    
    SUBCASE( "Both sides ineligible" )
    {
        CastlingEligibility eligibility { 3 };
        CHECK( eligibility.isSet (CastlingIneligible::Kingside) );
        CHECK( eligibility.isSet (CastlingIneligible::Queenside) );
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
        CHECK( one.isSet (CastlingIneligible::Kingside) );
        
        auto two = makeCastlingEligibilityFromInt (2);
        CHECK( two.toInt<uint8_t>() == 2 );
        CHECK( two.isSet (CastlingIneligible::Queenside) );
        
        auto three = makeCastlingEligibilityFromInt (3);
        CHECK( three.toInt<uint8_t>() == 3 );
        CHECK( three.isSet (CastlingIneligible::Kingside) );
        CHECK( three.isSet (CastlingIneligible::Queenside) );
    }
}

TEST_CASE( "CastlingEligibility - set and clear operations" )
{
    CastlingEligibility eligibility {};
    
    SUBCASE( "Set kingside ineligible" )
    {
        eligibility.set (CastlingIneligible::Kingside);
        CHECK( eligibility.isSet (CastlingIneligible::Kingside) );
        CHECK( !eligibility.isSet (CastlingIneligible::Queenside) );
        CHECK( eligibility.toInt<uint8_t>() == 1 );
    }
    
    SUBCASE( "Set queenside ineligible" )
    {
        eligibility.set (CastlingIneligible::Queenside);
        CHECK( !eligibility.isSet (CastlingIneligible::Kingside) );
        CHECK( eligibility.isSet (CastlingIneligible::Queenside) );
        CHECK( eligibility.toInt<uint8_t>() == 2 );
    }
    
    SUBCASE( "Set both sides ineligible" )
    {
        eligibility.set (CastlingIneligible::Kingside);
        eligibility.set (CastlingIneligible::Queenside);
        CHECK( eligibility.isSet (CastlingIneligible::Kingside) );
        CHECK( eligibility.isSet (CastlingIneligible::Queenside) );
        CHECK( eligibility.toInt<uint8_t>() == 3 );
    }
    
    SUBCASE( "Clear operations" )
    {
        eligibility.set (CastlingIneligible::Kingside | CastlingIneligible::Queenside);
        CHECK( eligibility.toInt<uint8_t>() == 3 );
        
        eligibility.clear (CastlingIneligible::Kingside);
        CHECK( !eligibility.isSet (CastlingIneligible::Kingside) );
        CHECK( eligibility.isSet (CastlingIneligible::Queenside) );
        CHECK( eligibility.toInt<uint8_t>() == 2 );
        
        eligibility.clear (CastlingIneligible::Queenside);
        CHECK( !eligibility.isSet (CastlingIneligible::Kingside) );
        CHECK( !eligibility.isSet (CastlingIneligible::Queenside) );
        CHECK( eligibility.toInt<uint8_t>() == 0 );
    }
}

TEST_CASE( "CastlingEligibility - bitwise operators" )
{
    auto kingside = CastlingIneligible::Kingside;
    auto queenside = CastlingIneligible::Queenside;
    
    SUBCASE( "OR operator" )
    {
        auto both = kingside | queenside;
        CHECK( both.isSet (CastlingIneligible::Kingside) );
        CHECK( both.isSet (CastlingIneligible::Queenside) );
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
        
        CHECK( !result.isSet (CastlingIneligible::Kingside) );
        CHECK( result.isSet (CastlingIneligible::Queenside) );
        CHECK( result.toInt<uint8_t>() == 2 );
    }
}

TEST_CASE( "CastlingEligibility - assignment operators" )
{
    CastlingEligibility eligibility {};
    
    SUBCASE( "OR assignment" )
    {
        eligibility |= CastlingIneligible::Kingside;
        CHECK( eligibility.toInt<uint8_t>() == 1 );
        
        eligibility |= CastlingIneligible::Queenside;
        CHECK( eligibility.toInt<uint8_t>() == 3 );
    }
    
    SUBCASE( "AND assignment" )
    {
        eligibility = CastlingIneligible::Kingside | CastlingIneligible::Queenside;
        eligibility &= CastlingIneligible::Kingside;
        CHECK( eligibility.toInt<uint8_t>() == 1 );
    }
    
    SUBCASE( "XOR assignment" )
    {
        eligibility = CastlingIneligible::Kingside | CastlingIneligible::Queenside;
        eligibility ^= CastlingIneligible::Kingside;
        CHECK( eligibility.toInt<uint8_t>() == 2 );
    }
    
    SUBCASE( "Regular assignment" )
    {
        eligibility = CastlingIneligible::Queenside;
        CHECK( eligibility.toInt<uint8_t>() == 2 );
    }
}

TEST_CASE( "CastlingEligibility - equality operators" )
{
    auto kingside = CastlingIneligible::Kingside;
    auto queenside = CastlingIneligible::Queenside;
    auto both = kingside | queenside;
    
    SUBCASE( "Equality" )
    {
        CHECK( kingside == CastlingIneligible::Kingside );
        CHECK( queenside == CastlingIneligible::Queenside );
        CHECK( both == (CastlingIneligible::Kingside | CastlingIneligible::Queenside) );
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
        CHECK( static_cast<bool> (CastlingIneligible::Kingside) );
        CHECK( static_cast<bool> (CastlingIneligible::Queenside) );
        CHECK( static_cast<bool> (CastlingIneligible::Kingside | CastlingIneligible::Queenside) );
    }
}

TEST_CASE( "CastlingIneligible - static constants" )
{
    SUBCASE( "Kingside constant" )
    {
        CHECK( CastlingIneligible::Kingside.toInt<uint8_t>() == 1 );
        CHECK( CastlingIneligible::Kingside.isSet (CastlingIneligible::Kingside) );
        CHECK( !CastlingIneligible::Kingside.isSet (CastlingIneligible::Queenside) );
    }
    
    SUBCASE( "Queenside constant" )
    {
        CHECK( CastlingIneligible::Queenside.toInt<uint8_t>() == 2 );
        CHECK( !CastlingIneligible::Queenside.isSet (CastlingIneligible::Kingside) );
        CHECK( CastlingIneligible::Queenside.isSet (CastlingIneligible::Queenside) );
    }
}

TEST_CASE( "Global constants" )
{
    SUBCASE( "CastlingEligibility::Either_Side" )
    {
        CHECK( CastlingEligibility::Either_Side.toInt<uint8_t>() == 0 );
        CHECK( !static_cast<bool> (CastlingEligibility::Either_Side) );
    }
    
    SUBCASE( "CastlingEligibility::Neither_Side" )
    {
        CHECK( CastlingEligibility::Neither_Side.toInt<uint8_t>() == 3 );
        CHECK( static_cast<bool> (CastlingEligibility::Neither_Side) );
        CHECK( CastlingEligibility::Neither_Side.isSet (CastlingIneligible::Kingside) );
        CHECK( CastlingEligibility::Neither_Side.isSet (CastlingIneligible::Queenside) );
    }
}

TEST_CASE( "toInt template function" )
{
    auto eligibility = CastlingIneligible::Kingside | CastlingIneligible::Queenside;
    
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