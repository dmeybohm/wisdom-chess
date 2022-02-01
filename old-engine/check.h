#ifndef EVOLVE_CHESS_CHECK_H
#define EVOLVE_CHESS_CHECK_H

int    was_legal_move      (struct board *board, color_t who, move_t *mv);
int    is_king_threatened (struct board *board, color_t who, unsigned char row,
                           unsigned char col);
int    is_checkmated      (struct board *board, color_t who);

#endif /* EVOLVE_CHESS_CHECK_H */
