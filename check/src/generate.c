#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "piece.h"
#include "move.h"
#include "move_list.h"
#include "move_tree.h"
#include "board.h"
#include "check.h"
#include "generate.h"
#include "debug.h"
#include "board_check.h"

///////////////////////////////////////////////

DEFINE_DEBUG_CHANNEL (generate, 0);

///////////////////////////////////////////////

#define Move_Func_Arguments \
	struct board *board, color_t who, \
	int piece_row, int piece_col, move_tree_t *history, move_list_t *moves

#define Move_Func_Param_Names \
	board, who, piece_row, piece_col, history, moves

#define MOVES_HANDLER(name) \
	static move_list_t *moves_##name (Move_Func_Arguments)

///////////////////////////////////////////////

typedef move_list_t *(*MoveFunc) (Move_Func_Arguments);

MOVES_HANDLER (none);
MOVES_HANDLER (king);
MOVES_HANDLER (queen);
MOVES_HANDLER (rook);
MOVES_HANDLER (bishop);
MOVES_HANDLER (knight);
MOVES_HANDLER (pawn);
MOVES_HANDLER (en_passant);

static void knight_move_list_init (void);

///////////////////////////////////////////////

static MoveFunc move_functions[] = 
{
	moves_none,    // PIECE_NONE   [0]
	moves_king,    // PIECE_KING   [1]
	moves_queen,   // PIECE_QUEEN  [2]
	moves_rook,    // PIECE_ROOK   [3]
	moves_bishop,  // PIECE_BISHOP [4]
	moves_knight,  // PIECE_KNIGHT [5]
	moves_pawn,    // PIECE_PAWN   [6]
	NULL,
};

static move_list_t *knight_moves[NR_ROWS][NR_COLUMNS];

///////////////////////////////////////////////

/* generate a lookup table for knight moves */
static void knight_move_list_init (void)
{
	int row, col;
	int k_row, k_col;

	for_each_position (row, col)
	{
		for (k_row = -2; k_row <= 2; k_row++)
		{
			if (!k_row)
				continue;

			if (INVALID (k_row + row))
				continue;

			for (k_col = 3-abs(k_row); k_col >= -2; k_col -= 2*abs(k_col))
			{
				if (INVALID (k_col + col))
					continue;

				knight_moves[k_row+row][k_col+col] = 
				   move_list_append (knight_moves[k_row+row][k_col+col], 
				                     k_row+row, k_col+col, row, col);
			}
		}
	}
}

MOVES_HANDLER (none)
{
	assert (0);

	return moves;
}

MOVES_HANDLER (king)
{
	int    row, col;
	move_t castled_moves[2];
	
	for (row = piece_row-1; row < 8 && row <= piece_row+1; row++)
	{
		if (INVALID (row))
			continue;

		for (col = piece_col-1; col < 8 && col <= piece_col+1; col++)
		{
			if (INVALID (col))
				continue;

			moves = move_list_append (moves, piece_row, piece_col, row, col);
		}
	}
			
	// castling
	if (able_to_castle (board, who, CASTLE_KINGSIDE | CASTLE_QUEENSIDE))
	{
		int i;

		for (i = 0; i < sizeof (castled_moves)/sizeof (move_t); i++)
			move_nullify (&castled_moves[i]);

		if (able_to_castle (board, who, CASTLE_QUEENSIDE))
		{
			castled_moves[0] = move_create (piece_row, piece_col, piece_row,
			                                piece_col - 2);
		}

		if (able_to_castle (board, who, CASTLE_KINGSIDE))
		{
			castled_moves[1] = move_create (piece_row, piece_col, piece_row,
			                                piece_col + 2);
		}

		for (i = 0; i < sizeof (castled_moves)/sizeof (move_t); i++)
		{
				if (!is_null_move (castled_moves[i]))
				{
					move_set_castling (&castled_moves[i]);
					moves = move_list_append_move (moves, castled_moves[i]);
				}
		}
	}

	return moves;
}

MOVES_HANDLER (queen)
{
	// use the generators for bishop and rook
	moves = moves_bishop (Move_Func_Param_Names);
	moves = moves_rook   (Move_Func_Param_Names);

	return moves;
}

