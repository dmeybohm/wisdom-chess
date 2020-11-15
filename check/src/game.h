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

	game(enum color human_player, enum color _turn)
    {
        board = board_new ();
        enum color computer_player = color_invert(human_player);

        // if 'turn' is something bogus, use white
        if (is_color_invalid (_turn))
            _turn = COLOR_WHITE;

        // if 'computer_player' is bogus, use black
        if (is_color_invalid (computer_player))
            computer_player = COLOR_BLACK;

        player = computer_player;
        turn   = _turn;
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
