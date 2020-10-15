#include "attack_vector.h"
#include "generate.h"

#include <memory.h>

static void update_pawn   (struct attack_vector *attacks, enum color who, coord_t at, int change);
static void update_knight (struct attack_vector *attacks, enum color who, coord_t at, int change);
static void update_bishop (struct attack_vector *attacks, enum color who, coord_t at, int change);
static void update_rook   (struct attack_vector *attacks, enum color who, coord_t at, int change);
static void update_queen  (struct attack_vector *attacks, enum color who, coord_t at, int change);
static void update_king   (struct attack_vector *attacks, enum color who, coord_t at, int change);

void attack_vector_init (struct attack_vector *attacks)
{
    memset (attacks, 0, sizeof(*attacks));
}

static void attack_vector_change (
        struct attack_vector *attacks, enum color who, coord_t coord, enum piece_type piece, int change)
{
    switch (piece)
    {
        case PIECE_PAWN:
            update_pawn (attacks, who, coord, change);
            break;

        case PIECE_KNIGHT:
            update_knight (attacks, who, coord, change);
            break;

        case PIECE_BISHOP:
            update_bishop (attacks, who, coord, change);
            break;

        case PIECE_ROOK:
            update_rook (attacks, who, coord, change);
            break;

        case PIECE_QUEEN:
            update_queen (attacks, who, coord, change);
            break;

        case PIECE_KING:
            update_king (attacks, who, coord, change);
            break;

        default:
            assert (0);
            break;
    }
}

void attack_vector_add (struct attack_vector *attacks, enum color who, coord_t coord, enum piece_type piece)
{
    attack_vector_change (attacks, who, coord, piece, +1);
}

void attack_vector_remove (struct attack_vector *attacks, enum color who, coord_t coord, enum piece_type piece)
{
    attack_vector_change (attacks, who, coord, piece, -1);
}

uint8_t attack_vector_count (struct attack_vector *attacks, enum color who, coord_t coord)
{
    assert (VALID(ROW(coord)) && VALID(COLUMN(coord)));
    return attacks->attack_counts[color_index(who)][ROW(coord)][COLUMN(coord)];
}

//////////////////////////////

static void update_pawn (struct attack_vector *attacks, enum color who, coord_t at, int change)
{
    int direction = PAWN_DIRECTION(who);
    int next_row = NEXT (ROW(at), direction);
    int left_col = NEXT (COLUMN(at), -1);
    int right_col = NEXT (COLUMN(at), +1);

    if (VALID(left_col))
        attacks->attack_counts[color_index(who)][next_row][left_col] += change;

    if (VALID(right_col))
        attacks->attack_counts[color_index(who)][next_row][right_col] += change;
}

static void update_knight (struct attack_vector *attacks, enum color who, coord_t at, int change)
{

}

static void update_bishop (struct attack_vector *attacks, enum color who, coord_t at, int change)
{

}

static void update_rook (struct attack_vector *attacks, enum color who, coord_t at, int change)
{

}

static void update_queen (struct attack_vector *attacks, enum color who, coord_t at, int change)
{
    update_bishop (attacks, who, at, change);
    update_rook (attacks, who, at, change);
}

static void update_king (struct attack_vector *attacks, enum color who, coord_t at, int change)
{
    uint8_t king_row = ROW(at);
    uint8_t king_col = COLUMN(at);
    uint8_t first_row = NEXT (king_row, -1);
    uint8_t first_col = NEXT (king_col, -1);
    uint8_t last_row = NEXT (king_row, +1);
    uint8_t last_col = NEXT (king_col, +1);
    color_index_t index = color_index(who);

    for (uint8_t row = first_row; row <= last_row; row = NEXT (row, +1))
    {
        for (uint8_t col = first_col; col <= last_col; col = NEXT (col, +1))
        {
            if (row == king_row && col == king_col)
                continue;

            if (!VALID(row) || !VALID(col))
                continue;

            attacks->attack_counts[index][row][col] += change;
        }
    }
}