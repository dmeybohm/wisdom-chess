#include "attack_vector.h"
#include "generate.h"

#include <memory.h>

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

static void update_nw_to_se (struct attack_vector *attacks, struct board *board, coord_t position)
{
    coord_t start_coord = first_nw_to_se_coord(position);
    uint8_t row, col;
    piece_t piece;
    enum piece_type piece_type;
    enum color piece_color;
    piece_t attacker = PIECE_AND_COLOR_NONE;

    for (row = ROW(start_coord), col = COLUMN(start_coord);
        VALID(row) && VALID(col);
        row = NEXT (row, +1), col = NEXT (col, +1))
    {
        piece = PIECE_AT (board, row, col);
        piece_color = PIECE_COLOR(piece);
        piece_type = PIECE_TYPE(piece);
        attacks->nw_to_se[COLOR_INDEX_BLACK][row][col] = 0;
        attacks->nw_to_se[COLOR_INDEX_WHITE][row][col] = 0;

        if (PIECE_COLOR(attacker) != COLOR_NONE)
            attacks->nw_to_se[color_index(PIECE_COLOR(attacker))][row][col] = 1;

        if (piece_color != COLOR_NONE)
        {
            if (piece_type == PIECE_BISHOP || piece_type == PIECE_QUEEN)
                attacker = piece;
            else
                attacker = PIECE_AND_COLOR_NONE;
        }
    }

    for (row = row - 1, col = col - 1, attacker = PIECE_AND_COLOR_NONE;
        VALID(row) && VALID(col);
        row = NEXT (row, -1), col = NEXT (col, -1))
    {
        piece = PIECE_AT (board, row, col);
        piece_color = PIECE_COLOR(piece);
        piece_type = PIECE_TYPE(piece);

        if (PIECE_COLOR(attacker) != COLOR_NONE)
            attacks->nw_to_se[color_index(PIECE_COLOR(attacker))][row][col] += 1;

        if (piece_color != COLOR_NONE)
        {
            if (piece_type == PIECE_BISHOP || piece_type == PIECE_QUEEN)
                attacker = piece;
            else
                attacker = PIECE_AND_COLOR_NONE;
        }
    }
}

static void update_ne_to_sw (struct attack_vector *attacks, struct board *board, coord_t coord)
{

}

static void update_horizontal (struct attack_vector *attacks, struct board *board, coord_t coord)
{

}

static void update_vertical (struct attack_vector *attacks, struct board *board, coord_t coord)
{

}

static void attack_vector_change (struct attack_vector *attacks, struct board *board, enum color who, coord_t coord,
                                  enum piece_type piece, int change)
{
    update_nw_to_se (attacks, board, coord);
    update_ne_to_sw (attacks, board, coord);
    update_horizontal (attacks, board, coord);
    update_vertical (attacks, board, coord);

    switch (piece)
    {
        case PIECE_PAWN:
            update_pawn (attacks, board, who, coord, change);
            break;

        case PIECE_KNIGHT:
            update_knight (attacks, board, who, coord, change);
            break;

        case PIECE_KING:
            update_king (attacks, board, who, coord, change);
            break;

        default:
            break;
    }
}

//////////////////////////////////////////////////////////////////////

void attack_vector_init (struct attack_vector *attacks, struct board *board)
{
    uint8_t row, col;

    memset (attacks, 0, sizeof(*attacks));

    for_each_position (row, col)
    {
        piece_t piece = PIECE_AT (board, row, col);
        if (PIECE_TYPE(piece) != PIECE_NONE)
            attack_vector_add (attacks, board, PIECE_COLOR(piece), coord_create (row, col), PIECE_TYPE(piece));
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

    return attacks->other[cindex][row][col] +
           attacks->ne_to_sw[cindex][row][col] +
           attacks->nw_to_se[cindex][row][col] +
           attacks->horizontals[cindex][row][col] +
           attacks->verticals[cindex][row][col];
}
