#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "board.h"
#include "board_positions.h"
#include "move.h"
#include "piece.h"
#include "material.h"
#include "debug.h"
#include "generate.h"
#include "coord.h"
#include "validate.h"

/* board length in characters */
#define BOARD_LENGTH            31

DEFINE_DEBUG_CHANNEL (board, 0);

static inline void set_piece (struct board *board, coord_t place, piece_t piece)
{
	board->board[ROW(place)][COLUMN(place)] = piece;
}

piece_t handle_en_passant (struct board *board, color_t who, move_t *move,
                           coord_t src, coord_t dst, int undo)
{
    // get the position of the pawn adjacent to the taking pawn
    coord_t taken_pawn_pos = coord_create (ROW(src), COLUMN (dst));

	if (undo)
	{
		piece_t taken = move_get_taken (move);
        set_piece (board, taken_pawn_pos, taken);

		return PIECE_AND_COLOR_NONE; // restore empty square where piece was replaced
	}
	else
	{
        piece_t taken = PIECE_AT_COORD (board, taken_pawn_pos);

        move_set_taken (move, taken);
        set_piece (board, taken_pawn_pos, PIECE_AND_COLOR_NONE);

        return taken;
	}
}

static move_t get_castling_rook_move (struct board *board, move_t *move, color_t who)
{
	unsigned char    src_row, src_col;
	unsigned char    dst_row, dst_col;
	coord_t          src, dst;

	assert (is_castling_move (move));

	src = MOVE_SRC (*move);
	dst = MOVE_DST (*move);

	src_row = ROW (src);
	dst_row = ROW (dst);

	if (COLUMN (src) < COLUMN (dst))
	{
		// castle to the right (kingside)
		src_col = LAST_COLUMN;
		dst_col = COLUMN (dst) - 1;
	}
	else
	{
		// castle to the left (queenside)
		src_col = 0;
		dst_col = COLUMN (dst) + 1;
	}

	if (!((PIECE_TYPE (PIECE_AT (board, src_row, src_col)) == PIECE_ROOK
		|| PIECE_TYPE (PIECE_AT (board, dst_row, dst_col)) == PIECE_ROOK)))
	{
	    printf ("move considering: %s (%s to move)\n", move_str(*move),
             who == COLOR_WHITE ? "White" : "Black");
		board_dump (board);
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

	rook_move = get_castling_rook_move (board, king_move, who);

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
	    king_position_set (board, who, src);

		// retrieve the old board castle status
		if (move_affects_current_castle_state(*move)) {
		    board_undo_castle_change (board, who, current_castle_state(move));
        }
	}
	else
	{
	    king_position_set (board, who, dst);

		// set as not able to castle
		if (able_to_castle (board, who, (CASTLE_KINGSIDE | CASTLE_QUEENSIDE)))
		{
			// save the old castle status
            castle_state_t old_castle_state = board_get_castle_state(board, who);
            save_current_castle_state (move, old_castle_state);

			if (!is_castling_move (move))
				old_castle_state |= CASTLE_KINGSIDE | CASTLE_QUEENSIDE;
			else
				old_castle_state = CASTLE_CASTLED;

			// set the new castle status
			board_apply_castle_change (board, who, old_castle_state);
		}
	}
}

static void update_opponent_rook_position (struct board *board, color_t opponent,
                                           piece_t dst_piece, move_t *move, coord_t src,
                                           coord_t dst, int undo)
{
    assert( PIECE_COLOR(dst_piece) == opponent && PIECE_TYPE(dst_piece) == PIECE_ROOK );

    if (undo)
    {
        // need to put castle status back...its saved in the move
        // from do_move()...
        if (move_affects_opponent_castle_state(*move))
            board_undo_castle_change (board, opponent, opponent_castle_state(move));
    }
    else
    {
        enum castle castle_state;

        //
        // This needs distinguishes between captures that end
        // up on the rook and moves from the rook itself.
        ///
        if (COLUMN(dst) == 0)
            castle_state = CASTLE_QUEENSIDE;
        else
            castle_state = CASTLE_KINGSIDE;

        //
        // Set inability to castle on one side. Note that
        // CASTLE_QUEENSIDE/KINGSIDE are _negative_ flags, indicating the
        // player cannot castle.  This is a bit confusing, not sure why i did
        // this.
        //
        if (able_to_castle (board, opponent, castle_state))
        {
            // save the current castle state
            castle_state_t orig_castle_state = board_get_castle_state (board, opponent);
            save_opponent_castle_state (move, orig_castle_state);

            castle_state |= orig_castle_state;
            board_apply_castle_change (board, opponent, castle_state);
        }
    }
}

