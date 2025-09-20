#pragma once

#include "wisdom-chess/engine/global.hpp"

namespace wisdom
{
    class CastlingEligibility
    {
    private:
        uint8_t my_flags;

        static constexpr auto
        fromInt (unsigned int flags)
            -> CastlingEligibility
        {
            return CastlingEligibility (narrow_cast<uint8_t> (flags));
        }

    public:
        constexpr CastlingEligibility() noexcept : my_flags (0)
        {}

        constexpr explicit
        CastlingEligibility (uint8_t flags) noexcept
            : my_flags { flags }
        {
            assert ((flags &~ (0x1|0x2)) == 0);
        }

        [[nodiscard]] constexpr auto
        isSet (CastlingEligibility flags) const noexcept
            -> bool
        {
            return (my_flags & flags.my_flags) != 0;
        }
        
        constexpr void
        set (CastlingEligibility flags) noexcept
        {
            my_flags |= flags.my_flags;
        }
        
        constexpr void
        clear (CastlingEligibility flags) noexcept
        {
            my_flags &= ~flags.my_flags;
        }
        
        constexpr auto
        operator| (CastlingEligibility other) const noexcept
            -> CastlingEligibility
        {
            return fromInt (my_flags | other.my_flags);
        }
        
        constexpr auto
        operator& (CastlingEligibility other) const noexcept
            -> CastlingEligibility
        {
            return fromInt (my_flags & other.my_flags);
        }
        
        constexpr auto
        operator^ (CastlingEligibility other) const noexcept
            -> CastlingEligibility
        {
            return fromInt (my_flags ^ other.my_flags);
        }
        
        constexpr auto
        operator|= (CastlingEligibility flags) noexcept
            -> CastlingEligibility&
        {
            my_flags |= flags.my_flags;
            return *this;
        }
        
        constexpr auto
        operator^= (CastlingEligibility flags) noexcept
            -> CastlingEligibility&
        {
            my_flags ^= flags.my_flags;
            return *this;
        }
        
        constexpr auto
        operator&= (CastlingEligibility flags) noexcept
            -> CastlingEligibility&
        {
            my_flags &= flags.my_flags;
            return *this;
        }
        
        constexpr auto
        operator== (CastlingEligibility other) const noexcept
            -> bool
        {
            return my_flags == other.my_flags;
        }

        constexpr CastlingEligibility&
        operator= (CastlingEligibility flags) noexcept
        {
            my_flags = flags.my_flags;
            return *this;
        }

        constexpr auto
        operator!= (CastlingEligibility other) const noexcept
            -> bool
        {
            return my_flags != other.my_flags;
        }
        
        constexpr explicit
        operator bool() const noexcept
        {
            return my_flags != 0;
        }

        template <typename IntegerType = uint8_t>
        [[nodiscard]] constexpr auto
        toInt() const
            -> IntegerType
        {
            return narrow_cast<IntegerType> (my_flags);
        }

        // Static constants for common eligibility states (defined after class)
        static const CastlingEligibility Either_Side;
        static const CastlingEligibility Neither_Side;

    };

    template <typename IntegerType = uint8_t>
    constexpr auto
    toInt (CastlingEligibility eligibility) 
        -> IntegerType
    {
        return eligibility.toInt();
    }

    namespace CastlingIneligible
    {
        constexpr static CastlingEligibility Kingside { 1 };
        constexpr static CastlingEligibility Queenside { 2 };
    };

    // Define the static constants for CastlingEligibility
    inline constexpr CastlingEligibility CastlingEligibility::Either_Side {};
    inline constexpr CastlingEligibility CastlingEligibility::Neither_Side
        = CastlingIneligible::Kingside | CastlingIneligible::Queenside;


    constexpr auto
    makeCastlingEligibilityFromInt (unsigned int flags)
        -> CastlingEligibility
	{
		return CastlingEligibility { narrow<uint8_t> (flags) };
	}

    // Send the castling eligibility to the ostream.
    inline auto
    operator<< (std::ostream& os, const CastlingEligibility& value)
        -> std::ostream&
    {
        std::string result = "{ Kingside: ";

        result += value.isSet (CastlingIneligible::Kingside)
            ? "not eligible, "
            : "eligible, ";
        result += "Queenside: ";
        result += value.isSet (CastlingIneligible::Queenside)
            ? "not eligible"
            : "eligible";
        result += " }";

        os << result;
        return os;
    }

}
