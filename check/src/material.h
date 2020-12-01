#ifndef EVOLVE_CHESS_MATERIAL_H_
#define EVOLVE_CHESS_MATERIAL_H_

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
private:
	int my_score[NR_PLAYERS]{};

public:
    material () = default;

    [[nodiscard]] static int weight (enum piece_type piece) noexcept
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

    void add (piece_t piece)
    {
        my_score[color_index(PIECE_COLOR(piece))] += weight (PIECE_TYPE(piece));
    }

    void remove (piece_t piece)
    {
        my_score[color_index(PIECE_COLOR(piece))] -= weight (PIECE_TYPE(piece));
    }

    [[nodiscard]] int score (enum color who) const
    {
        color_index_t my_index = color_index(who);
        color_index_t opponent_index = color_index(color_invert(who));
        return my_score[my_index] - my_score[opponent_index];
    }

};

#endif // EVOLVE_CHESS_MATERIAL_H_
