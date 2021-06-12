#ifndef WISDOM_CHESS_GAME_H_
#define WISDOM_CHESS_GAME_H_

#include "global.hpp"
#include "piece.hpp"
#include "move.hpp"
#include "board.hpp"
#include "move_history.hpp"
#include "board_builder.hpp"
#include "history.hpp"
#include "analytics.hpp"

namespace wisdom
{
    struct Game
    {
        Board board;
        History history;
        Color player;   // side the computer is playing as
        Color turn;
        std::unique_ptr<analysis::Analytics> analytics;

        Game (Color _turn, Color computer_player) :
                player { computer_player },
                turn { _turn },
                analytics { analysis::make_dummy_analytics() }
        {
            assert (is_color_valid (_turn));
            assert (is_color_valid (computer_player));
            player = computer_player;
            turn = _turn;
        }

        Game (Color _turn, Color computer_player, BoardBuilder builder)
                : board { builder.build () },
                  player { computer_player },
                  turn { _turn },
                  analytics { analysis::make_dummy_analytics() }
        {
            assert (is_color_valid (_turn));
            assert (is_color_valid (computer_player));
        }

        bool save (const std::string &filename) const;

        static std::optional<Game> load (const std::string &filename, Color player);

        void move (Move move);
    };
}

#endif // WISDOM_CHESS_GAME_H_
