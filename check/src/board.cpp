#include <cstdio>
#include <cstdlib>
#include <cassert>

#include "board.h"
#include "debug.h"
#include "validate.h"

// board length in characters
constexpr unsigned int BOARD_LENGTH = 31;

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
	{ 0, COLOR_NONE, nullptr }
};

board::board()
{
    board_init_from_positions (this, init_board);
}

board::board (const struct board_positions *positions)
{
    board_init_from_positions (this, positions);
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

    for (uint8_t & i : board->castled)
        i = CASTLE_NONE;

    for (auto& piece : *board)
        piece = PIECE_AND_COLOR_NONE;

    material_init (&board->material);
    position_init (&board->position);
    
    for (ptr = positions; ptr->pieces != nullptr; ptr++)
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

    board->en_passant_target[COLOR_INDEX_WHITE] = no_en_passant_coord;
    board->en_passant_target[COLOR_INDEX_BLACK] = no_en_passant_coord;

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
	uint8_t col;
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
	uint8_t row, col;

	print_divider (file);

	for (row = 0; row < NR_ROWS; row++)
	{
		for (col = 0; col < NR_COLUMNS; col++)
		{
			piece_t  piece = PIECE_AT (board, row, col);

			if (!col)
				fprintf (file, "|");

			if (PIECE_TYPE(piece) != PIECE_NONE &&
				PIECE_COLOR(piece) == COLOR_BLACK)
			{
				fprintf (file, "*");
			}
			else
			{
				fprintf (file, " ");
			}

			switch (PIECE_TYPE(piece))
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

board_iterator board::begin()
{
    return board_iterator { this, 0, 0 };
}

board_iterator board::end()
{
    return board_iterator { this, NR_ROWS, 0 };
}

void add_divider (std::string &result)
{
    uint8_t col;
    int i;

    result += " ";

    for (col = 0; col < BOARD_LENGTH; col += 4)
    {
        for (i = 0; i < 3; i++)
            result += '-';
        result += ' ';
    }

    result += "\n";
}

std::string board::to_string()
{
    std::string result;
    uint8_t row, col;

    add_divider (result);
    for (row = 0; row < NR_ROWS; row++)
    {
        for (col = 0; col < NR_COLUMNS; col++)
        {
            piece_t piece = PIECE_AT (this, row, col);

            if (!col)
                result += "|";

            if (PIECE_TYPE(piece) != PIECE_NONE &&
                PIECE_COLOR(piece) == COLOR_BLACK)
            {
                result += "*";
            }
            else
            {
                result += " ";
            }

            switch (PIECE_TYPE(piece))
            {
                case PIECE_PAWN:    result += "p"; break;
                case PIECE_KNIGHT:  result += "N"; break;
                case PIECE_BISHOP:  result += "B"; break;
                case PIECE_ROOK:    result += "R"; break;
                case PIECE_QUEEN:   result += "Q"; break;
                case PIECE_KING:    result += "K"; break;
                case PIECE_NONE:    result += " "; break;
                default:            assert (0);  break;
            }

            result += " |";
        }

        result += "\n";

        add_divider (result);;
    }

    return result;
}
