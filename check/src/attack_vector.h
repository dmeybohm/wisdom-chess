#ifndef WIZDUMB_ATTACK_VECTOR_H
#define WIZDUMB_ATTACK_VECTOR_H

#include "global.h"
#include "piece.h"
#include "coord.h"

struct board;

struct attack_vector
{
    uint8_t   ne_to_sw_diagonals[NR_PLAYERS][NR_ROWS * 2];       // bishop, queen
    uint8_t   nw_to_se_diagonals[NR_PLAYERS][NR_ROWS * 2];       // bishop, queen
    uint8_t   horizontals[NR_PLAYERS][NR_ROWS];                  // rook, queen
    uint8_t   verticals[NR_PLAYERS][NR_COLUMNS];                 // rook, queen
    uint8_t   other[NR_PLAYERS][NR_ROWS][NR_COLUMNS];            // pawn, knight, king
};

// Initialize the attack vector.
void attack_vector_init (struct attack_vector *attacks);

// Add the piece's contribution to the attacks at the coordinate.
void attack_vector_add (struct attack_vector *attacks, struct board *board, enum color who, coord_t coord,
                        enum piece_type piece);

// Remove the piece's contribution to the attacks at the coordinate.
void attack_vector_remove (struct attack_vector *attacks, struct board *board, enum color who, coord_t coord,
                           enum piece_type piece);

// Get the number of pieces attacking a coordinate.
uint8_t attack_vector_count (const struct attack_vector *attacks, enum color who, coord_t coord);

void attack_vector_init_coord_to_diagonal_table (uint8_t *coord_to_diagnal_table);

/////////////////////////////////////////


#endif //WIZDUMB_ATTACK_VECTOR_H