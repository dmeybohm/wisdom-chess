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

class board_iterator;

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

	board_iterator begin();
	board_iterator end();
	[[nodiscard]] std::string to_string();
};

struct board_positions
{
    int               rank;
    enum color        piece_color;
    enum piece_type  *pieces;
};

class board_iterator final
{
private:
    struct board *my_board;
    uint8_t my_row;
    uint8_t my_col;

public:
    board_iterator(struct board *board, uint8_t row, uint8_t col) :
            my_board { board },
            my_row { row },
            my_col { col }
    {
        assert (my_row <= NR_ROWS);
        assert (my_col < NR_COLUMNS);
    }

    piece_t& operator*() const
    {
        return my_board->board[my_row][my_col];
    }

    bool operator == (const board_iterator & other) const
    {
        return my_board == other.my_board &&
               my_row == other.my_row &&
               my_col == other.my_col;
    }

    bool operator != (const board_iterator & other) const
    {
        return !(*this == other);
    }

    board_iterator& operator++()
    {
        my_col++;
        if (my_col == NR_COLUMNS)
        {
            my_row++;
            my_col = 0;
        }
        return *this;
    }

    [[nodiscard]] coord_t coord() const
    {
        return coord_create (my_row, my_col);
    }

    [[nodiscard]] uint8_t row() const
    {
        return my_row;
    }

    [[nodiscard]] uint8_t col() const
    {
        return my_col;
    };
};

///////////////////////////////////////////////

#define for_each_position(row_i, col_i) \
	for ((row_i) = 0; (row_i) < NR_ROWS; (row_i)++) \
		for ((col_i) = 0; (col_i) < NR_COLUMNS; (col_i)++)

static inline piece_t PIECE_AT (const struct board *board, uint8_t row, uint8_t col)
{
    assert (row < NR_ROWS && col < NR_COLUMNS);
    return board->board[row][col];
}

static inline piece_t PIECE_AT_COORD (const struct board *board, coord_t coord)
{
    return PIECE_AT (board, ROW(coord), COLUMN(coord));
}

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

constexpr int able_to_castle (struct board *board, enum color who,
                                  castle_state_t castle_type)
{
    color_index_t c_index = color_index(who);

	int didnt_castle = !!(board->castled[c_index] != CASTLE_CASTLED);
	int neg_not_set  = !!(((~board->castled[c_index]) & castle_type) != 0);

	return didnt_castle && neg_not_set;
}

constexpr castle_state_t board_get_castle_state (struct board *board, enum color who)
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
