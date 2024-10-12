#pragma once

#include "wisdom-chess/engine/global.hpp"

namespace wisdom
{
    constexpr auto 
    randomSeed() 
        -> std::uint64_t
    {
        return 0x123456789abcdefaULL;
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
            std::uint64_t state = 0;
            std::uint64_t inc = randomSeed();
        };
        RandomState rng;
        using ResultType = std::uint32_t;

        constexpr auto 
        operator()()
            -> ResultType 
        {
            return pcg32_random_r();
        }

        static auto constexpr 
        min()
            -> ResultType 
        {
            return std::numeric_limits<ResultType>::min();
        }

        static auto constexpr 
        max()
            -> ResultType
        {
            return std::numeric_limits<ResultType>::min();
        }

    private:
        constexpr auto 
        pcg32_random_r()
            -> std::uint32_t 
        {
            std::uint64_t oldState = rng.state;
            // Advance internal state
            rng.state = oldState * 6364136223846793005ULL + (rng.inc | 1);
            // Calculate output function (XSH RR), uses old state for max ILP
            std::uint32_t xorshifted = ((oldState >> 18u) ^ oldState) >> 27u;
            std::uint32_t rot = oldState >> 59u;
            return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
        }
    };

    constexpr auto 
    getCompileTimeRandom48 (CompileTimeRandom& random) 
        -> std::uint64_t
    {
        return ((random() & 0xffff0000ULL) << 16ULL) | random();
    }
}
