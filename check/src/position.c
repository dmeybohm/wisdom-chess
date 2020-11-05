
#include "position.h"

static int knight_positions[NR_ROWS][NR_COLUMNS] = {
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 2, 2, 0, 0, 0 },
        { 0, 0, 2, 2, 2, 2, 0, 0 },
        { 0, 0, 2, 0, 0, 2, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
};

static int bishop_positions[NR_ROWS][NR_COLUMNS] = {
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 2, 0, 0, 2, 0, 0 },
        { 0, 0, 2, 0, 0, 2, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 1, 0, 0, 0, 0, 1, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
};

static int rook_positions[NR_ROWS][NR_COLUMNS] = {
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 7, 8, 8, 8, 8, 8, 8, 7 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
};

static int pawn_positions[NR_ROWS][NR_COLUMNS] = {
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 5, 5, 5, 5, 0, 0 },
        { 0, 0, 5, 8, 8, 5, 0, 0 },
        { 0, 0, 5, 8, 8, 5, 0, 0 },
        { 0, 0, 5, 5, 5, 5, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
};

static int king_positions[NR_ROWS][NR_COLUMNS] = {
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 7, 0, 0, 0, 8, 0 },
};

static int queen_positions[NR_ROWS][NR_COLUMNS] = {
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 7, 8, 8, 8, 8, 8, 8, 7 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
};

static coord_t translate_position (coord_t coord, enum color who)
{
    if (who == COLOR_WHITE)
        return coord;

    uint8_t row = ROW(coord);
    uint8_t col = COLUMN(coord);

    return coord_create (7 - row, col);
}

void position_init (struct position *positions)
{
    positions->score[0] = positions->score[1] = 0;
}

static int change (coord_t coord, color_t who, piece_t piece)
{
    coord_t translated_pos = translate_position (coord, who);
    uint8_t row = ROW(translated_pos);
    uint8_t col = COLUMN(translated_pos);

    switch (PIECE_TYPE(piece))
    {
        case PIECE_PAWN:
            return pawn_positions[row][col];
        case PIECE_KNIGHT:
            return knight_positions[row][col];
        case PIECE_BISHOP:
            return bishop_positions[row][col];
        case PIECE_ROOK:
            return rook_positions[row][col];
        case PIECE_QUEEN:
            return queen_positions[row][col];
        case PIECE_KING:
            return king_positions[row][col];
        default:
            assert (0);
    }
}

void position_add (struct position *position, coord_t coord, color_t who, piece_t piece)
{
    color_index_t index = color_index(who);
    position->score[index] += change (coord, who, piece);
}

void position_remove (struct position *position, coord_t coord, color_t who, piece_t piece)
{
    color_index_t index = color_index(who);
    position->score[index] -= change (coord, who, piece);
}