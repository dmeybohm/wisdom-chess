#ifndef WISDOM_CHESS_GAME_H_
#define WISDOM_CHESS_GAME_H_

#include "global.hpp"
#include "piece.hpp"
#include "move.hpp"
#include "board.hpp"
#include "analytics.hpp"
#include "history.hpp"

namespace wisdom::analysis
{
    class Analytics;
}

namespace wisdom
{
    class Board;
    class History;
    class BoardBuilder;
    class Logger;

    struct Game
    {
        std::unique_ptr<Board> board = std::make_unique<Board> ();
        std::unique_ptr<History> history = std::make_unique<History> ();
        std::unique_ptr<analysis::Analytics> analytics = std::make_unique<analysis::DummyAnalytics>();
        Color player;   // side the computer is playing as
        Color turn;

        Game (Color turn_, Color computer_player);

        Game (Color turn_, Color computer_player, BoardBuilder builder);

        void save (const std::string &filename) const;

        static std::optional<Game> load (const std::string &filename, Color player);

        std::optional<Move> find_best_move (Logger &logger, Color whom = Color::None);

        void set_analytics (std::unique_ptr<analysis::Analytics> new_analytics);

        void move (Move move);


    };
}

#endif // WISDOM_CHESS_GAME_H_
