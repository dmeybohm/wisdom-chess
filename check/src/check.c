#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "coord.h"
#include "move.h"
#include "board.h"
#include "generate.h"
#include "move_list.h"
#include "check.h"

bool is_checkmated (struct board *board, color_t who)
{
	int          row, col;
	move_list_t *legal_moves;

	coord_t king_pos = king_position (board, who);
	row = ROW    (king_pos);
	col = COLUMN (king_pos);

	if (!is_king_threatened (board, who, row, col))
		return false;

	legal_moves = generate_legal_moves (board, who, NULL);
	bool result = !legal_moves;

	move_list_destroy (legal_moves);

	return result;
}

bool is_king_threatened (struct board *board, color_t who,
                         uint8_t king_row, uint8_t king_col)
{
    return attack_vector_count (&board->attacks, color_invert(who), coord_create (king_row, king_col));
}

bool was_legal_move (struct board *board, color_t who, move_t *mv)
{
    coord_t king_pos = king_position (board, who);

	if (is_king_threatened (board, who, ROW(king_pos), COLUMN(king_pos)))
	{
		return false;
	}

	if (is_castling_move (mv))
    {
	    coord_t castled_pos = MOVE_DST (*mv);

	    int castled_row = ROW(castled_pos);
	    int castled_col = COLUMN(castled_pos);
	    assert (ROW(king_pos) == castled_row);
	    assert (COLUMN(king_pos) == castled_col);

        int direction = is_castling_move_on_king_side(mv) ? -1 : 1;

        if (is_king_threatened (board, who, castled_row, NEXT (castled_col, direction)) ||
            is_king_threatened (board, who, castled_row, NEXT (NEXT (castled_col, direction), direction)))
        {
            return false;
        }

    }

	return true;
}

bool is_drawing_move (const move_tree_t *move_tree, const move_t *move)
{
    return false;
}

// vi: set ts=4 sw=4:
