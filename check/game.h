#ifndef EVOLVE_CHESS_GAME_H_
#define EVOLVE_CHESS_GAME_H_

struct board;
struct move_tree;

struct game
{
	struct board      *board;
	struct move_tree  *history;
	color_t            turn;
};

#endif /* EVOLVE_CHESS_GAME_H_ */
