#ifndef WIZDUMB_BOARD_CHECK_H
#define WIZDUMB_BOARD_CHECK_H

struct board;
struct move;

// Do a hash function run every board to check if state was updated correctly

// #define CHECK_BOARD_EVERY_MOVE 1

#include "board_hash.h"
#include "move.h"

typedef struct board_check
{
    struct board_hash hash;
} board_check_t;

void board_check_init (struct board_check *__restrict board_check, struct board *board);
void board_check_validate (struct board_check *board_check, struct board *restored_board, uint8_t who, move_t move);

#endif // WIZDUMB_BOARD_CHECK_H
