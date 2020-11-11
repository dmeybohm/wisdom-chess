#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "coord.h"
#include "move.h"
#include "board.h"
#include "generate.h"
#include "move_list.h"
#include "check.h"

bool is_checkmated (struct board *board, enum color who)
{
	int          row, col;
	move_list_t *legal_moves;

	coord_t king_pos = king_position (board, who);
	row = ROW    (king_pos);
	col = COLUMN (king_pos);

	if (!is_king_threatened (board, who, row, col))
		return false;

	// todo: need history here to be correct with en-passant.
	legal_moves = generate_legal_moves (board, who, NULL);
	bool result = !legal_moves;

	move_list_destroy (legal_moves);

    return result;
}

bool is_king_threatened (struct board *board, enum color who,
                         uint8_t king_row, uint8_t king_col)
{
	int           row, col;
	piece_t       what;
	int           c_dir, r_dir;
	move_list_t  *kt_moves;
	move_t       *move;

	// check each side of the king's row
	col = king_col;
	for (r_dir = -1; r_dir <= 1; r_dir += 2)
	{
		for (row = NEXT (king_row, r_dir); VALID (row); row = NEXT (row, r_dir))
		{
			what = PIECE_AT (board, row, col);

			if (PIECE_TYPE (what) == PIECE_NONE)
				continue;

			if (PIECE_COLOR (what) == who)
				break;

			if (PIECE_TYPE (what) == PIECE_ROOK || 
				PIECE_TYPE (what) == PIECE_QUEEN)
				return true;

			break;
		}
	}

	// check each side of the king's column
	row = king_row;
	for (c_dir = -1; c_dir <= 1; c_dir += 2)
	{
		for (col = NEXT (king_col, c_dir); VALID (col); col = NEXT (col, c_dir))
		{
			what = PIECE_AT (board, row, col);

			if (PIECE_TYPE (what) == PIECE_NONE)
				continue;

			if (PIECE_COLOR (what) == who)
				break;

			if (PIECE_TYPE (what) == PIECE_ROOK || 
				PIECE_TYPE (what) == PIECE_QUEEN)
				return true;

			break;
		}
	}

	// check each diagonal direction
	for (r_dir = -1; r_dir <= 1; r_dir += 2)
	{
		for (c_dir = -1; c_dir <= 1; c_dir += 2)
		{
			for (row = NEXT (king_row, r_dir), col = NEXT (king_col, c_dir); 
				 VALID (row) && VALID (col); 
                 row = NEXT (row, r_dir), col = NEXT (col, c_dir))
			{
				what = PIECE_AT (board, row, col);

				if (PIECE_TYPE (what) == PIECE_NONE)
					continue;

				if (PIECE_COLOR (what) == who)
					break;

				if (PIECE_TYPE (what) == PIECE_BISHOP || 
				    PIECE_TYPE (what) == PIECE_QUEEN)
				{
					return true;
				}

				break;
			}
		}
	}

	// check for knight checks
	kt_moves = generate_knight_moves (king_row, king_col);

	for_each_move (move, kt_moves)
	{
		coord_t dst;

		dst = MOVE_DST (*move);

		row = ROW (dst);
		col = COLUMN (dst);

		what = PIECE_AT (board, row, col);

		if (PIECE_TYPE (what) == PIECE_NONE)
			continue;

		if (PIECE_TYPE (what) == PIECE_KNIGHT && PIECE_COLOR (what) != who)
			return true;
	}
	
	// check for pawn checks
	r_dir = PAWN_DIRECTION (who);

	for (c_dir = -1; c_dir <= 1; c_dir += 2)
	{
		row = NEXT (king_row, r_dir);
		col = NEXT (king_col, c_dir);

		if (INVALID (row) || INVALID (col))
			continue;

		what = PIECE_AT (board, row, col);

		if (PIECE_TYPE (what) == PIECE_PAWN && PIECE_COLOR (what) != who)
			return true;
	}

	// check for king checks
	for (row = king_row-1; row <= king_row+1; row++)
	{
		if (INVALID (row))
			continue;

		for (col = king_col-1; col <= king_col+1; col++)
		{
			if (INVALID (col))
				continue;

			if (col == king_col && row == king_row)
				continue;

			what = PIECE_AT (board, row, col);
			
			if (PIECE_TYPE (what) == PIECE_KING && PIECE_COLOR (what) != who)
				return true;
		}
	}

	return false;
}

bool was_legal_move (struct board *board, enum color who, move_t mv)
{
    coord_t king_pos = king_position (board, who);

	if (is_king_threatened (board, who, ROW(king_pos), COLUMN(king_pos)))
		return false;

	if (is_castling_move(mv))
    {
	    coord_t castled_pos = MOVE_DST (mv);

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

bool is_drawing_move (const move_tree_t *move_tree, move_t move)
{
    return false;
}

// vi: set ts=4 sw=4:
