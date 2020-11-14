#ifndef EVOLVE_CHESS_MATERIAL_H_
#define EVOLVE_CHESS_MATERIAL_H_

#include <cstdlib>

#include "global.h"
#include "piece.h"

enum material_weight
{
    MATERIAL_WEIGHT_NONE   = 0,
    MATERIAL_WEIGHT_KING   = 1500,
    MATERIAL_WEIGHT_QUEEN  = 1000,
    MATERIAL_WEIGHT_ROOK   = 500,
    MATERIAL_WEIGHT_BISHOP = 320,
    MATERIAL_WEIGHT_KNIGHT = 320,
    MATERIAL_WEIGHT_PAWN   = 100,
};

struct material
{
	int score[NR_PLAYERS];
};

static inline int material_piece_weight (enum piece_type piece)
{
	switch (piece)
	{
		case PIECE_NONE:    return MATERIAL_WEIGHT_NONE;
		case PIECE_KING:    return MATERIAL_WEIGHT_KING;
		case PIECE_QUEEN:   return MATERIAL_WEIGHT_QUEEN;
		case PIECE_ROOK:    return MATERIAL_WEIGHT_ROOK;
		case PIECE_BISHOP:  return MATERIAL_WEIGHT_BISHOP;
		case PIECE_KNIGHT:  return MATERIAL_WEIGHT_KNIGHT;
		case PIECE_PAWN:    return MATERIAL_WEIGHT_PAWN;
		default: abort();
    }
}

static inline void material_del (struct material *material, piece_t piece)
{
	material->score[color_index(PIECE_COLOR(piece))]
		-= material_piece_weight (PIECE_TYPE(piece));
}

static inline void material_add (struct material *material, piece_t piece)
{
	material->score[color_index(PIECE_COLOR(piece))] +=
		material_piece_weight (PIECE_TYPE(piece));
}

static inline int material_score (struct material *material, enum color who)
{
    color_index_t my_index = color_index(who);
    color_index_t opponent_index = color_index(color_invert(who));
	return material->score[my_index] - material->score[opponent_index];
}

void material_init  (struct material *material);

#endif // EVOLVE_CHESS_MATERIAL_H_
