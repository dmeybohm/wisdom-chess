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
        static std::unique_ptr<DummyAnalytics> dummy_analytics = std::make_unique<DummyAnalytics> ();
        return dummy_analytics.get ();
    }
}