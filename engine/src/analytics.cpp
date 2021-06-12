#include "analytics.hpp"

#include <sqlite3.h>

namespace wisdom::analysis
{
    // dummy destructors for interfaces:
    Position::~Position () = default;
    Search::~Search () = default;
    Decision::~Decision () = default;
    Analytics::~Analytics () = default;

    std::unique_ptr<Analytics> make_dummy_analytics ()
    {
        return std::make_unique<DummyAnalytics> ();
    }
}