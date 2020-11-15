#ifndef EVOLVE_CHESS_GAME_H_
#define EVOLVE_CHESS_GAME_H_

#include <memory>

#include "piece.h"
#include "move.h"
#include "board.h"
#include "move_history.hpp"

struct game
{
	struct board      *board;
	move_history_t     history;
	enum color         player;   // side the computer is playing as
	enum color         turn;

	game(enum color _turn, enum color computer_player)
    {
        assert (is_color_valid (_turn));
        assert (is_color_valid(computer_player));
        board = board_new();
        player = computer_player;
        turn = _turn;
    }

	~game()
    {
	    board_free (board);
    }

    bool save();
	static std::unique_ptr<game> load(enum color player);

	void move(move_t move);
};

#endif // EVOLVE_CHESS_GAME_H_
