#include "analytics.hpp"

#include <sqlite3.h>

namespace wisdom::analysis
{
    // dummy destructors for interfaces:
    Position::~Position () = default;
    Search::~Search () = default;
    Decision::~Decision () = default;
    Analytics::~Analytics () = default;

    Analytics *make_dummy_analytics ()
    {
        return DummyAnalytics::get_analytics();
    }
}