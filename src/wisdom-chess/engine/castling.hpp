#pragma once

#include "wisdom-chess/engine/global.hpp"

namespace wisdom
{
    // Represents castling eligibility using intuitive bit logic:
    // Bit set (1) = eligible to castle on that side
    // Bit clear (0) = not eligible to castle on that side
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
            Expects ((flags &~ (0x1|0x2)) == 0);
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
        
        [[nodiscard]] constexpr auto
        operator| (CastlingEligibility other) const noexcept
            -> CastlingEligibility
        {
            return fromInt (my_flags | other.my_flags);
        }
        
        [[nodiscard]] constexpr auto
        operator& (CastlingEligibility other) const noexcept
            -> CastlingEligibility
        {
            return fromInt (my_flags & other.my_flags);
        }
        
        [[nodiscard]] constexpr auto
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
        
        [[nodiscard]] constexpr auto
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

        [[nodiscard]] constexpr auto
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
            static_assert (std::is_unsigned_v<IntegerType>);
            return narrow_cast<IntegerType> (my_flags);
        }

        [[nodiscard]] constexpr auto
        canCastleKingside() const noexcept
            -> bool;

        [[nodiscard]] constexpr auto
        canCastleQueenside() const noexcept
            -> bool;

        // Static constants for common eligibility states (defined after class)
        static const CastlingEligibility Either_Side;
        static const CastlingEligibility Neither_Side;

    };

    template <typename IntegerType = uint8_t>
    constexpr auto
    toInt (CastlingEligibility eligibility)
        -> IntegerType
    {
        static_assert (std::is_unsigned_v<IntegerType>);
        return eligibility.toInt<IntegerType>();
    }

    namespace CastlingRights
    {
        constexpr static CastlingEligibility Kingside { 1 };
        constexpr static CastlingEligibility Queenside { 2 };
    };

    // Define the static constants for CastlingEligibility
    inline constexpr CastlingEligibility CastlingEligibility::Either_Side
        = CastlingRights::Kingside | CastlingRights::Queenside;
    inline constexpr CastlingEligibility CastlingEligibility::Neither_Side {};


    constexpr auto
    makeCastlingEligibilityFromInt (unsigned int flags)
        -> CastlingEligibility
    {
        return CastlingEligibility { narrow<uint8_t> (flags) };
    }

    // Inline definitions for helper methods
    inline constexpr auto
    CastlingEligibility::canCastleKingside() const noexcept
        -> bool
    {
        return isSet (CastlingRights::Kingside);
    }

    inline constexpr auto
    CastlingEligibility::canCastleQueenside() const noexcept
        -> bool
    {
        return isSet (CastlingRights::Queenside);
    }

    // Send the castling eligibility to the ostream.
    auto
    operator<< (std::ostream& os, const CastlingEligibility& value)
        -> std::ostream&;

}
