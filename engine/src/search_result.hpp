#ifndef WISDOM_SEARCH_RESULT_HPP
#define WISDOM_SEARCH_RESULT_HPP

#include "variation_glimpse.hpp"

namespace wisdom
{
    struct SearchResult
    {
        optional<Move> move;
        int score;
        int depth;
        bool timed_out;

        static SearchResult from_initial () noexcept
        {
            SearchResult result { nullopt, -Initial_Alpha, 0, false };
            return result;
        }
    };
}

#endif //WISDOM_SEARCH_RESULT_HPP
