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

        void debug (const std::string& output) noexcept override;
        void info (const std::string& output) noexcept override;
        void emergency (std::string_view output) noexcept override;

        static void consoleLog (czstring str);
        static void consoleError (czstring str);
    };

    [[nodiscard]] inline auto
    makeLogger()
        -> shared_ptr<WebLogger>
    {
        using namespace wisdom;
        static std::shared_ptr<WebLogger> worker_logger = std::make_shared<WebLogger>();
        return worker_logger;
    }
}
