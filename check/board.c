#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "board.h"
#include "move.h"
#include "piece.h"
#include "material.h"
#include "debug.h"

/* board length in characters */
#define BOARD_LENGTH            31

DEFINE_DEBUG_CHANNEL (board, 0);

static inline void set_piece (struct board *board, coord_t place, piece_t piece)
{
	board->board[ROW(place)][COLUMN(place)] = piece;
}

void handle_en_passant (struct board *board, color_t who, move_t *move, 
                        coord_t src, coord_t dst, int undo)
{
	coord_t   taken_pos;
	piece_t   piece;

	/* get the position of the pawn adjacent to the taking pawn */
	taken_pos = coord_create (ROW (src), COLUMN (dst));

	/* get rid of the the pawn */
	if (undo)
	{
		piece = move_get_taken (move);
#if 0
		printf ("board before undo:\n");
		board_print (board);
#endif
		set_piece (board, taken_pos, piece);

#if 0
		printf ("undo_move: move %s\n", move_str (*move));
		board_print (board);
#endif
	}
	else
	{
		move_set_taken (move, PIECE_AT_COORD (board, taken_pos));
#if 0
		printf ("board before move:\n");
		board_print (board);
#endif
		set_piece (board, taken_pos, PIECE_NONE);

#if 0
		printf ("do_move: move %s\n", move_str (*move));
		board_print (board);
#endif
	}
}

static move_t get_castling_rook_move (struct board *board, move_t *move)
{
	unsigned char    src_row, src_col;
	unsigned char    dst_row, dst_col;
	coord_t          src, dst;

	assert (is_castling_move (move));

	src = MOVE_SRC (*move);
	dst = MOVE_DST (*move);

	src_row = ROW (src);
	dst_row = ROW (src);
	
	if (COLUMN (src) < COLUMN (dst))
	{
		/* castle to the right (kingside) */
#if 0
		printf ("kingside castle\n");
#endif
		src_col = LAST_COLUMN;
		dst_col = COLUMN (dst) - 1;
	}
	else
	{
#if 0
		/* castle to the left (queenside) */
		printf ("queenside castle\n");
#endif
		src_col = 0;
		dst_col = COLUMN (dst) + 1;
	}

	if (!((PIECE_TYPE (PIECE_AT (board, src_row, src_col)) == PIECE_ROOK
		|| PIECE_TYPE (PIECE_AT (board, dst_row, dst_col)) == PIECE_ROOK)))
	{
		board_print (board);
		assert (0);
	}

	return move_create (src_row, src_col, dst_row, dst_col);
}

static void handle_castling (struct board *board, color_t who, 
                             move_t *king_move, coord_t src, coord_t dst, 
                             int undo)
{
	move_t  rook_move;
	coord_t rook_src, rook_dst;
	piece_t rook, empty_piece;

	rook_move = get_castling_rook_move (board, king_move);
	
#if 0
	printf ("src row = %d, src col = %d, dst row = %d, dst col = %d\n",
			ROW (src), COLUMN (src), ROW (dst), COLUMN (dst));

	printf ("rook move = %s\n", move_str (rook_move));
	printf ("king move = %s\n", move_str (*king_move));
	printf ("piece = %s\n", piece_str (PIECE_AT_COORD (board, src)));
#endif

	if (undo)
		assert (PIECE_TYPE (PIECE_AT_COORD (board, dst)) == PIECE_KING);
	else
		assert (PIECE_TYPE (PIECE_AT_COORD (board, src)) == PIECE_KING);

	assert (abs (COLUMN (src) - COLUMN (dst)) == 2);

	rook_src = MOVE_SRC (rook_move);
	rook_dst = MOVE_DST (rook_move);

	empty_piece = MAKE_PIECE (COLOR_NONE, PIECE_NONE);

	if (undo)
	{
		/* undo the rook move */
		rook = PIECE_AT_COORD (board, rook_dst);

		/* undo the rook move */
		set_piece (board, rook_dst, empty_piece);
		set_piece (board, rook_src, rook);
	}
	else
	{
		rook = PIECE_AT_COORD (board, rook_src);

		/* do the rook move */
		set_piece (board, rook_dst, rook);
		set_piece (board, rook_src, empty_piece);
	}
}

void update_king_position (struct board *board, color_t who, move_t *move,
                           coord_t src, coord_t dst, int undo)
{
	if (undo)
	{
		board->king_pos[who] = src;

#if 0
		/* retrive the old board castle status */
		board->castled[who] = move_get_castle_state (move);
#endif
	}
	else
	{
		board->king_pos[who] = dst;

		/* set as not able to castle */
		if (able_to_castle (board, who, (CASTLE_KINGSIDE | CASTLE_QUEENSIDE)))
		{
			enum castle castle_status;

#if 0
			/* save the old castle status */
			move_set_castle_state (move, board->castled[who]);
#endif

			if (!is_castling_move (move))
				castle_status = CASTLE_KINGSIDE | CASTLE_QUEENSIDE;
			else
				castle_status = CASTLE_CASTLED;

			/* set the new castle status */
			board->castled[who] = (castle_status);
		}
	}
}

static void update_rook_position (struct board *board, color_t who, 
								  move_t *move, coord_t src, coord_t dst, 
								  int undo)
{
	enum castle castle_state;

	/* 
	 * Ugh, this needs to distinguish between captures that end
	 * up on the rook and moves from the rook itself.
	 */
	if (!is_capture_move (move))
	{
		if (COLUMN(src) == 0)
			castle_state = CASTLE_QUEENSIDE;
		else
			castle_state = CASTLE_KINGSIDE;
	}
	else
	{
		if (COLUMN(dst) == 0)
			castle_state = CASTLE_QUEENSIDE;
		else
			castle_state = CASTLE_KINGSIDE;
	}

	if (undo)
	{
		/* need to put castle status back...its saved in the move
		 * from do_move()... */
	}
	else
	{
		/* 
		 * Set inability to castle on one side. Note that
		 * CASTLE_QUEENSIDE/KINGSIDE are _negative_ flags, indicating the
		 * player cannot castle.  This is a bit confusing, not sure why i did
		 * this.
		 */
		if (able_to_castle (board, who, castle_state))
			board->castled[who] |= castle_state;
	}
}

void do_move (struct board *board, color_t who, move_t *move)
{
	piece_t    src_piece, dst_piece;
	coord_t    src, dst;

	src = MOVE_SRC (*move);
	dst = MOVE_DST (*move);

#if 0
	if (!strcasecmp (move_str (*move), "O-O-O") && who == COLOR_BLACK)
		printf ("do_move: black is considering castling\n");
#endif

	src_piece = PIECE_AT_COORD (board, src);
	dst_piece = PIECE_AT_COORD (board, dst);

	/* save the taken piece in the move 
	 * This may be a bad idea to set here */
	move_set_taken (move, dst_piece);

	/* save the current castle state */
	move_set_castle_state (move, board->castled[who]);

	if (PIECE_TYPE (src_piece) != PIECE_NONE && 
	    PIECE_TYPE (dst_piece) != PIECE_NONE)
	{
		if (PIECE_COLOR (src_piece) == PIECE_COLOR (dst_piece))
		{
			printf ("ERROR: piece tried to take same color\n");
			printf ("src piece; %s\n", piece_str (src_piece));
			printf ("dst piece: %s\n", piece_str (dst_piece));
			printf ("src piece: %d, dst_piece: %d\n", src_piece, dst_piece);
			printf ("move was [%s]\n", move_str (*move));
			printf ("src color: %d, dst color: %d\n", PIECE_COLOR (src_piece),
			        PIECE_COLOR (dst_piece));
			assert (0);
		}
	}

	/* check for promotion */
	if (unlikely (is_promoting_move (move)))
		src_piece = move_get_promoted (move);

	/* check for en passant */
	if (unlikely (is_en_passant_move (move)))
		handle_en_passant (board, who, move, src, dst, 0);

	/* check for castling */
	if (unlikely (is_castling_move (move)))
		handle_castling (board, who, move, src, dst, 0);

	set_piece (board, src, PIECE_NONE);
	set_piece (board, dst, src_piece);

	/* update king position */
	if (PIECE_TYPE (src_piece) == PIECE_KING)
		update_king_position (board, who, move, src, dst, 0);

	/* update rook position -- for castling */
	if (PIECE_TYPE (src_piece) == PIECE_ROOK)
		update_rook_position (board, who, move, src, dst, 0);

	if (is_capture_move (move))
	{
		/* update material estimate */
		material_del (board->material, dst_piece);

		/* update castle state if somebody takes the rook */
		if (PIECE_TYPE (dst_piece) == PIECE_ROOK)
			update_rook_position (board, color_invert (who), move, src, dst, 0);
	}
}

void undo_move (struct board *board, color_t who, move_t *move)
{
	piece_t     src_piece, dst_piece;
	coord_t     src, dst;

	src = MOVE_SRC (*move);
	dst = MOVE_DST (*move);

	dst_piece = move_get_taken (move);
	src_piece = PIECE_AT_COORD (board, dst);

	if (PIECE_TYPE (src_piece) != PIECE_NONE && 
	    PIECE_TYPE (dst_piece) != PIECE_NONE)
	{
		assert (PIECE_COLOR (src_piece) != PIECE_COLOR (dst_piece));
	}

	/* check for promotion */
	if (unlikely (is_promoting_move (move)))
		src_piece = MAKE_PIECE (PIECE_COLOR (src_piece), PIECE_PAWN);

	/* check for castling */
	if (unlikely (is_castling_move (move)))
		handle_castling (board, who, move, src, dst, 1);
		
	/* check for en passant */
	if (unlikely (is_en_passant_move (move)))
	{
		handle_en_passant (board, who, move, src, dst, 1);
		dst_piece = MAKE_PIECE (COLOR_NONE, PIECE_NONE);
	}

	/* put the pieces back */
	set_piece (board, dst, dst_piece);
	set_piece (board, src, src_piece);

	/* update king position */
	if (unlikely (PIECE_TYPE (src_piece) == PIECE_KING))
		update_king_position (board, who, move, src, dst, 1);

	if (PIECE_TYPE (src_piece) == PIECE_ROOK)
		update_rook_position (board, who, move, src, dst, 1);

	if (is_capture_move (move))
	{
		material_add (board->material, dst_piece);

		if (PIECE_TYPE (dst_piece) == PIECE_ROOK)
			update_rook_position (board, color_invert (who), move, src, dst, 1);
	}

	if (unlikely (is_en_passant_move (move)))
	{
#if 0
		printf ("after undo_move entirely:\n");
		board_print (board);
#endif
	}

	/* restore the previous castled state */
	board->castled[who] = move_get_castle_state (move);
}

