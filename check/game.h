#ifndef EVOLVE_CHESS_GAME_H_
#define EVOLVE_CHESS_GAME_H_

struct game
{
	struct board    *board;
	move_tree_t     *history;
	color_t          turn;
};

#endif /* EVOLVE_CHESS_GAME_H_ */
