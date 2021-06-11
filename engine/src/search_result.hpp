#ifndef WISDOM_SEARCH_RESULT_HPP
#define WISDOM_SEARCH_RESULT_HPP

#include "variation_glimpse.hpp"

namespace wisdom
{
    struct SearchResult
    {
        VariationGlimpse variation_glimpse;
        std::optional<Move> move;
        int score;
        int depth;
        bool timed_out;

        static SearchResult from_initial () noexcept
        {
            SearchResult result { {}, std::nullopt, -Initial_Alpha, 0, false };
            return result;
        }

        static SearchResult from_timeout () noexcept
        {
            SearchResult result { {}, std::nullopt, -Initial_Alpha, 0, true };
            return result;
        }
    };
}

#endif //WISDOM_SEARCH_RESULT_HPP
