#ifndef EVOLVE_CHESS_BOARD_H_
#define EVOLVE_CHESS_BOARD_H_

#include <stdio.h>
#include <assert.h>

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
	piece_t              board[NR_ROWS][NR_COLUMNS];

	// positions of the kings
	coord_t              king_pos[NR_PLAYERS];

	// castle state of the board
	castle_state_t       castled[NR_PLAYERS];

	// keep track of the material on the board
	struct material      material;

	// keep track of the positions on the board
	struct position      position;

	// keep track of hashing information
	// TODO maybe move this higher up for better performance
	struct board_hash    hash;
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
    ((_board)->board)[ROW ((_coord))][COLUMN ((_coord))]

///////////////////////////////////////////////

// white moves up (-)
// black moves down (+)
static inline int PAWN_DIRECTION (color_t color)
{
    assert (color == COLOR_WHITE || color == COLOR_BLACK);
	return color == COLOR_BLACK ? 1 : -1;
}

static inline bool need_pawn_promotion (uint8_t row, color_t who)
{
    switch (who)
    {
        case COLOR_WHITE: return 0 == row;
        case COLOR_BLACK: return 7 == row;
        case COLOR_NONE: fprintf (stderr, "oops\n"); abort();
        default: abort();
    }
}

static inline int able_to_castle (struct board *board, color_t who,
                                  castle_state_t castle_type)
{
	int didnt_castle;
	int neg_not_set;
    color_index_t c_index = color_index(who);

	didnt_castle = !!(board->castled[c_index] != CASTLE_CASTLED);
	neg_not_set  = !!(((~board->castled[c_index]) & castle_type) != 0);

	return didnt_castle && neg_not_set;
}

static inline castle_state_t board_get_castle_state (struct board *board, color_t who)
{
    color_index_t index = color_index(who);
    return board->castled[index];
}

static inline void board_apply_castle_change (struct board *board, color_t who, castle_state_t castle_state)
{
    color_index_t index = color_index(who);
    board->castled[index] = castle_state;
}

static inline void board_undo_castle_change (struct board *board, color_t who, castle_state_t castle_state)
{
    color_index_t index = color_index(who);
    board->castled[index] = castle_state;
}

static inline int may_do_en_passant (unsigned char row, color_t who)
{
	// if WHITE rank 4, black rank 3
	assert (who == COLOR_WHITE || who == COLOR_BLACK);
	return (who == COLOR_WHITE ? 3 : 4) == row;
}

static inline int is_pawn_unmoved (struct board *board, 
                                   coord_t row, coord_t col)
{
	piece_t piece = PIECE_AT (board, row, col);

	if (PIECE_COLOR(piece) == COLOR_WHITE)
		return row == 6;
	else
		return row == 1;
}

static inline coord_t king_position (const struct board *board, color_t who)
{
	return board->king_pos[color_index(who)];
}

static inline void king_position_set (struct board *board, color_t who, coord_t pos)
{
	board->king_pos[color_index(who)] = pos;
}

static inline void board_set_piece (struct board *board, coord_t place, piece_t piece)
{
    board->board[ROW(place)][COLUMN(place)] = piece;
}

///////////////////////////////////////////////

struct board *board_new            (void);
struct board *board_from_positions (const struct board_positions *positions);
void          board_free           (struct board *board);

void          board_print     (struct board *board);
void          board_print_err (struct board *board);
void          board_dump      (struct board *board);

///////////////////////////////////////////////

#endif // EVOLVE_CHESS_BOARD_H_
