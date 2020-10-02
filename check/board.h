#ifndef EVOLVE_CHESS_BOARD_H_
#define EVOLVE_CHESS_BOARD_H_

#include <assert.h>

#include "coord.h"
#include "move.h"
#include "piece.h"
#include "board_hash.h"

/**************************************/

/* place holders for branch prediction optimization */
#define likely(x)        (x)
#define unlikely(x)      (x)

#define NR_PLAYERS       2

#define NR_ROWS          8
#define NR_COLUMNS       8

#define LAST_ROW         (NR_ROWS-1)
#define LAST_COLUMN      (NR_COLUMNS-1)

#define KING_COLUMN      4

/**************************************/

struct material;

struct board
{
	piece_t            board[NR_ROWS][NR_COLUMNS];

	/* positions of the kings */
	coord_t            king_pos[NR_PLAYERS];

	/* castle state of the board */
	enum castle        castled[NR_PLAYERS];

	/* keep track of the material on the board */
	struct material   *material;

	/* keep track of hashing information */
	struct board_hash  board_hash;
};

/**************************************/

#define for_each_position(row_i, col_i) \
	for ((row_i) = 0; (row_i) < NR_ROWS; (row_i)++) \
		for ((col_i) = 0; (col_i) < NR_COLUMNS; (col_i)++)

#define PIECE_AT(_board,_row,_col) \
    ((_board)->board)[(_row)][(_col)]

#define PIECE_AT_COORD(_board, _coord) \
    ((_board)->board)[ROW ((_coord))][COLUMN ((_coord))]

/**************************************/

/* COLOR_WHITE (0) needs to map to -1
 * COLOR_BLACK (1) to map to 1
 * because white moves up (-)
 * black moves down (+) */
static inline int PAWN_DIRECTION (color_t color)
{
    assert (color == COLOR_WHITE || color == COLOR_BLACK);
	return color == COLOR_BLACK ? 1 : -1;
}

static inline int need_pawn_promotion (unsigned char row, color_t who) 
{
	return (row - 7 * who == COLOR_WHITE);
}

static inline int able_to_castle (struct board *board, color_t who,
                                  enum castle castle_type)
{
	int didnt_castle;
	int neg_not_set;

	didnt_castle = !!(board->castled[who] != CASTLE_CASTLED);
	neg_not_set  = !!(((~board->castled[who]) & castle_type) != 0);

	return didnt_castle && neg_not_set;
	//board->castled[who] != CASTLE_CASTLED && 
	//      (board->castled[who] & castle_type) == 0;
}

static inline int may_do_en_passant (unsigned char row, color_t who)
{
	/* if WHITE rank 3, black rank 4 */
	assert (who == COLOR_WHITE || who == COLOR_BLACK);
	return (who == COLOR_WHITE ? 3 : 4) == row;
}

static inline int is_pawn_unmoved (struct board *board, 
                                   coord_t row, coord_t col)
{
	piece_t piece = PIECE_AT (board, row, col);

	/* -1 -> white, 1 -> black */
	if (PIECE_COLOR (piece) == COLOR_WHITE)
		return row == 6;
	else
		return row == 1;
}
                
/**************************************/

struct board *board_new       (void);
void          board_free      (struct board *board);

void          board_print     (struct board *board);
void          board_print_err (struct board *board);

void          do_move   (struct board *board, color_t who, struct move *m);
void          undo_move (struct board *board, color_t who, struct move *m);


/**************************************/

#endif /* EVOLVE_CHESS_BOARD_H_ */