static piece_t back_rank[] =
{
	PIECE_ROOK,   PIECE_KNIGHT, PIECE_BISHOP, PIECE_QUEEN, PIECE_KING,
	PIECE_BISHOP, PIECE_KNIGHT, PIECE_ROOK, PIECE_LAST
};

static piece_t pawns[] =
{
	PIECE_PAWN, PIECE_PAWN, PIECE_PAWN, PIECE_PAWN, PIECE_PAWN, 
	PIECE_PAWN, PIECE_PAWN, PIECE_PAWN, PIECE_LAST
};

static struct init_board 
{
	int       rank;
	color_t   piece_color;
	piece_t  *pieces;
} init_board[] =
{
	{ 0, COLOR_BLACK, back_rank, },
	{ 1, COLOR_BLACK, pawns,     },
	{ 6, COLOR_WHITE, pawns,     },
	{ 7, COLOR_WHITE, back_rank, },
	{ 0, 0, NULL }
};

struct board *board_new ()
{
	struct board *new_board;
	struct init_board *ptr;
	int i;

	new_board = malloc (sizeof (struct board));
	assert (new_board);
	memset (new_board, 0, sizeof (struct board));

	new_board->material = material_new ();
	assert (new_board->material != NULL);

	for (ptr = init_board; ptr->pieces; ptr++)
	{
		piece_t *pptr;
		color_t  color = ptr->piece_color;
		int      row   = ptr->rank;

		for (pptr = ptr->pieces; *pptr != PIECE_LAST; pptr++)
		{
			int     col       = pptr - ptr->pieces;
			piece_t new_piece;

			new_piece = MAKE_PIECE (color, *pptr);

			set_piece (new_board, coord_create (row, col), new_piece);
			material_add (new_board->material, new_piece);

			if (PIECE_TYPE (*pptr) == PIECE_KING)
			{
				new_board->king_pos[color] = coord_create (row, col);
			}
		}
	}

	for (i = 0; i < NR_PLAYERS; i++)
		new_board->castled[i] = CASTLE_NOTCASTLED;

	return new_board;
}

void board_free (struct board *board)
{
	if (!board)
		return;

	material_free (board->material);

	/* 
	 * 2003-08-30: I have forgotten if this is all that needs freeing..
	 */
	free (board);
}

static void print_divider (FILE *file)
{
	int col;
	int i;

	fprintf (file, " ");

	for (col = 0; col < BOARD_LENGTH; col += 4)
	{
		for (i = 0; i < 3; i++)
			putc ('-', file);
		putc (' ', file);
	}

	fprintf (file, "\n");
}

void board_print_to_file (struct board *board, FILE *file)
{
	int row, col;

	print_divider (file);

	for (row = 0; row < NR_ROWS; row++)
	{
		for (col = 0; col < NR_COLUMNS; col++)
		{
			piece_t  piece = PIECE_AT (board, row, col);

			if (!col)
				fprintf (file, "|");

			if (PIECE_TYPE (piece) != PIECE_NONE && 
				PIECE_COLOR (piece) == COLOR_BLACK)
			{
				fprintf (file, "*");
			}
			else
			{
				fprintf (file, " ");
			}

			switch (PIECE_TYPE (piece))
			{
			 case PIECE_PAWN:    fprintf (file, "p"); break;
			 case PIECE_KNIGHT:  fprintf (file, "N"); break;
			 case PIECE_BISHOP:  fprintf (file, "B"); break;
			 case PIECE_ROOK:    fprintf (file, "R"); break;
			 case PIECE_QUEEN:   fprintf (file, "Q"); break;
			 case PIECE_KING:    fprintf (file, "K"); break;
			 case PIECE_NONE:    fprintf (file, " "); break;
			 default:            assert (0);  break;
			}

			fprintf (file, " |");
		}

		fprintf (file, "\n");

		print_divider (file);
	}
}

void board_print (struct board *board)
{
	board_print_to_file (board, stdout);
}

/* for printing the board from gdb */
void board_print_err (struct board *board)
{
	board_print_to_file (board, stderr);
}

void check (void)
{
	enum color c; 
	enum piece p;

	for_each_color (c)
	{
		for_each_piece (p)
		{
			assert (PIECE_COLOR (MAKE_PIECE (c, p)) == c);
			assert (PIECE_TYPE (MAKE_PIECE (c, p)) == p);
		}
	}
}

/* vi: set ts=4 sw=4: */
