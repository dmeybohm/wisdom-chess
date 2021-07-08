#include "analytics.hpp"

#include <sqlite3.h>

namespace wisdom::analysis
{
    // dummy destructors for interfaces:
    Analytics::~Analytics () = default;
    IterativeSearch::~IterativeSearch () = default;
    Iteration::~Iteration () = default;
    Search::~Search () = default;
    Decision::~Decision () = default;
    Position::~Position () = default;

    Analytics &make_dummy_analytics ()
    {
        return DummyAnalytics::get_analytics();
    }
}