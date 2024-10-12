#pragma once

#include <emscripten.h>

#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/logger.hpp"

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

    [[nodiscard]] inline auto makeLogger() -> shared_ptr<WebLogger>
    {
        using namespace wisdom;
        static std::shared_ptr<WebLogger> worker_logger = std::make_shared<WebLogger>();
        return worker_logger;
    }
}

