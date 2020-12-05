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
	struct board &board, Color who, \
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
	moves_none,    // Piece::None   [0]
	moves_king,    // Piece::King   [1]
	moves_queen,   // Piece::Queen  [2]
	moves_rook,    // Piece::Rook   [3]
	moves_bishop,  // Piece::Bishop [4]
	moves_knight,  // Piece::Knight [5]
	moves_pawn,    // Piece::Pawn   [6]
	nullptr,
};

move_list_t knight_moves[NR_ROWS][NR_COLUMNS];

void add_en_passant_move (const struct board &board, Color who, int8_t piece_row, int8_t piece_col,
                          move_list_t &moves, int8_t en_passant_column);

///////////////////////////////////////////////

// generate a lookup table for knight moves
static void knight_move_list_init ()
{
	int k_row, k_col;
    coord_iterator all_coords;

    for (auto [row, col] : all_coords)
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
	int8_t       dir;
	int8_t       row, col;
	piece_t      piece;

	for (dir = -1; dir <= 1; dir += 2)
	{
		for (row = NEXT (piece_row, dir); VALID (row); row = NEXT (row, dir))
		{
			piece = piece_at (board, row, piece_col);

			moves.push_back (move_create (piece_row, piece_col, row, piece_col));

			if (piece_type (piece) != Piece::None)
				break;
		}

		for (col = NEXT (piece_col, dir); VALID (col); col = NEXT (col, dir))
		{
			piece = piece_at (board, piece_row, col);

			moves.push_back (move_create (piece_row, piece_col, piece_row, col));

			if (piece_type (piece) != Piece::None)
				break;
		}
	}
}

MOVES_HANDLER (bishop)
{
	int8_t r_dir, c_dir;
	int8_t row,   col;

	for (r_dir = -1; r_dir <= 1; r_dir += 2)
	{
		for (c_dir = -1; c_dir <= 1; c_dir += 2)
		{
			for (row = NEXT (piece_row, r_dir), col = NEXT (piece_col, c_dir);
			     VALID (row) && VALID (col);
			     row = NEXT (row, r_dir), col = NEXT (col, c_dir))
			{
				piece_t  piece = piece_at (board, row, col);
				
				moves.push_back (move_create (piece_row, piece_col, row, col));

				if (piece_type (piece) != Piece::None)
					break;
			}
		}
	}
}

static void moves_knight (struct board &board, Color who,
	int piece_row, int piece_col, move_list_t &moves)
{
    move_list_t kt_moves = generate_knight_moves (piece_row, piece_col);

    moves.append (kt_moves);
}

// Returns -1 if no column is eligible.
static int8_t eligible_en_passant_column (const struct board &board, int8_t row, int8_t column, Color who)
{
    color_index_t opponent_index = color_index(color_invert(who));

    if (coord_equals (board.en_passant_target[opponent_index], no_en_passant_coord))
        return -1;

    // if WHITE rank 4, black rank 3
    if ((who == Color::White ? 3 : 4) != row)
        return -1;

    int8_t left_column = column - 1;
    int8_t right_column = column + 1;
    int8_t target_column = COLUMN(board.en_passant_target[opponent_index]);

    if (left_column == target_column)
    {
        assert (VALID(left_column));
        return left_column;
    }

    if (right_column == target_column)
    {
        assert (VALID(right_column));
        return right_column;
    }

    return -1;
}