MOVES_HANDLER (rook)
{
	int          dir;
	int          row,   col;
	piece_t      piece;

	for (dir = -1; dir <= 1; dir += 2)
	{
		for (row = NEXT (piece_row, dir); VALID (row); row = NEXT (row, dir))
		{
			piece = PIECE_AT (board, row, piece_col);

			moves = move_list_append (moves, piece_row, piece_col, row, 
			                          piece_col);

			if (PIECE_TYPE (piece) != PIECE_NONE)
				break;
		}

		for (col = NEXT (piece_col, dir); VALID (col); col = NEXT (col, dir))
		{
			piece = PIECE_AT (board, piece_row, col);

			moves = move_list_append (moves, piece_row, piece_col, piece_row, 
			                          col);

			if (PIECE_TYPE (piece) != PIECE_NONE)
				break;
		}
	}

	return moves;
}

MOVES_HANDLER (bishop)
{
	int          r_dir, c_dir;
	int          row,   col;

	for (r_dir = -1; r_dir <= 1; r_dir += 2)
	{
		for (c_dir = -1; c_dir <= 1; c_dir += 2)
		{
			for (row = NEXT (piece_row, r_dir), col = NEXT (piece_col, c_dir);
			     VALID (row) && VALID (col);
			     row = NEXT (row, r_dir), col = NEXT (col, c_dir))
			{
				piece_t    piece = PIECE_AT (board, row, col);
				
				moves = move_list_append (moves, piece_row, piece_col,
				                          row, col);

				if (PIECE_TYPE (piece) != PIECE_NONE)
					break;
			}
		}
	}

	return moves;
}

MOVES_HANDLER (knight)
{
	move_list_t  *kt_moves;
	move_t       *move;

	kt_moves = generate_knight_moves (piece_row, piece_col);

	assert (kt_moves);

	DBG (generate, "length of knight move list = %d\n", kt_moves->len);

	for_each_move (move, kt_moves)
		moves = move_list_append_move (moves, *move);

	return moves;
}

MOVES_HANDLER (pawn)
{
	int                dir;
	int                row;
	int                take_col;
	int                c_dir;
	move_t             move[4];        /* 4 possible pawn moves */
	enum piece_type    piece_type;
	piece_t            piece;
	int                i;

	dir = PAWN_DIRECTION (who);

	// row is _guaranteed_ to be on the board, because
	// a pawn on the eight rank can't remain a pawn, and that's
	// the only direction moved in
	assert (VALID (piece_row));

	row = NEXT (piece_row, dir);

	memset (move, 0, sizeof (move));

	/* single move */
	if (PIECE_TYPE (PIECE_AT (board, row, piece_col)) == PIECE_NONE)
		move[0] = move_create (piece_row, piece_col, row, piece_col);

	/* double move */
	if (is_pawn_unmoved (board, piece_row, piece_col))
	{
		int next_row = NEXT (row, dir);

		if (!is_null_move (move[0]) &&
			PIECE_TYPE (PIECE_AT (board, next_row, piece_col)) == PIECE_NONE)
		{
			move[1] = move_create (piece_row, piece_col, next_row, piece_col);
		}
	}

	// take pieces
	for (c_dir = -1; c_dir <= 1; c_dir += 2)
	{
		take_col = NEXT (piece_col, c_dir);

		if (INVALID (take_col))
			continue;

		piece = PIECE_AT (board, row, take_col);

		if (PIECE_TYPE (PIECE_AT (board, row, take_col)) != PIECE_NONE && 
		    PIECE_COLOR (piece) != who)
		{
			/* Hmm, it would be nice to do this without a branch... */
			if (c_dir == -1)
				move[2] = move_create (piece_row, piece_col, row, take_col);
			else
				move[3] = move_create (piece_row, piece_col, row, take_col);
		}
	}

	// promotion
	if (unlikely (need_pawn_promotion (row, who)))
	{
		for (piece_type = PIECE_QUEEN; piece_type < PIECE_PAWN; piece_type++)
		{
			piece = MAKE_PIECE (who, piece_type);

			// promotion moves dont include en passant
			for (i = 0; i < 4; i++)
			{
				if (!is_null_move (move[i]))
				{
					move[i] = move_promote (move[i], piece);

					moves = move_list_append_move (moves, move[i]); 
				}
			}
		}

		return moves;
	}
	
	// en passant
	if (unlikely (may_do_en_passant (piece_row, who)))
		moves = moves_en_passant (Move_Func_Param_Names);

	for (i = 0; i < sizeof (move) / sizeof (move[0]); i++)
		if (!is_null_move (move[i]))
			moves = move_list_append_move (moves, move[i]);

	return moves;
}

// put en passant in a separate handler
// in order to not pollute instruction cache with it
MOVES_HANDLER (en_passant)
{
	coord_t       src, dst;
	move_t        mv;
	move_t        new_move;
	int           direction;
	piece_t       piece;
	unsigned char take_row, take_col;
	unsigned char start_row;

	direction = PAWN_DIRECTION (who);

	if (unlikely (!history))
		return moves;

	mv = history->move;

	dst = MOVE_DST (mv);
	src = MOVE_SRC (mv);

	start_row = ROW (src);

	take_col  = COLUMN (dst);
	take_row  = ROW (dst);

	if (abs (start_row - take_row) != 2)
		return moves;

	if (piece_row != take_row)
		return moves;

	if (abs (take_col - piece_col) != 1)
		return moves;

	piece = PIECE_AT (board, take_row, take_col);

	if (PIECE_TYPE (piece) != PIECE_PAWN)
		return moves;

	assert (PIECE_COLOR (piece) != who);

	/* ok, looks like the previous move could yield en passant */
	take_row = NEXT (piece_row, direction);

	new_move = move_create (piece_row, piece_col, take_row, take_col);

	move_set_en_passant (&new_move);

	move_set_taken (&new_move, MAKE_PIECE(color_invert(who), PIECE_PAWN));

	moves = move_list_append_move (moves, new_move);
	
	return moves;
}

///////////////////////////////////////////////

move_list_t *generate_knight_moves (unsigned char row, unsigned char col)
{
	/* Check f3 */
	if (unlikely (!knight_moves[0][0]))
		knight_move_list_init ();

	return knight_moves[row][col];
}

static char *piece_type_str (piece_t piece)
{
	switch (PIECE_TYPE (piece))
	{
	 case PIECE_KING: return "king";
	 case PIECE_QUEEN: return "queen";
	 case PIECE_ROOK: return "rook";
	 case PIECE_BISHOP: return "bishop";
	 case PIECE_KNIGHT: return "knight";
	 case PIECE_PAWN: return "pawn";
	 case PIECE_NONE: return "<none>";
	 default: assert(0);
	}
}

void print_the_board (struct board *board)
{
	int row, col;

	debug_multi_line_start (&CHANNEL_NAME (generate));

	for_each_position (row, col)
	{
//		DBG (generate, "%2d ", PIECE_AT (board, row, col));
		if (col == 7)
			DBG (generate, "\n");
	}

	debug_multi_line_stop (&CHANNEL_NAME (generate));
}

move_list_t *generate_legal_moves (struct board *board, color_t who,
                                   move_tree_t *history)
{
	move_list_t *all_moves  = NULL;
	move_list_t *non_checks = NULL;
	move_t      *mptr;
    board_check_t board_check;
	all_moves = generate_moves (board, who, history);

	for_each_move (mptr, all_moves)
	{
	    board_check_init (&board_check, board);
		do_move (board, who, mptr);

        if (was_legal_move (board, who, mptr))
        {
            non_checks = move_list_append_move (non_checks, *mptr);
        }

		undo_move (board, who, mptr);
        board_check_validate (&board_check, board, who, mptr);
	}

	move_list_destroy (all_moves);
	
	return non_checks;
}

static int valid_castling_move (struct board *board, color_t who, move_t *move)
{
	// check for an intervening piece
	int     direction;
	coord_t src, dst;
	piece_t piece1, piece2, piece3;

	src = MOVE_SRC (*move);
	dst = MOVE_DST (*move);

	piece3 = MAKE_PIECE (COLOR_NONE, PIECE_NONE);

	/* find which direction the king was castling in */
	direction = (COLUMN (dst) - COLUMN (src)) / 2;

	piece1 = PIECE_AT (board, ROW (src), COLUMN (dst) - direction);
	piece2 = PIECE_AT (board, ROW (src), COLUMN (dst));
	
	if (direction < 0)
	{
		// check for piece next to rook on queenside
		piece3 = PIECE_AT (board, ROW (src), COLUMN (dst) - 1);
	}

	return PIECE_TYPE (piece1) == PIECE_NONE && 
	       PIECE_TYPE (piece2) == PIECE_NONE &&
		   PIECE_TYPE (piece3) == PIECE_NONE;
}

move_list_t *validate_moves (move_list_t *move_list, struct board *board,
                             color_t who)
{
	move_t      *move;
	move_list_t *captures = NULL, *non_captures = NULL;

	move_list_print (move_list);

	for_each_move (move, move_list)
	{
		coord_t src, dst;
		piece_t src_piece, dst_piece;
		int     is_capture = 0;

		src = MOVE_SRC (*move);
		dst = MOVE_DST (*move);

		src_piece = PIECE_AT_COORD (board, src);
		dst_piece = PIECE_AT_COORD (board, dst);

		assert (PIECE_TYPE (src_piece) != PIECE_NONE);

		is_capture = (PIECE_TYPE (dst_piece) != PIECE_NONE);

		if (unlikely (is_en_passant_move (move)))
			is_capture = 1;

		if (unlikely (is_castling_move (move)))
			if (!valid_castling_move (board, who, move))
				continue;

		if (PIECE_COLOR (src_piece) == PIECE_COLOR (dst_piece) && 
		    PIECE_TYPE (dst_piece) != PIECE_NONE &&
			is_capture)
		{
			assert (is_capture);

			continue;
		}

		// check for an illegal king capture
		if (PIECE_TYPE (dst_piece) == PIECE_KING)
		{
			assert (is_capture);

			move_list_destroy (captures);
			move_list_destroy (non_captures);
			move_list_destroy (move_list);

			return NULL;
		}

		if (is_capture)
		{
			if (!is_capture_move (move))
				move_set_taken (move, dst_piece);

			captures = move_list_append_move (captures, *move);
		}
		else
		{
#if 0
			non_captures = move_list_append_move (non_captures, *move);	
#endif
			captures = move_list_append_move (captures, *move);	
		}

		if (is_promoting_move (move))
		{
			DBG (generate, "PROMOTION MOVE: %s -> %s", move_str (*move), 
			     piece_str (move_get_promoted (move)));
		}
	}

	move_list_destroy (move_list);

#if 0
	captures = move_list_append_list (captures, non_captures);

	move_list_destroy (non_captures);
#endif

#if 0
	if (unlikely (is_king_threatened (board, who, ROW (board->king_pos[who]),
		                              COLUMN (board->king_pos[who]))))
	{
		captures = remove_checks (captures, board, who);
	}
#endif

	move_list_print (captures);

	return captures;
}

move_list_t *generate_captures (struct board *board, color_t who,
                                move_tree_t *history)
{
	move_list_t *move_list = NULL;
	move_list_t *captures  = NULL;
	move_t      *mv;

	move_list = generate_moves (board, who, history);

	for_each_move (mv, move_list)
		if (is_capture_move (mv))
			captures = move_list_append_move (captures, *mv);

	move_list_destroy (move_list);

	return captures;
}

move_list_t *generate_moves (struct board *board, color_t who,
                             move_tree_t *history)
{
	int          row, col;
	move_list_t *new_moves    = NULL;
	
	DBG (generate, "calling print_board\n");
	DBG (generate, "done calling print_board\n");

	for_each_position (row, col)
	{
		piece_t           piece     = PIECE_AT (board, row, col);
		color_t           color     = PIECE_COLOR (piece);

		if (PIECE_TYPE (piece) == PIECE_NONE)
			continue;

		if (color != who)
			continue;

		assert (PIECE_TYPE (piece) < PIECE_LAST);

		color = PIECE_COLOR (piece);

		if (PIECE_TYPE (piece) != PIECE_NONE)
		{
			DBG (generate, "piece color[%d][%d] = %s [%s]\n", row, col, 
			     color == COLOR_WHITE ?  "white" : "black",
			     piece_type_str (piece));
		}

		new_moves = (*move_functions[PIECE_TYPE (piece)])
			(board, who, row, col, history, new_moves);

#if 0
		for_each_move (move, new_moves)
			if (is_legal_move (board, who, move))
				moves = move_list_append_move (moves, *move);

#endif
	}

	return validate_moves (new_moves, board, who);
}

// vim: set ts=4 sw=4:
