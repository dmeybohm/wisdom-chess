#pragma once

#include "wisdom-chess/engine/global.hpp"

namespace wisdom
{
    [[nodiscard]] constexpr auto
    randomSeed()
        -> std::uint64_t
    {
        return 0xfa082aaf7c7e212ULL;
    }

    [[nodiscard]] constexpr auto
    randomInitialState()
        -> std::uint64_t
    {
        return 0x853c49e6748fea9bULL;
    }

    /**
     * Derived from Jason Turner's YouTube video:  https://godbolt.org/g/zbWvXK
     *
     * This is used to generate a 48-bit random number, which combined with 16-bit
     * random state yields the board position Zobrist hash code.
     */
    struct CompileTimeRandom
    {
        struct RandomState
        {
            std::uint64_t state = randomInitialState();
            std::uint64_t inc = randomSeed();
        };
        RandomState rng;
        using ResultType = std::uint32_t;

        constexpr CompileTimeRandom() = default;

        constexpr explicit CompileTimeRandom (RandomState initial_state)
            : rng { initial_state }
        {
        }

        [[nodiscard]] constexpr auto
        operator()()
            -> ResultType
        {
            return pcg32_random_r();
        }

        [[nodiscard]] static constexpr auto
        min()
            -> ResultType
        {
            return std::numeric_limits<ResultType>::min();
        }

        [[nodiscard]] static constexpr auto
        max()
            -> ResultType
        {
            return std::numeric_limits<ResultType>::max();
        }

    private:
        [[nodiscard]] constexpr auto
        pcg32_random_r()
            -> std::uint32_t
        {
            std::uint64_t oldState = rng.state;
            // Advance internal state
            rng.state = oldState * 6364136223846793005ULL + (rng.inc | 1);
            // Calculate output function (XSH RR), uses old state for max ILP
            auto xorshifted = truncate<std::uint32_t> (((oldState >> 18u) ^ oldState) >> 27u);
            auto rot = truncate<std::uint32_t> (oldState >> 59u);
            return (xorshifted >> rot) | (xorshifted << ((32u - rot) & 31u));
        }
    };

    [[nodiscard]] constexpr auto
    getCompileTimeRandom48 (CompileTimeRandom& random)
        -> std::uint64_t
    {
        return ((random() & 0xffff0000ULL) << 16ULL) | random();
    }
}
