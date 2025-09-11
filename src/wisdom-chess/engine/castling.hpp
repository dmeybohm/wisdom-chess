#pragma once

#include "wisdom-chess/engine/global.hpp"

namespace wisdom
{
    enum class CastlingIneligible : uint8_t
    {
        Kingside = 1,
        Queenside = 2,
    };

    template <typename IntegerType = uint8_t>
    constexpr auto
    toInt (CastlingIneligible flag)
        -> IntegerType
    {
        return gsl::narrow_cast<IntegerType> (flag);
    }

    class CastlingEligibility
    {
    private:
        uint8_t flags_;
        
    public:
        constexpr CastlingEligibility() noexcept : flags_ (0)
        {}

        constexpr explicit
        CastlingEligibility (uint8_t flags) noexcept
			: flags_ (flags)
        {}

        [[nodiscard]] constexpr explicit
        CastlingEligibility (CastlingIneligible flag) noexcept :
			flags_ (toInt<uint8_t> (flag))
        {}
        
        [[nodiscard]] constexpr auto isSet (CastlingIneligible flag) const noexcept -> bool
        {
            return (flags_ & toInt (flag)) != 0;
        }
        
        constexpr void set (CastlingIneligible flag) noexcept
        {
            flags_ |= toInt (flag);
        }
        
        constexpr void clear (CastlingIneligible flag) noexcept
        {
            flags_ &= ~toInt (flag);
        }
        
        constexpr auto operator| (CastlingEligibility other) const noexcept -> CastlingEligibility
        {
            return CastlingEligibility (flags_ | other.flags_);
        }
        
        constexpr auto operator& (CastlingEligibility other) const noexcept -> CastlingEligibility
        {
            return CastlingEligibility (flags_ & other.flags_);
        }
        
        constexpr auto operator^ (CastlingEligibility other) const noexcept -> CastlingEligibility
        {
            return CastlingEligibility (flags_ ^ other.flags_);
        }
        
        constexpr auto operator| (CastlingIneligible flag) const noexcept -> CastlingEligibility
        {
            return CastlingEligibility (flags_ | toInt (flag));
        }
        
        constexpr auto operator& (CastlingIneligible flag) const noexcept -> CastlingEligibility
        {
            return CastlingEligibility (flags_ & toInt (flag));
        }
        
        constexpr auto operator^ (CastlingIneligible flag) const noexcept -> CastlingEligibility
        {
            return CastlingEligibility (flags_ ^ toInt (flag));
        }
        
        constexpr auto operator|= (CastlingIneligible flag) noexcept -> CastlingEligibility&
        {
            flags_ |= toInt (flag);
            return *this;
        }
        
        constexpr auto operator^= (CastlingIneligible flag) noexcept -> CastlingEligibility&
        {
            flags_ ^= toInt (flag);
            return *this;
        }
        
        constexpr auto operator&= (CastlingIneligible flag) noexcept -> CastlingEligibility&
        {
            flags_ &= toInt (flag);
            return *this;
        }
        
        constexpr auto operator== (CastlingEligibility other) const noexcept -> bool
        {
            return flags_ == other.flags_;
        }

        constexpr auto operator== (CastlingIneligible flag) const noexcept -> bool
        {
			return flags_ == toInt (flag);
        }

        constexpr CastlingEligibility& operator= (CastlingIneligible flag) noexcept
        {
            flags_ = toInt (flag);
            return *this;
        }

        constexpr auto operator!= (CastlingEligibility other) const noexcept -> bool
        {
            return flags_ != other.flags_;
        }
        
        constexpr explicit
        operator bool() const noexcept
        {
            return flags_ != 0;
        }
        
        [[nodiscard]] constexpr auto
        to_uint8() const noexcept
            -> uint8_t
        {
            return flags_;
        }
    };

    constexpr auto 
    operator| (CastlingIneligible left, CastlingIneligible right) noexcept 
        -> CastlingEligibility
    {
        return CastlingEligibility (toInt (left) | toInt (right));
    }

    inline constexpr CastlingEligibility Either_Side_Eligible {};
    inline constexpr CastlingEligibility Neither_Side_Eligible
        = CastlingIneligible::Kingside | CastlingIneligible::Queenside;

    inline constexpr auto 
    makeCastlingEligibilityFromInt (unsigned int flags) 
        -> CastlingEligibility
    {
        return CastlingEligibility (gsl::narrow<uint8_t> (flags));
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

    template <typename IntegerType = uint8_t>
    constexpr auto
    toInt (CastlingEligibility eligibility) 
        -> IntegerType
    {
        return gsl::narrow_cast<IntegerType> (eligibility.to_uint8());
    }

}