MOVES_HANDLER (pawn)
{
	int8_t             dir;
	int8_t             row;
	int8_t             take_col;
	int8_t             c_dir;
	move_t             move[4];        // 4 possible pawn moves
	piece_t            piece;
	size_t             i;

	dir = pawn_direction (who);

	// row is _guaranteed_ to be on the board, because
	// a pawn on the eight rank can't remain a pawn, and that's
	// the only direction moved in
	assert (VALID(piece_row));

	row = NEXT (piece_row, dir);
	assert (VALID(row));

	memset (move, 0, sizeof (move));

	// single move
	if (piece_type (piece_at (board, row, piece_col)) == Piece::None)
		move[0] = move_create (piece_row, piece_col, row, piece_col);

	// double move
	if (is_pawn_unmoved (board, piece_row, piece_col))
	{
		int next_row = NEXT (row, dir);

		if (!is_null_move (move[0]) &&
            piece_type (piece_at (board, next_row, piece_col)) == Piece::None)
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

		piece = piece_at (board, row, take_col);

		if (piece_type (piece_at (board, row, take_col)) != Piece::None &&
                piece_color (piece) != who)
		{
			if (c_dir == -1)
				move[2] = move_create_capturing (piece_row, piece_col, row, take_col);
			else
				move[3] = move_create_capturing (piece_row, piece_col, row, take_col);
		}
	}

	// promotion
	if (need_pawn_promotion (row, who))
	{
		for (auto promotable_piece_type : all_promotable_piece_types)
		{
			auto promoted_piece = make_piece (who, promotable_piece_type);

			// promotion moves dont include en passant
			for (i = 0; i < 4; i++)
			{
				if (!is_null_move (move[i]))
				{
					move[i] = move_with_promotion (move[i], promoted_piece);

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
void add_en_passant_move (const struct board &board, Color who, int8_t piece_row, int8_t piece_col,
                          move_list_t &moves, int8_t en_passant_column)
{
    move_t new_move;
    int direction;
    int8_t take_row, take_col;

    direction = pawn_direction (who);

    take_row = NEXT (piece_row, direction);
    take_col = en_passant_column;

    piece_t take_piece = piece_at (board, piece_row, take_col);

    assert (piece_type (take_piece) == Piece::Pawn);
    assert (piece_color (take_piece) == color_invert (who));

    new_move = move_create_en_passant (piece_row, piece_col, take_row, take_col);

    moves.push_back (new_move);
}

///////////////////////////////////////////////

const move_list_t &generate_knight_moves (int8_t row, int8_t col)
{
	if (knight_moves[0][0].empty())
		knight_move_list_init ();

	return knight_moves[row][col];
}

static const char *piece_type_str (piece_t piece)
{
	switch (piece_type (piece))
	{
	 case Piece::King: return "king";
	 case Piece::Queen: return "queen";
	 case Piece::Rook: return "rook";
	 case Piece::Bishop: return "bishop";
	 case Piece::Knight: return "knight";
	 case Piece::Pawn: return "pawn";
	 case Piece::None: return "<none>";
	 default: assert(0);
	}
}

move_list_t generate_legal_moves (struct board &board, Color who)
{
    move_list_t    non_checks;

	move_list_t all_moves = generate_moves (board, who);
	for (auto move : all_moves)
	{
        undo_move_t undo_state = do_move (board, who, move);

        if (was_legal_move (board, who, move))
            non_checks.push_back (move);

        undo_move (board, who, move, undo_state);
	}

	return non_checks;
}

static int valid_castling_move (const struct board &board, Color who, move_t move)
{
	// check for an intervening piece
	int     direction;
	coord_t src, dst;
	piece_t piece1, piece2, piece3;

	src = MOVE_SRC(move);
	dst = MOVE_DST(move);

	piece3 = make_piece (Color::None, Piece::None);

	// find which direction the king was castling in
	direction = (COLUMN(dst) - COLUMN(src)) / 2;

	piece1 = piece_at (board, ROW (src), COLUMN (dst) - direction);
	piece2 = piece_at (board, ROW (src), COLUMN (dst));
	
	if (direction < 0)
	{
		// check for piece next to rook on queenside
		piece3 = piece_at (board, ROW (src), COLUMN (dst) - 1);
	}

	return piece_type (piece1) == Piece::None &&
            piece_type (piece2) == Piece::None &&
            piece_type (piece3) == Piece::None;
}

move_list_t validate_moves (const move_list_t &move_list, const struct board &board,
                            Color who)
{
    move_list_t captures;

	for (auto move : move_list)
	{
		coord_t src, dst;
		piece_t src_piece, dst_piece;
		int     is_capture = 0;

		src = MOVE_SRC(move);
		dst = MOVE_DST(move);

		src_piece = piece_at (board, src);
		dst_piece = piece_at (board, dst);

		assert (piece_type (src_piece) != Piece::None);

		is_capture = (piece_type (dst_piece) != Piece::None);

		if (is_en_passant_move(move))
			is_capture = 1;

		if (is_castling_move(move))
			if (!valid_castling_move (board, who, move))
				continue;

		if (piece_color (src_piece) == piece_color (dst_piece) &&
                piece_type (dst_piece) != Piece::None &&
            is_capture)
		{
			assert (is_capture);

			continue;
		}

		// check for an illegal king capture
		assert (piece_type (dst_piece) != Piece::King);

		if (is_capture)
		{
			if (!is_capture_move(move) && !is_en_passant_move(move))
				move = move_with_capture (move);

			captures.push_back (move);
		}
		else
		{
			captures.push_back (move);
		}

	}

	return captures;
}

move_list_t generate_captures (struct board &board, Color who)
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

move_list_t generate_moves (struct board &board, Color who)
{
	move_list_t new_moves;

	for (const auto coord : all_coords_iterator)
	{
	    piece_t piece = piece_at (board, coord);

		if (piece_type (piece) == Piece::None)
			continue;

		Color color = piece_color (piece);
		if (color != who)
			continue;

	    auto [row, col]  = coord;
		(*move_functions[piece_index(piece_type(piece))])
		    (board, who, row, col, new_moves);
	}

	return validate_moves (new_moves, board, who);
}
