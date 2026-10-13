#pragma once

#include <bit>

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/coord.hpp"

namespace wisdom
{
    // A set of squares, one bit for each square index.
    class CoordSet
    {
    public:
        class Iterator
        {
        public:
            using difference_type = int;
            using value_type = Coord;
            using reference = Coord;
            using iterator_category = std::forward_iterator_tag;

            constexpr
            Iterator() noexcept = default;

            explicit constexpr
            Iterator (uint64_t bits) noexcept
                : my_bits { bits }
            {}

            [[nodiscard]] constexpr auto
            operator*() const noexcept
                -> Coord
            {
                return Coord::fromIndex (std::countr_zero (my_bits));
            }

            constexpr auto
            operator++() noexcept
                -> Iterator&
            {
                my_bits &= my_bits - 1;
                return *this;
            }

            constexpr auto
            operator++ (int) noexcept
                -> Iterator
            {
                auto previous = *this;
                ++*this;
                return previous;
            }

            [[nodiscard]] constexpr auto
            operator== (const Iterator& other) const noexcept
                -> bool
            {
                return my_bits == other.my_bits;
            }

        private:
            uint64_t my_bits = 0;
        };

        constexpr
        CoordSet() noexcept = default;

        explicit constexpr
        CoordSet (uint64_t bits) noexcept
            : my_bits { bits }
        {}

        [[nodiscard]] static constexpr auto
        bit (Coord coord) noexcept
            -> uint64_t
        {
            return uint64_t { 1 } << coord.index();
        }

        [[nodiscard]] constexpr auto
        contains (Coord coord) const noexcept
            -> bool
        {
            return (my_bits & bit (coord)) != 0;
        }

        [[nodiscard]] constexpr auto
        size() const noexcept
            -> int
        {
            return std::popcount (my_bits);
        }

        [[nodiscard]] constexpr auto
        begin() const noexcept
            -> Iterator
        {
            return Iterator { my_bits };
        }

        [[nodiscard]] constexpr auto
        end() const noexcept  // NOLINT(readability-convert-member-functions-to-static)
            -> Iterator
        {
            return Iterator {};
        }

        [[nodiscard]] constexpr auto
        operator== (const CoordSet& other) const noexcept
            -> bool = default;

    private:
        uint64_t my_bits = 0;
    };
}