static void update_current_rook_position (struct board *board, color_t player,
                                           piece_t src_piece, move_t *move, coord_t src,
                                           coord_t dst,int undo)
{
    if (!( PIECE_COLOR(src_piece) == player && PIECE_TYPE(src_piece) == PIECE_ROOK))
    {
        printf ("update_current_rook_position failed: move %s\n", move_str(*move));
        board_dump (board);
        abort ();
    }

    assert( PIECE_COLOR(src_piece) == player && PIECE_TYPE(src_piece) == PIECE_ROOK );

	if (undo)
	{
		// need to put castle status back...its saved in the move
		// from do_move()...
		if (move_affects_current_castle_state(*move))
            board_undo_castle_change(board, player, current_castle_state(move));
	}
	else
	{
        enum castle castle_state;

        //
        // This needs distinguishes between captures that end
        // up on the rook and moves from the rook itself.
        ///
        if (COLUMN(src) == 0)
            castle_state = CASTLE_QUEENSIDE;
        else
            castle_state = CASTLE_KINGSIDE;

		//
		// Set inability to castle on one side. Note that
		// CASTLE_QUEENSIDE/KINGSIDE are _negative_ flags, indicating the
		// player cannot castle.  This is a bit confusing, not sure why i did
		// this.
		//
		if (able_to_castle (board, player, castle_state))
		{
            // save the current castle state
            castle_state_t orig_castle_state = board_get_castle_state (board, player);
            save_current_castle_state (move, orig_castle_state);

            castle_state |= orig_castle_state;
            board_apply_castle_change (board, player, castle_state);
        }
	}
}

