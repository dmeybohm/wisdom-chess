#ifndef WISDOM_ANALYTICS_SQLITE_HPP
#define WISDOM_ANALYTICS_SQLITE_HPP

#include "analytics.hpp"

namespace wisdom
{
    class Logger;
}

namespace wisdom::analysis
{
    auto make_sqlite_analytics (const std::string &analytics_file, wisdom::Logger &logger)
        -> std::unique_ptr<Analytics>;
}

#endif //WISDOM_ANALYTICS_SQLITE_HPP
