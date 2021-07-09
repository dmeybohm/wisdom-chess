#include "analytics.hpp"

namespace wisdom::analysis
{
    Decision Decision::make_child (Position &position)
    {
        if (impl)
            return impl->make_child (position);
        else
            return {};
    }

    Position Decision::make_position (Move move)
    {
        if (impl)
            return impl->make_position (move);
        else
            return {};
    }

    void Decision::finalize (const SearchResult &result)
    {
        if (impl)
            impl->finalize (result);
    }

    void Decision::preliminary_choice (Position &position)
    {
        if (impl)
            impl->preliminary_choice (position);
    }

    Analytics &make_dummy_analytics ()
    {
        static Analytics analytics;
        return analytics;
    }
}