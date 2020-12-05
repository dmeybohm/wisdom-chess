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

    [[nodiscard]] static int weight (Piece piece) noexcept
    {
        switch (piece)
        {
            case Piece::None:    return MATERIAL_WEIGHT_NONE;
            case Piece::King:    return MATERIAL_WEIGHT_KING;
            case Piece::Queen:   return MATERIAL_WEIGHT_QUEEN;
            case Piece::Rook:    return MATERIAL_WEIGHT_ROOK;
            case Piece::Bishop:  return MATERIAL_WEIGHT_BISHOP;
            case Piece::Knight:  return MATERIAL_WEIGHT_KNIGHT;
            case Piece::Pawn:    return MATERIAL_WEIGHT_PAWN;
            default: abort();
        }
    }

    void add (piece_t piece)
    {
        my_score[color_index(piece_color (piece))] += weight (piece_type (piece));
    }

    void remove (piece_t piece)
    {
        my_score[color_index(piece_color (piece))] -= weight (piece_type (piece));
    }

    [[nodiscard]] int score (Color who) const
    {
        color_index_t my_index = color_index(who);
        color_index_t opponent_index = color_index(color_invert(who));
        return my_score[my_index] - my_score[opponent_index];
    }

};

#endif // EVOLVE_CHESS_MATERIAL_H_
