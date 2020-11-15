#ifndef EVOLVE_CHESS_GAME_H_
#define EVOLVE_CHESS_GAME_H_

#include <memory>
#include <optional>

#include "piece.h"
#include "move.h"
#include "board.h"
#include "move_history.hpp"
#include "board_builder.hpp"

struct game
{
	struct board       board;
	move_history_t     history;
	enum color         player;   // side the computer is playing as
	enum color         turn;

	game(enum color _turn, enum color computer_player) :
	        player { computer_player },
	        turn { _turn }
    {
        assert (is_color_valid (_turn));
        assert (is_color_valid(computer_player));
        player = computer_player;
        turn = _turn;
    }

    game(enum color _turn, enum color computer_player, board_builder builder)
            : board { builder.build() },
            player { computer_player},
            turn { _turn }
    {
        assert (is_color_valid (_turn));
        assert (is_color_valid(computer_player));
    }

    bool save();
	static std::optional<game> load(enum color player);

	void move(move_t move);
};

#endif // EVOLVE_CHESS_GAME_H_
