#ifndef WISDOMCHESS_WEB_LOGGER_HPP
#define WISDOMCHESS_WEB_LOGGER_HPP

#include <emscripten.h>

#include "game.hpp"
#include "logger.hpp"

namespace wisdom::worker
{
    class WebLogger : public wisdom::Logger
    {
    private:
        static std::unique_ptr<wisdom::Game> my_worker_game;

    public:
        WebLogger() = default;

        void debug (const std::string& output) const override;
        void info (const std::string& output) const override;

        friend auto get_game () -> wisdom::observer_ptr<wisdom::Game>;

        static void console_log (const char* str);
    };

    [[nodiscard]] inline auto get_logger() -> const WebLogger&
    {
        using namespace wisdom;
        static std::unique_ptr<WebLogger> worker_logger = std::make_unique<WebLogger>();
        return *worker_logger;
    }

    [[nodiscard]] inline auto get_game () -> wisdom::observer_ptr<wisdom::Game>
    {
        using namespace wisdom;
        if (!WebLogger::my_worker_game) {
            WebLogger::my_worker_game = std::make_unique<wisdom::Game> (Player::Human, Player::ChessEngine);
        }
        return WebLogger::my_worker_game.get();
    }
}

#endif // WISDOMCHESS_WEB_LOGGER_HPP
