
#include "board_check.h"
#include "move.h"
#include "board.h"

void board_check_init (struct board_check *board_check, struct board &board)
{
#ifdef CHECK_BOARD_EVERY_MOVE
    board_hash_init (&board_check->hash, board);
#endif
}

void board_check_validate (struct board_check *board_check, struct board &restored_board, uint8_t who, struct move mv)
{
#ifdef CHECK_BOARD_EVERY_MOVE
    struct board_hash updated_hash;

    board_hash_init (&updated_hash, restored_board);
    if (updated_hash.hash != board_check->hash.hash)
    {
        printf ("move considering: %s (%s to move)\n", to_string(*mv).c_str(),
                who == COLOR_WHITE ? "White" : "Black");
        board_dump (restored_board);
        assert (0);
    }
#endif
}

