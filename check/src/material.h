#ifndef EVOLVE_CHESS_MATERIAL_H_
#define EVOLVE_CHESS_MATERIAL_H_

#include <stdlib.h>

#include "global.h"
#include "piece.h"

#define MATERIAL_WEIGHT_NONE       0
#define MATERIAL_WEIGHT_KING       1500
#define MATERIAL_WEIGHT_QUEEN      1000
#define MATERIAL_WEIGHT_ROOK       500
#define MATERIAL_WEIGHT_BISHOP     320
#define MATERIAL_WEIGHT_KNIGHT     320
#define MATERIAL_WEIGHT_PAWN       100

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
	
	return 0;
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

static inline int material_score (struct material *material, color_t who)
{
	return material->score[who] - material->score[color_invert(who)];
}

void material_init  (struct material *material);

#endif /* EVOLVE_CHESS_MATERIAL_H_ */
