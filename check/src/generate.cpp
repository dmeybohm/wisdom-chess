#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "piece.h"
#include "move.h"
#include "board.h"
#include "check.h"
#include "generate.h"
#include "debug.h"
#include "board_check.h"

///////////////////////////////////////////////

#define Move_Func_Arguments \
	struct board *board, enum color who, \
	int piece_row, int piece_col, move_list_t &moves

#define Move_Func_Param_Names \
	board, who, piece_row, piece_col, moves

#define MOVES_HANDLER(name) \
	static void moves_##name (Move_Func_Arguments)

///////////////////////////////////////////////

typedef void (*MoveFunc) (Move_Func_Arguments);

MOVES_HANDLER (none);
MOVES_HANDLER (king);
MOVES_HANDLER (queen);
MOVES_HANDLER (rook);
MOVES_HANDLER (bishop);
MOVES_HANDLER (knight);
MOVES_HANDLER (pawn);

static void knight_move_list_init ();

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
	nullptr,
};

move_list_t knight_moves[NR_ROWS][NR_COLUMNS];

void add_en_passant_move (const struct board *board, enum color who, uint8_t piece_row, uint8_t piece_col,
                          move_list_t &moves, uint8_t en_passant_column);

///////////////////////////////////////////////

// generate a lookup table for knight moves
static void knight_move_list_init ()
{
	uint8_t row, col;
	uint8_t k_row, k_col;

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

				move_t knight_move = move_create (k_row+row, k_col+col, row, col);
				knight_moves[k_row+row][k_col+col].push_back (knight_move);
			}
		}
	}
}

MOVES_HANDLER (none)
{
	assert (0);
}

MOVES_HANDLER (king)
{
	int    row, col;
	
	for (row = piece_row-1; row < 8 && row <= piece_row+1; row++)
	{
		if (INVALID(row))
			continue;

		for (col = piece_col-1; col < 8 && col <= piece_col+1; col++)
		{
			if (INVALID (col))
				continue;

			moves.push_back (move_create (piece_row, piece_col, row, col));
		}
	}

    if (able_to_castle (board, who, CASTLE_QUEENSIDE))
    {
        move_t queenside_castle = move_create_castling (piece_row, piece_col,
                                                        piece_row, piece_col - 2);
        moves.push_back (queenside_castle);
    }

    if (able_to_castle (board, who, CASTLE_KINGSIDE))
    {
        move_t kingside_castle = move_create_castling (piece_row, piece_col,
                                                       piece_row, piece_col + 2);
        moves.push_back (kingside_castle);
    }
}

MOVES_HANDLER (queen)
{
	// use the generators for bishop and rook
	moves_bishop (Move_Func_Param_Names);
	moves_rook   (Move_Func_Param_Names);
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

			moves.push_back (move_create (piece_row, piece_col, row, piece_col));

			if (PIECE_TYPE(piece) != PIECE_NONE)
				break;
		}

		for (col = NEXT (piece_col, dir); VALID (col); col = NEXT (col, dir))
		{
			piece = PIECE_AT (board, piece_row, col);

			moves.push_back (move_create (piece_row, piece_col, piece_row, col));

			if (PIECE_TYPE(piece) != PIECE_NONE)
				break;
		}
	}
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
				
				moves.push_back (move_create (piece_row, piece_col, row, col));

				if (PIECE_TYPE (piece) != PIECE_NONE)
					break;
			}
		}
	}
}

MOVES_HANDLER (knight)
{
	move_list_t kt_moves = generate_knight_moves (piece_row, piece_col);

	for (auto move : kt_moves)
		moves.push_back (move);
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
	assert (VALID(piece_row));

	row = NEXT (piece_row, dir);

	memset (move, 0, sizeof (move));

	// single move
	if (PIECE_TYPE (PIECE_AT (board, row, piece_col)) == PIECE_NONE)
		move[0] = move_create (piece_row, piece_col, row, piece_col);

	// double move
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
		    PIECE_COLOR(piece) != who)
		{
			if (c_dir == -1)
				move[2] = move_create (piece_row, piece_col, row, take_col);
			else
				move[3] = move_create (piece_row, piece_col, row, take_col);
		}
	}

	// promotion
	if (need_pawn_promotion (row, who))
	{
		for_each_promotable_piece (piece_type)
		{
			piece = MAKE_PIECE (who, piece_type);

			// promotion moves dont include en passant
			for (i = 0; i < 4; i++)
			{
				if (!is_null_move (move[i]))
				{
					move[i] = move_with_promotion (move[i], piece);

					moves.push_back (move[i]);
				}
			}
		}

		return;
	}
	
	// en passant
	int8_t en_passant_column = eligible_en_passant_column (board, piece_row, piece_col, who);
	if (VALID(en_passant_column))
		add_en_passant_move (board, who, piece_row, piece_col, moves, en_passant_column);

	for (i = 0; i < sizeof (move) / sizeof (move[0]); i++)
		if (!is_null_move (move[i]))
			moves.push_back (move[i]);
}

