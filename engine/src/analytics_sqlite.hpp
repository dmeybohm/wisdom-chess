#ifndef WISDOM_ANALYTICS_SQLITE_HPP
#define WISDOM_ANALYTICS_SQLITE_HPP

#include "analytics.hpp"

namespace wisdom::analysis
{
    class Logger;

    std::unique_ptr<Analytics> make_sqlite_analytics (const std::string &analytics_file, Logger &logger);
}

#endif //WISDOM_ANALYTICS_SQLITE_HPP
