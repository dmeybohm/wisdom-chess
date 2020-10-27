#ifndef WIZDUMB_ATTACK_VECTOR_H
#define WIZDUMB_ATTACK_VECTOR_H

#include "global.h"
#include "piece.h"
#include "coord.h"
#include "bitboard.h"
#include "compact_bitboard.h"

struct board;

#define NR_DIAGONAL_ROWS     (2U)

#define ATTACK_VECTOR_HORIZ_VERT_ROWS   (1U)

#define ATTACK_VECTOR_DIAG_RELEVANT_UNITS \
    compact_bitboard_total_units(NR_DIAGONAL_ROWS, NR_PLAYERS, 4)

#define ATTACK_VECTOR_HORIZ_VERT_RELEVANT_UNITS \
    compact_bitboard_total_units(1, NR_PLAYERS, 4)

struct attack_vector
{
    // pawn, knight, king
    per_player_bitboard_t other[per_player_bitboard_total_units(4)]; // 4 bits - max 16 pieces on one square.

    per_player_bitboard_t nw_to_se[per_player_bitboard_total_units(2)];    // bishop, queen
    per_player_bitboard_t ne_to_sw[per_player_bitboard_total_units(2)];    // bishop, queen
//    per_player_bitboard_t horizontals[per_player_bitboard_total_units(2)]; // rook, queen
//    per_player_bitboard_t verticals[per_player_bitboard_total_units(2)];   // rook, queen

//    uint8_t   nw_to_se[NR_PLAYERS][NR_ROWS][NR_COLUMNS];
//    uint8_t   ne_to_sw[NR_PLAYERS][NR_ROWS][NR_COLUMNS];
    uint8_t   horizontals[NR_PLAYERS][NR_ROWS][NR_COLUMNS];
    uint8_t   verticals[NR_PLAYERS][NR_ROWS][NR_COLUMNS];
//    uint8_t   other[NR_PLAYERS][NR_ROWS][NR_COLUMNS];

    compact_bitboard_t nw_to_se_relevant_count[ATTACK_VECTOR_DIAG_RELEVANT_UNITS];
    compact_bitboard_t ne_to_sw_relevant_count[ATTACK_VECTOR_DIAG_RELEVANT_UNITS];

    compact_bitboard_t horizontal_relevant_count[ATTACK_VECTOR_HORIZ_VERT_RELEVANT_UNITS];
    compact_bitboard_t vertical_relevant_count[ATTACK_VECTOR_HORIZ_VERT_RELEVANT_UNITS];
};

// Initialize the attack vector.
void attack_vector_init (struct attack_vector *attacks, struct board *board);

// Add the piece's contribution to the attacks at the coordinate.
void attack_vector_add (struct attack_vector *attacks, struct board *board, enum color who, coord_t coord,
                        enum piece_type piece);

// Remove the piece's contribution to the attacks at the coordinate.
void attack_vector_remove (struct attack_vector *attacks, struct board *board, enum color who, coord_t coord,
                           enum piece_type piece);

// Get the number of pieces attacking a coordinate.
uint8_t attack_vector_count (const struct attack_vector *attacks, enum color who, coord_t coord);

/////////////////////////////////////////

static inline coord_t first_nw_to_se_coord (coord_t coord)
{
    // 0  1  2  3  4  5  6  7
    // 8  0  1  2  3  4  5  6
    // 2  8  0  1  2  3  4  5
    // 3  4  8  0  1  2  3  4
    // 4  5  6  8  0  1  2  3
    // 5  6  7  8  8  0  1  2
    // 6  7  8  9 10  8  0  1
    // 7  8  9 10 11 12  8  0

    // 0, 0 -> 0, 0
    // 1, 1, -> 0, 0
    // 2, 2, -> 0, 0

    // 1, 0 -> 1, 0
    // 2, 1 -> 1, 0
    // 3, 2 -> 1, 0

    // 0, 6 -> 0, 6
    // 1, 7 -> 0, 6

    uint8_t row = ROW(coord), col = COLUMN(coord);

    if (row >= col) {
        return coord_create(row - col, 0);
    } else {
        return coord_create (0, col - row);
    }
}

static inline coord_t first_ne_to_sw_coord (coord_t coord)
{
    // 0  1  2  3  4  5  6  7
    // 1  2  3  4  5  6  7  8
    // 2  3  4  5  6  7  8  9
    // 3  4  5  6  7  8  9 10
    // 4  5  6  7  8  9 10 11
    // 5  6  7  8  9 10 11 12
    // 6  7  8  9 10 11 12 13
    // 7  8  9 10 11 12 13 14

    uint8_t row = ROW(coord), col = COLUMN(coord);

    // 7, 7 -> (7, 7)
    // 6, 7 -> (6, 7)
    // 7, 6 -> (6, 7)
    // 5, 7 -> (5, 7)

    uint8_t sum = row + col;

    if (sum < NR_COLUMNS) {
        return coord_create(0, sum);
    } else {
        return coord_create (sum - 7, 7);
    }
}

#endif //WIZDUMB_ATTACK_VECTOR_H