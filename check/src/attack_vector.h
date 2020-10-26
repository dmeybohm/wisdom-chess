#ifndef WIZDUMB_ATTACK_VECTOR_H
#define WIZDUMB_ATTACK_VECTOR_H

#include "global.h"
#include "piece.h"
#include "coord.h"

struct board;

struct attack_vector
{
    uint8_t   nw_to_se[NR_PLAYERS][NR_ROWS][NR_COLUMNS];       // bishop, queen
    uint8_t   ne_to_sw[NR_PLAYERS][NR_ROWS][NR_COLUMNS];       // bishop, queen
    uint8_t   horizontals[NR_PLAYERS][NR_ROWS][NR_COLUMNS];    // rook, queen
    uint8_t   verticals[NR_PLAYERS][NR_ROWS][NR_COLUMNS];      // rook, queen
    uint8_t   other[NR_PLAYERS][NR_ROWS][NR_COLUMNS];          // pawn, knight, king
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


#endif //WIZDUMB_ATTACK_VECTOR_H