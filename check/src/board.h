#ifndef EVOLVE_CHESS_BOARD_H_
#define EVOLVE_CHESS_BOARD_H_

#include <cstdio>
#include <cassert>

#include "global.h"
#include "config.h"
#include "coord.h"
#include "move.h"
#include "piece.h"
#include "board_hash.h"
#include "material.h"
#include "position.h"

///////////////////////////////////////////////

/* place holders for branch prediction optimization */
#define likely(x)        (x)
#define unlikely(x)      (x)

///////////////////////////////////////////////

struct material;
struct move_tree;
struct board_positions;

struct board
{
	piece_t                  board[NR_ROWS][NR_COLUMNS];

	// positions of the kings
	coord_t                  king_pos[NR_PLAYERS];

	// castle state of the board
	castle_state_t           castled[NR_PLAYERS];

	// keep track of the material on the board
	struct material          material;

	// keep track of the positions on the board
	struct position          position;

	// The columns which are eligible for en_passant
	coord_t                  en_passant_target[NR_PLAYERS];

	// keep track of hashing information
	// TODO maybe move this higher up for better performance
	struct board_hash        hash;

	// Number of half moves since pawn or capture.
	size_t                   half_move_clock;

	// Number of full moves, updated after black moves.
	size_t                   full_moves;
};

struct board_positions
{
    int               rank;
    enum color        piece_color;
    enum piece_type  *pieces;
};

///////////////////////////////////////////////

#define for_each_position(row_i, col_i) \
	for ((row_i) = 0; (row_i) < NR_ROWS; (row_i)++) \
		for ((col_i) = 0; (col_i) < NR_COLUMNS; (col_i)++)

#define PIECE_AT(_board,_row,_col) \
    ((_board)->board)[(_row)][(_col)]

#define PIECE_AT_COORD(_board, _coord) \
    ((_board)->board)[ROW((_coord))][COLUMN((_coord))]

///////////////////////////////////////////////

// white moves up (-)
// black moves down (+)
static inline int PAWN_DIRECTION (enum color color)
{
    assert (color == COLOR_WHITE || color == COLOR_BLACK);
	return color == COLOR_BLACK ? 1 : -1;
}

static inline bool need_pawn_promotion (uint8_t row, enum color who)
{
    switch (who)
    {
        case COLOR_WHITE: return 0 == row;
        case COLOR_BLACK: return 7 == row;
        case COLOR_NONE: fprintf (stderr, "oops\n"); abort();
        default: abort();
    }
}

static inline int able_to_castle (struct board *board, enum color who,
                                  castle_state_t castle_type)
{
	int didnt_castle;
	int neg_not_set;
    color_index_t c_index = color_index(who);

	didnt_castle = !!(board->castled[c_index] != CASTLE_CASTLED);
	neg_not_set  = !!(((~board->castled[c_index]) & castle_type) != 0);

	return didnt_castle && neg_not_set;
}

static inline castle_state_t board_get_castle_state (struct board *board, enum color who)
{
    color_index_t index = color_index(who);
    return board->castled[index];
}

static inline void board_apply_castle_change (struct board *board, enum color who, castle_state_t castle_state)
{
    color_index_t index = color_index(who);
    board->castled[index] = castle_state;
}

static inline void board_undo_castle_change (struct board *board, enum color who, castle_state_t castle_state)
{
    color_index_t index = color_index(who);
    board->castled[index] = castle_state;
}

static inline coord_t king_position (const struct board *board, enum color who)
{
	return board->king_pos[color_index(who)];
}

static inline void king_position_set (struct board *board, enum color who, coord_t pos)
{
	board->king_pos[color_index(who)] = pos;
}

static inline void board_set_piece (struct board *board, coord_t place, piece_t piece)
{
    board->board[ROW(place)][COLUMN(place)] = piece;
}

constexpr bool is_en_passant_vulnerable (const struct board *board, enum color who)
{
    return board->en_passant_target[color_index(who)] != no_en_passant_coord;
}

static inline bool board_equals (const struct board &a, const struct board &b)
{
    for (uint8_t row = 0; row < NR_ROWS; row++)
    {
		for (uint8_t col = 0; col < NR_COLUMNS; col++)
        {
            if (a.board[row][col] != b.board[row][col])
                return false;
        }
    }

    // todo check more
    return true;
}

///////////////////////////////////////////////

struct board *board_new            ();
struct board *board_from_positions (const struct board_positions *positions);
void          board_free           (struct board *board);

void          board_print          (struct board *board);
void          board_print_err      (struct board *board);
void          board_dump           (struct board *board);

///////////////////////////////////////////////

#endif // EVOLVE_CHESS_BOARD_H_