// put en passant in a separate handler
// in order to not pollute instruction cache with it
void add_en_passant_move (const struct board *board, enum color who, uint8_t piece_row, uint8_t piece_col,
                          move_list_t &moves, uint8_t en_passant_column)
{
	move_t        new_move;
	int           direction;
	uint8_t       take_row, take_col;

	direction = PAWN_DIRECTION(who);

	take_row = NEXT (piece_row, direction);
	take_col = en_passant_column;

	piece_t take_piece = PIECE_AT (board, piece_row, take_col);

	assert (PIECE_TYPE(take_piece) == PIECE_PAWN);
	assert (PIECE_COLOR(take_piece) == color_invert(who));

	new_move = move_create_en_passant (piece_row, piece_col, take_row, take_col);

	moves.push_back (new_move);
}

///////////////////////////////////////////////

move_list_t generate_knight_moves (uint8_t row, uint8_t col)
{
	// Check f3
	if (knight_moves[0][0].empty())
		knight_move_list_init ();

	return knight_moves[row][col];
}

static const char *piece_type_str (piece_t piece)
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
	uint8_t row, col;

	for_each_position (row, col)
	{
//		DBG (generate, "%2d ", PIECE_AT (board, row, col));
		if (col == 7)
			DBG (generate, "\n");
	}

}

move_list_t generate_legal_moves (struct board *board, enum color who)
{
    board_check_t       board_check;
    move_list_t non_checks;

	move_list_t all_moves = generate_moves (board, who);
	for (auto move : all_moves)
	{
	    board_check_init (&board_check, board);
        undo_move_t undo_state = do_move (board, who, move);

        if (was_legal_move (board, who, move))
            non_checks.push_back (move);

        undo_move (board, who, move, undo_state);
        board_check_validate (&board_check, board, who, move);
	}

	return non_checks;
}

static int valid_castling_move (struct board *board, enum color who, move_t move)
{
	// check for an intervening piece
	int     direction;
	coord_t src, dst;
	piece_t piece1, piece2, piece3;

	src = MOVE_SRC(move);
	dst = MOVE_DST(move);

	piece3 = MAKE_PIECE (COLOR_NONE, PIECE_NONE);

	// find which direction the king was castling in
	direction = (COLUMN(dst) - COLUMN(src)) / 2;

	piece1 = PIECE_AT (board, ROW(src), COLUMN(dst) - direction);
	piece2 = PIECE_AT (board, ROW(src), COLUMN(dst));
	
	if (direction < 0)
	{
		// check for piece next to rook on queenside
		piece3 = PIECE_AT (board, ROW(src), COLUMN(dst) - 1);
	}

	return PIECE_TYPE(piece1) == PIECE_NONE &&
	       PIECE_TYPE(piece2) == PIECE_NONE &&
		   PIECE_TYPE(piece3) == PIECE_NONE;
}

move_list_t validate_moves (const move_list_t &move_list, struct board *board,
                                    enum color who)
{
    move_list_t captures;

	for (auto move : move_list)
	{
		coord_t src, dst;
		piece_t src_piece, dst_piece;
		int     is_capture = 0;

		src = MOVE_SRC(move);
		dst = MOVE_DST(move);

		src_piece = PIECE_AT_COORD (board, src);
		dst_piece = PIECE_AT_COORD (board, dst);

		assert (PIECE_TYPE(src_piece) != PIECE_NONE);

		is_capture = (PIECE_TYPE(dst_piece) != PIECE_NONE);

		if (is_en_passant_move(move))
			is_capture = 1;

		if (is_castling_move(move))
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
		assert (PIECE_TYPE(dst_piece) != PIECE_KING);

		if (is_capture)
		{
			if (!is_capture_move(move) && !is_en_passant_move(move))
				move = move_with_capture (move);

			captures.push_back (move);
		}
		else
		{
#if 0
			non_captures = move_list_append_move (non_captures, *move);	
#endif
			captures.push_back (move);
		}

	}

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

	return captures;
}

move_list_t generate_captures (struct board *board, enum color who)
{
    move_list_t captures;

    move_list_t move_list = generate_moves (board, who);

    for (auto move : move_list)
    {
        if (is_capture_move (move))
            captures.push_back (move);
    }

	return captures;
}

move_list_t generate_moves (struct board *board, enum color who)
{
	uint8_t     row, col;
	move_list_t new_moves;

	for_each_position (row, col)
	{
		piece_t    piece  = PIECE_AT (board, row, col);
		enum color color  = PIECE_COLOR (piece);

		if (PIECE_TYPE(piece) == PIECE_NONE)
			continue;

		if (color != who)
			continue;

		(*move_functions[PIECE_TYPE(piece)])
		    (board, who, row, col, new_moves);
	}

	return validate_moves (new_moves, board, who);
}

// vim: set ts=4 sw=4:
