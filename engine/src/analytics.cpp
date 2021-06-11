#include "analytics.hpp"

#include <sqlite3.h>

namespace wisdom
{
    // destructors for interfaces:
    AnalyzedPosition::~AnalyzedPosition () = default;

    AnalyzedDecision::~AnalyzedDecision () = default;

    Analytics::~Analytics () = default;

    std::unique_ptr<Analytics> make_dummy_analytics ()
    {
        return std::make_unique<DummyAnalytics> ();
    }
}