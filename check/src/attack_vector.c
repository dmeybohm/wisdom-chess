#include "attack_vector.h"
#include "generate.h"

#include <memory.h>

uint8_t ne_to_sw_coord_to_diagonal_table[NR_ROWS][NR_COLUMNS] =
{
    {  0,  1,  2,  3,  4,  5,  6,  7  },
    {  1,  2,  3,  4,  5,  6,  7,  8  },
    {  2,  3,  4,  5,  6,  7,  8,  9  },
    {  3,  4,  5,  6,  7,  8,  9,  10 },
    {  4,  5,  6,  7,  8,  9,  10, 11 },
    {  5,  6,  7,  8,  9,  10, 11, 12 },
    {  6,  7,  8,  9,  10, 11, 12, 13 },
    {  7,  8,  9,  10, 11, 12, 13, 14 },
};

uint8_t nw_to_se_coord_to_diagonal_table[NR_ROWS][NR_COLUMNS] =
{
    {  0,  1,  2,  3,  4,  5,  6,  7  },
    {  8,  0,  1,  2,  3,  4,  5,  6  },
    {  9,  8,  0,  1,  2,  3,  4,  5  },
    {  10, 9,  8,  0,  1,  2,  3,  4  },
    {  11, 10, 9,  8,  0,  1,  2,  3  },
    {  12, 11, 10, 9,  8,  0,  1,  2  },
    {  13, 12, 11, 10, 9,  8 , 0,  1  },
    {  14, 13, 12, 11, 10, 9,  8,  0  },
};

static void update_pawn   (struct attack_vector *attacks, struct board *board, enum color who, coord_t at, int change);
static void update_knight (struct attack_vector *attacks, struct board *board, enum color who, coord_t at, int change);
static void update_bishop (struct attack_vector *attacks, struct board *board, enum color who, coord_t at, int change);
static void update_rook   (struct attack_vector *attacks, struct board *board, enum color who, coord_t at, int change);
static void update_queen  (struct attack_vector *attacks, struct board *board, enum color who, coord_t at, int change);
static void update_king   (struct attack_vector *attacks, struct board *board, enum color who, coord_t at, int change);

void attack_vector_init (struct attack_vector *attacks)
{
    memset (attacks, 0, sizeof(*attacks));
}

static void attack_vector_change (struct attack_vector *attacks, struct board *board, enum color who, coord_t coord,
                                  enum piece_type piece, int change)
{
    switch (piece)
    {
        case PIECE_PAWN:
            update_pawn (attacks, board, who, coord, change);
            break;

        case PIECE_KNIGHT:
            update_knight (attacks, board, who, coord, change);
            break;

        case PIECE_BISHOP:
            update_bishop (attacks, board, who, coord, change);
            break;

        case PIECE_ROOK:
            update_rook (attacks, board, who, coord, change);
            break;

        case PIECE_QUEEN:
            update_queen (attacks, board, who, coord, change);
            break;

        case PIECE_KING:
            update_king (attacks, board, who, coord, change);
            break;

        default:
            assert (0);
            break;
    }
}

void attack_vector_add (struct attack_vector *attacks, struct board *board, enum color who, coord_t coord,
                        enum piece_type piece)
{
    attack_vector_change (attacks, board, who, coord, piece, +1);
}

void attack_vector_remove (struct attack_vector *attacks, struct board *board, enum color who, coord_t coord,
                           enum piece_type piece)
{
    attack_vector_change (attacks, board, who, coord, piece, -1);
}

uint8_t attack_vector_count (const struct attack_vector *attacks, enum color who, coord_t coord)
{
    uint8_t row = ROW (coord);
    uint8_t col = COLUMN (coord);
    assert (VALID(row) && VALID(col));
    color_index_t cindex = color_index (who);

    uint8_t ne_to_sw_index = ne_to_sw_coord_to_diagonal_table[row][col];
    uint8_t nw_to_se_index = nw_to_se_coord_to_diagonal_table[row][col];

    return attacks->other[cindex][row][col] +
           attacks->ne_to_sw_diagonals[cindex][ne_to_sw_index] +
           attacks->nw_to_se_diagonals[cindex][nw_to_se_index] +
           attacks->horizontals[cindex][row] +
           attacks->verticals[cindex][col];
}

//////////////////////////////

static void update_pawn (struct attack_vector *attacks, struct board *board, enum color who, coord_t at, int change)
{
    int direction = PAWN_DIRECTION(who);
    int next_row = NEXT (ROW(at), direction);
    int left_col = NEXT (COLUMN(at), -1);
    int right_col = NEXT (COLUMN(at), +1);

    if (VALID(left_col))
        attacks->other[color_index(who)][next_row][left_col] += change;

    if (VALID(right_col))
        attacks->other[color_index(who)][next_row][right_col] += change;
}

static void update_knight (struct attack_vector *attacks, struct board *board, enum color who, coord_t at, int change)
{
    const move_list_t *knight_moves = generate_knight_moves (ROW(at), COLUMN(at));
    const move_t *mv;
    color_index_t player_index = color_index(who);

    for_each_move (mv, knight_moves)
    {
        coord_t dst = MOVE_DST(*mv);
        attacks->other[player_index][ROW(dst)][COLUMN(dst)] += change;
    }
}

static void update_bishop (struct attack_vector *attacks, struct board *board, enum color who, coord_t at, int change)
{

}

static void update_rook (struct attack_vector *attacks, struct board *board, enum color who, coord_t at, int change)
{

}

static void update_queen (struct attack_vector *attacks, struct board *board, enum color who, coord_t at, int change)
{
    update_bishop (attacks, board, who, at, change);
    update_rook (attacks, board, who, at, change);
}

static void update_king (struct attack_vector *attacks, struct board *board, enum color who, coord_t at, int change)
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

            attacks->other[index][row][col] += change;
        }
    }
}