#ifndef EVOLVE_CHESS_GAME_H_
#define EVOLVE_CHESS_GAME_H_

#include "piece.h"

struct move;

struct game
{
	struct board      *board;
	struct move_tree  *history;
	enum color         player;   /* side the computer is playing as */
	enum color         turn;
};

struct game *game_new   (enum color turn, enum color computer_player);
void         game_free  (struct game *game);

int          game_save  (struct game *game);
struct game *game_load  (enum color player);
void         game_move  (struct game *game, struct move move);

#endif // EVOLVE_CHESS_GAME_H_
