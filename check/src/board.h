#ifndef EVOLVE_CHESS_BOARD_H_
#define EVOLVE_CHESS_BOARD_H_

#include <cassert>
#include <vector>

#include "global.h"
#include "config.h"
#include "coord.h"
#include "coord_iterator.hpp"
#include "move.h"
#include "piece.h"
#include "board_hash.h"
#include "material.h"
#include "position.h"

///////////////////////////////////////////////

struct move_tree;

struct board_positions
{
    int  rank;
    Color piece_color;
    std::vector<Piece> pieces;
};

struct board
{
    piece_t                  squares[NR_ROWS][NR_COLUMNS];

	// positions of the kings
	coord_t                  king_pos[NR_PLAYERS];

	// castle state of the board
	castle_state_t           castled[NR_PLAYERS];

	// keep track of hashing information
	struct board_hash        hash;

	// keep track of the material on the board
	struct material          material;

	// keep track of the positions on the board
	struct position          position;

	// The columns which are eligible for en_passant
	coord_t                  en_passant_target[NR_PLAYERS];

	// Number of half moves since pawn or capture.
	size_t                   half_move_clock;

	// Number of full moves, updated after black moves.
	size_t                   full_moves;

	board ();
	board (const board &board);
    explicit board (const std::vector<board_positions> &positions);

    void print () const;
    void print_to_file  (std::ostream &out) const;
    void dump () const;

	[[nodiscard]] std::string to_string() const;
};

///////////////////////////////////////////////

static inline piece_t piece_at (const struct board &board, int8_t row, int8_t col)
{
    assert (is_valid_row(row) && is_valid_column(col));
    return board.squares[row][col];
}

static inline piece_t piece_at (const struct board &board, coord_t coord)
{
    return piece_at (board, coord.row, coord.col);
}

///////////////////////////////////////////////

// white moves up (-)
// black moves down (+)
static inline int pawn_direction (Color color)
{
    assert (color == Color::White || color == Color::Black);
	return color == Color::Black ? 1 : -1;
}

static inline bool need_pawn_promotion (int8_t row, Color who)
{
    assert (is_color_valid(who));
    switch (who)
    {
        case Color::White: return 0 == row;
        case Color::Black: return 7 == row;
        default: abort();
    }
}

constexpr int able_to_castle (const struct board &board, Color who,
                              castle_state_t castle_type)
{
    color_index_t c_index = color_index(who);

	int didnt_castle = !!(board.castled[c_index] != CASTLE_CASTLED);
	int neg_not_set  = !!(((~board.castled[c_index]) & castle_type) != 0);

	return didnt_castle && neg_not_set;
}

constexpr castle_state_t board_get_castle_state (const struct board &board, Color who)
{
    color_index_t index = color_index(who);
    return board.castled[index];
}

static inline void board_apply_castle_change (struct board &board, Color who, castle_state_t castle_state)
{
    color_index_t index = color_index(who);
    board.castled[index] = castle_state;
}

static inline void board_undo_castle_change (struct board &board, Color who, castle_state_t castle_state)
{
    color_index_t index = color_index(who);
    board.castled[index] = castle_state;
}

static inline coord_t king_position (const struct board &board, Color who)
{
	return board.king_pos[color_index(who)];
}

static inline void king_position_set (struct board &board, Color who, coord_t pos)
{
	board.king_pos[color_index(who)] = pos;
}

static inline void board_set_piece (struct board &board, coord_t place, piece_t piece)
{
    board.squares[ROW(place)][COLUMN(place)] = piece;
}

constexpr bool is_en_passant_vulnerable (const struct board &board, Color who)
{
    return board.en_passant_target[color_index(who)] != no_en_passant_coord;
}

static inline bool board_equals (const struct board &a, const struct board &b)
{
    for (int8_t row = 0; row < NR_ROWS; row++)
    {
		for (int8_t col = 0; col < NR_COLUMNS; col++)
        {
            if (a.squares[row][col] != b.squares[row][col])
                return false;
        }
    }

    // todo check more
    return true;
}

///////////////////////////////////////////////


#endif // EVOLVE_CHESS_BOARD_H_
