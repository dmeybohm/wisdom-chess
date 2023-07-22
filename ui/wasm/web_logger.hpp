#ifndef WISDOMCHESS_WEB_LOGGER_HPP
#define WISDOMCHESS_WEB_LOGGER_HPP

#include <emscripten.h>

#include "game.hpp"
#include "logger.hpp"

namespace wisdom::worker
{
    class WebLogger : public wisdom::Logger
    {
    public:
        WebLogger() = default;

        void debug (const std::string& output) const override;
        void info (const std::string& output) const override;

        static void consoleLog (const char* str);
    };

    [[nodiscard]] inline auto getLogger() -> const WebLogger&
    {
        using namespace wisdom;
        static std::unique_ptr<WebLogger> worker_logger = std::make_unique<WebLogger>();
        return *worker_logger;
    }
}

#endif // WISDOMCHESS_WEB_LOGGER_HPP
