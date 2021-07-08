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

    class Game
    {
    public:
        Game (Color current_turn, Color computer_player);

        Game (Color current_turn, Color computer_player, BoardBuilder builder);

        void save (const std::string &filename) const;

        static std::optional<Game> load (const std::string &filename, Color player);

        std::optional<Move> find_best_move (Logger &logger, Color whom = Color::None);

        void move (Move move);

        [[nodiscard]] Color get_computer_player () const;

        void set_computer_player (Color player);

        [[nodiscard]] Color get_current_turn () const;

        [[nodiscard]] Board &get_board() const;

        [[nodiscard]] History &get_history () const;

        void set_analytics (std::unique_ptr<analysis::Analytics> new_analytics);

    private:
        std::unique_ptr<Board> my_board = std::make_unique<Board> ();
        std::unique_ptr<History> my_history = std::make_unique<History> ();
        std::unique_ptr<analysis::Analytics> my_analytics = std::make_unique<analysis::DummyAnalytics> ();
        Color my_current_turn;        // whose turn it is
        Color my_computer_player;     // side the computer is playing as
    };
}

#endif // WISDOM_CHESS_GAME_H_
