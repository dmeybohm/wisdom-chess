#ifndef EVOLVE_CHESS_GAME_H_
#define EVOLVE_CHESS_GAME_H_

struct board;
struct move_tree;
struct move;

struct game
{
	struct board      *board;
	struct move_tree  *history;
	color_t            player;   /* side the computer is playing as */
	color_t            turn;
};

struct game *game_new   (color_t turn, color_t computer_player);
void         game_free  (struct game *game);

int          game_save  (struct game *game);
struct game *game_load  (const char *file, color_t player);
void         game_move  (struct game *game, struct move *move);

#endif /* EVOLVE_CHESS_GAME_H_ */
