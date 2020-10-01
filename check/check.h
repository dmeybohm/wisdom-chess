#ifndef EVOLVE_CHESS_CHECK_H
#define EVOLVE_CHESS_CHECK_H

struct move_tree;
struct move;

int    was_legal_move      (struct board *board, color_t who, move_t *mv);
int    is_king_threatened  (struct board *board, color_t who, unsigned char row,
                            unsigned char col);
int    is_checkmated       (struct board *board, color_t who);
int    is_drawing_move     (struct move_tree *history, struct move *move);

#endif /* EVOLVE_CHESS_CHECK_H */
