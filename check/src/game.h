#ifndef EVOLVE_CHESS_GAME_H_
#define EVOLVE_CHESS_GAME_H_

#include <memory>
#include <optional>

#include "piece.h"
#include "move.h"
#include "board.h"
#include "move_history.hpp"
#include "board_builder.hpp"
#include "history.hpp"

struct Game
{
	Board            board;
	History          history;
	Color            player;   // side the computer is playing as
	Color            turn;

	Game (Color _turn, Color computer_player) :
	        player { computer_player },
	        turn { _turn }
    {
        assert (is_color_valid(_turn));
        assert (is_color_valid(computer_player));
        player = computer_player;
        turn = _turn;
    }

    Game (Color _turn, Color computer_player, BoardBuilder builder)
            : board { builder.build() },
            player { computer_player},
            turn { _turn }
    {
        assert (is_color_valid(_turn));
        assert (is_color_valid(computer_player));
    }

    bool save();
	static std::optional<Game> load (Color player);

	void move (Move move);
};

#endif // EVOLVE_CHESS_GAME_H_
