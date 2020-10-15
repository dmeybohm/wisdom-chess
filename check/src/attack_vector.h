#ifndef WIZDUMB_ATTACK_VECTOR_H
#define WIZDUMB_ATTACK_VECTOR_H

#include "global.h"
#include "piece.h"
#include "coord.h"

struct attack_vector
{
    uint8_t   attack_counts[NR_PLAYERS][NR_ROWS][NR_COLUMNS];
};

// Initialize the attack vector.
void attack_vector_init (struct attack_vector *attacks);

// Add the piece's contribution to the attacks at the coordinate.
void attack_vector_add (struct attack_vector *attacks, enum color who, coord_t coord, enum piece_type piece);

// Remove the piece's contribution to the attacks at the coordinate.
void attack_vector_remove (struct attack_vector *attacks, enum color who, coord_t coord, enum piece_type piece);

// Get the number of pieces attacking a coordinate.
uint8_t attack_vector_count (struct attack_vector *attacks, enum color who, coord_t coord);

#endif //WIZDUMB_ATTACK_VECTOR_H