#ifndef WISDOM_ANALYTICS_SQLITE_HPP
#define WISDOM_ANALYTICS_SQLITE_HPP

#include "analytics.hpp"

namespace wisdom
{
    class Logger;
}

namespace wisdom::analysis
{
    auto make_sqlite_analytics (const wisdom::string &analytics_file, wisdom::Logger &logger)
        -> wisdom::unique_ptr<Analytics>;
}

#endif //WISDOM_ANALYTICS_SQLITE_HPP