void do_move (struct board *board, color_t who, move_t *move)
{
	piece_t    orig_src_piece, src_piece, dst_piece;
	coord_t    src, dst;

	src = MOVE_SRC (*move);
	dst = MOVE_DST (*move);

	orig_src_piece = src_piece = PIECE_AT_COORD (board, src);
	dst_piece = PIECE_AT_COORD (board, dst);

	/* save the taken piece in the move
	 * This may be a bad idea to set here */
	if (PIECE_TYPE (dst_piece) != PIECE_NONE)
	    move_set_taken (move, dst_piece);

	if (PIECE_TYPE (src_piece) != PIECE_NONE &&
	    PIECE_TYPE (dst_piece) != PIECE_NONE)
	{
		if (PIECE_COLOR (src_piece) == PIECE_COLOR (dst_piece))
		{
			printf ("ERROR: piece tried to take same color\n");
			printf ("src piece; %s\n", piece_str (src_piece));
			printf ("dst piece: %s\n", piece_str (dst_piece));
			printf ("src piece_type: %d, dst_piece_type: %d\n", PIECE_TYPE(src_piece), PIECE_TYPE(dst_piece));
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
        dst_piece = handle_en_passant(board, who, move, src, dst, 0);

	/* check for castling */
	if (unlikely (is_castling_move (move)))
		handle_castling (board, who, move, src, dst, 0);

	set_piece (board, src, PIECE_AND_COLOR_NONE);
	set_piece (board, dst, src_piece);

	/* update king position */
	if (PIECE_TYPE(src_piece) == PIECE_KING)
		update_king_position (board, who, move, src, dst, 0);

	/* update rook position -- for castling */
	if (PIECE_TYPE(orig_src_piece) == PIECE_ROOK)
		update_current_rook_position (board, who, orig_src_piece, move, src, dst, 0);

	if (is_capture_move (move))
	{
		// update material estimate
		material_del (&board->material, dst_piece);

		// update castle state if somebody takes the rook
		if (PIECE_TYPE(dst_piece) == PIECE_ROOK)
			update_opponent_rook_position (board, color_invert(who), dst_piece, move, src, dst, 0);
	}

	validate_castle_state (board, move);
}

void undo_move (struct board *board, color_t who, move_t *move)
{
	piece_t     orig_src_piece, src_piece, dst_piece;
	coord_t     src, dst;

	src = MOVE_SRC(*move);
	dst = MOVE_DST(*move);

	dst_piece = move_get_taken(move);
	orig_src_piece = src_piece = PIECE_AT_COORD (board, dst);

	if (PIECE_TYPE(src_piece) != PIECE_NONE &&
	    PIECE_TYPE(dst_piece) != PIECE_NONE)
	{
		assert( PIECE_COLOR (src_piece) != PIECE_COLOR (dst_piece) );
	}

	/* check for promotion */
	if (unlikely(is_promoting_move (move)))
		src_piece = MAKE_PIECE (PIECE_COLOR (src_piece), PIECE_PAWN);

	/* check for castling */
	if (unlikely(is_castling_move(move)))
		handle_castling (board, who, move, src, dst, 1);

	/* check for en passant */
	if (unlikely(is_en_passant_move(move)))
		dst_piece = handle_en_passant (board, who, move, src, dst, 1);

	/* put the pieces back */
	set_piece (board, dst, dst_piece);
	set_piece (board, src, src_piece);

	/* update king position */
	if (unlikely(PIECE_TYPE(src_piece) == PIECE_KING))
		update_king_position (board, who, move, src, dst, 1);

	if (PIECE_TYPE(orig_src_piece) == PIECE_ROOK)
		update_current_rook_position (board, who, orig_src_piece, move, src, dst, 1);

	if (is_capture_move(move))
	{
        // NOTE: we reload from the move in case of en-passant, since dst_piece could be none.
        material_add (&board->material, move_get_taken(move));

		if (PIECE_TYPE(dst_piece) == PIECE_ROOK)
			update_opponent_rook_position (board, color_invert(who), dst_piece, move, src, dst, 1);
	}
	validate_castle_state (board, move);
}

static enum piece_type back_rank[] =
{
	PIECE_ROOK,   PIECE_KNIGHT, PIECE_BISHOP, PIECE_QUEEN, PIECE_KING,
	PIECE_BISHOP, PIECE_KNIGHT, PIECE_ROOK, PIECE_LAST
};

static enum piece_type pawns[] =
{
	PIECE_PAWN, PIECE_PAWN, PIECE_PAWN, PIECE_PAWN, PIECE_PAWN,
	PIECE_PAWN, PIECE_PAWN, PIECE_PAWN, PIECE_LAST
};

static struct board_positions init_board[] =
{
	{ 0, COLOR_BLACK, back_rank, },
	{ 1, COLOR_BLACK, pawns,     },
	{ 6, COLOR_WHITE, pawns,     },
	{ 7, COLOR_WHITE, back_rank, },
	{ 0, 0, NULL }
};

struct board *board_new (void)
{
	struct board *new_board;
	int i;

	new_board = malloc (sizeof (struct board));
	if (!new_board)
    {
	    abort ();
    }
	assert (new_board);
	memset (new_board, 0, sizeof (struct board));

	for (i = 0; i < NR_PLAYERS; i++)
		new_board->castled[i] = CASTLE_NONE;

    board_init_from_positions (new_board, init_board);

	return new_board;
}

struct board *board_from_positions (struct board_positions *positions)
{
    struct board *new_board;

    new_board = malloc (sizeof(*new_board));
    assert (new_board);
    memset (new_board, 0, sizeof (struct board));

    board_init_from_positions (new_board, init_board);

    return new_board;
}

static castle_state_t init_castle_state (struct board *board, color_t who)
{
    int row = (who == COLOR_WHITE ? 7 : 0);
    int king_col = 4;
    piece_t prospective_king;
    piece_t prospective_queen_rook;
    piece_t prospective_king_rook;

    castle_state_t state = CASTLE_NONE;

    // todo set CASTLED flag if rook/king in right position:
    prospective_king = PIECE_AT (board, row, king_col);
    prospective_queen_rook = PIECE_AT (board, row, 0);
    prospective_king_rook = PIECE_AT (board, row, 7);

    if (PIECE_TYPE(prospective_king) != PIECE_KING ||
        PIECE_COLOR(prospective_king) != who ||
        PIECE_TYPE(prospective_queen_rook) != PIECE_ROOK ||
        PIECE_COLOR(prospective_queen_rook) != who
    ) {
        state |= CASTLE_QUEENSIDE;
    }
    if (PIECE_TYPE(prospective_king) != PIECE_KING ||
        PIECE_COLOR(prospective_king) != who ||
        PIECE_TYPE(prospective_king_rook) != PIECE_ROOK ||
        PIECE_COLOR(prospective_king_rook) != who
    ) {
        state |= CASTLE_KINGSIDE;
    }

    return state;
}


void board_init_from_positions (struct board *board, struct board_positions *positions)
{
    struct board_positions *ptr;
    uint8_t row, col;

    for_each_position (row, col)
        board->board[row][col] = PIECE_AND_COLOR_NONE;

    material_init (&board->material);

    for (ptr = positions; ptr->pieces != NULL; ptr++)
    {
        enum piece_type *pptr = ptr->pieces;
        color_t  color = ptr->piece_color;
        int      row   = ptr->rank;

        for (uint8_t col = 0; col < NR_COLUMNS && pptr[col] != PIECE_LAST; col++)
        {
            piece_t new_piece;

            if (pptr[col] == PIECE_NONE)
                continue;

            new_piece = MAKE_PIECE (color, pptr[col]);
            set_piece (board, coord_create (row, col), new_piece);
            material_add (&board->material, new_piece);

            if (pptr[col] == PIECE_KING)
            {
                king_position_set (board, color, coord_create (row, col));
            }
        }
    }

    board->castled[COLOR_INDEX_WHITE] = init_castle_state (board, COLOR_WHITE);
    board->castled[COLOR_INDEX_BLACK] = init_castle_state (board, COLOR_BLACK);

	board_hash_init (&board->hash, board);
}

void board_free (struct board *board)
{
	if (!board)
		return;

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

	fflush (file);
}

void board_print (struct board *board)
{
	board_print_to_file (board, stdout);
}

void board_print_castle_state (struct board *board)
{
    printf("castle_state: White: %02x Black: %02x\n",
           board->castled[COLOR_INDEX_WHITE], board->castled[COLOR_INDEX_BLACK]);
}

void board_print_material (struct board *board)
{
    printf("material score: White: %d Black: %d\n",
           board->material.score[COLOR_INDEX_WHITE],
           board->material.score[COLOR_INDEX_BLACK]);
}

void board_dump (struct board *board)
{
    board_print_castle_state (board);
    board_print_material (board);
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
	enum piece_type p;

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