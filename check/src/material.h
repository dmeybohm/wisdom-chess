#ifndef EVOLVE_CHESS_MATERIAL_H_
#define EVOLVE_CHESS_MATERIAL_H_

#include "global.h"
#include "piece.h"

enum material_weight
{
    Material_Weight_None   = 0,
    Material_Weight_King   = 1500,
    Material_Weight_Queen  = 1000,
    Material_Weight_Rook   = 500,
    Material_Weight_Bishop = 320,
    Material_Weight_Knight = 320,
    Material_Weight_Pawn   = 100,
};

struct material
{
private:
	int my_score[Num_Players]{};

public:
    material () = default;

    [[nodiscard]] static int weight (Piece piece) noexcept
    {
        switch (piece)
        {
            case Piece::None:    return Material_Weight_None;
            case Piece::King:    return Material_Weight_King;
            case Piece::Queen:   return Material_Weight_Queen;
            case Piece::Rook:    return Material_Weight_Rook;
            case Piece::Bishop:  return Material_Weight_Bishop;
            case Piece::Knight:  return Material_Weight_Knight;
            case Piece::Pawn:    return Material_Weight_Pawn;
            default: abort();
        }
    }

    void add (piece_t piece)
    {
        my_score[color_index(piece_color(piece))] += weight (piece_type(piece));
    }

    void remove (piece_t piece)
    {
        my_score[color_index(piece_color(piece))] -= weight (piece_type(piece));
    }

    [[nodiscard]] int score (Color who) const
    {
        color_index_t my_index = color_index(who);
        color_index_t opponent_index = color_index(color_invert(who));
        return my_score[my_index] - my_score[opponent_index];
    }

};

#endif // EVOLVE_CHESS_MATERIAL_H_
