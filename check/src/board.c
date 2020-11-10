#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "board.h"
#include "debug.h"
#include "validate.h"

// board length in characters
#define BOARD_LENGTH            31

DEFINE_DEBUG_CHANNEL (board, 0);

static void board_init_from_positions (struct board *board, const struct board_positions *positions);


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
    return board_from_positions (init_board);
}

struct board *board_from_positions (const struct board_positions *positions)
{
    struct board *new_board;
    size_t i;

    new_board = malloc (sizeof(*new_board));
    assert (new_board);
    memset (new_board, 0, sizeof(*new_board));

    for (i = 0; i < NR_PLAYERS; i++)
        new_board->castled[i] = CASTLE_NONE;

    board_init_from_positions (new_board, positions);

    return new_board;
}

static castle_state_t init_castle_state (struct board *board, enum color who)
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

void board_init_from_positions (struct board *board, const struct board_positions *positions)
{
    const struct board_positions *ptr;
    uint8_t row, col;

    for_each_position (row, col)
        board->board[row][col] = PIECE_AND_COLOR_NONE;

    material_init (&board->material);
    position_init (&board->position);
    
    for (ptr = positions; ptr->pieces != NULL; ptr++)
    {
        enum piece_type *pieces = ptr->pieces;
        enum color color = ptr->piece_color;
        row = ptr->rank;

        for (col = 0; col < NR_COLUMNS && pieces[col] != PIECE_LAST; col++)
        {
            piece_t new_piece;

            if (pieces[col] == PIECE_NONE)
                continue;

            new_piece = MAKE_PIECE (color, pieces[col]);
            coord_t place = coord_create (row, col);
            board_set_piece (board, place, new_piece);

            material_add (&board->material, new_piece);
            position_add (&board->position, color, place, new_piece);

            if (pieces[col] == PIECE_KING)
                king_position_set (board, color, place);
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

// for printing the board from gdb
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

// vi: set ts=4 sw=4:
