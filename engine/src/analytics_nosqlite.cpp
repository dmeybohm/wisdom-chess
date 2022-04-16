#include "analytics_sqlite.hpp"

namespace wisdom::analysis
{
    auto make_sqlite_analytics (const wisdom::string &analytics_file, wisdom::Logger &logger)
    -> wisdom::unique_ptr<Analytics>
    {
        return std::make_unique<Analytics>();
    }
}
