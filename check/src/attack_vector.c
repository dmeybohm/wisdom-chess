#include "attack_vector.h"
#include "attack_vector_private.h"
#include "generate.h"
#include "move.h"

#include <memory.h>

static void update_pawn (struct attack_vector *attacks, enum color who, coord_t at, int change)
{
    player_index_t player_index = color_to_player_index(who);

    int direction = PAWN_DIRECTION(who);
    int next_row = NEXT (ROW(at), direction);
    int left_col = NEXT (COLUMN(at), -1);
    int right_col = NEXT (COLUMN(at), +1);

    if (VALID(left_col))
        add_other (attacks, player_index, next_row, left_col, change);

    if (VALID(right_col))
        add_other (attacks, player_index, next_row, right_col, change);
}

static void update_knight (struct attack_vector *attacks, enum color who, coord_t at, int change)
{
    const move_list_t *knight_moves = generate_knight_moves (ROW(at), COLUMN(at));
    const move_t *mv;
    player_index_t player_index = color_to_player_index(who);

    for_each_move (mv, knight_moves)
    {
        coord_t dst = MOVE_DST(*mv);
        add_other (attacks, player_index, ROW(dst), COLUMN(dst), change);
    }
}

static void update_king (struct attack_vector *attacks, enum color who, coord_t at, int change)
{
    uint8_t king_row = ROW(at);
    uint8_t king_col = COLUMN(at);
    uint8_t first_row = NEXT (king_row, -1);
    uint8_t first_col = NEXT (king_col, -1);
    uint8_t last_row = NEXT (king_row, +1);
    uint8_t last_col = NEXT (king_col, +1);
    player_index_t player_index = color_to_player_index(who);

    for (uint8_t row = first_row; row <= last_row; row = NEXT (row, +1))
    {
        for (uint8_t col = first_col; col <= last_col; col = NEXT (col, +1))
        {
            if (row == king_row && col == king_col)
                continue;

            if (!VALID(row) || !VALID(col))
                continue;

            add_other (attacks, player_index, row, col, change);
        }
    }
}

static void update_bishop (struct attack_vector *attacks, enum color color, coord_t coord, int change)
{
    add_to_ne_to_sw_relevant_count (attacks, coord, change);
    add_to_nw_to_se_relevant_count (attacks, coord, change);
}

static void update_rook (struct attack_vector *attacks, enum color color, coord_t coord, int change)
{
    add_to_horizontal_relevant_count (attacks, coord, change);
    add_to_vertical_relevant_count (attacks, coord, change);
}

static void update_nw_to_se (struct attack_vector *attacks, const struct board *board, coord_t position)
{
    coord_t start_coord = first_nw_to_se_coord(position);
    uint8_t row, col;
    piece_t piece;
    enum piece_type piece_type;
    enum color piece_color;
    piece_t attacker = PIECE_AND_COLOR_NONE;

    player_index_t white_index = color_to_player_index(COLOR_WHITE);
    player_index_t black_index = color_to_player_index(COLOR_BLACK);

    for (row = ROW(start_coord), col = COLUMN(start_coord);
        VALID(row) && VALID(col);
        row = NEXT (row, +1), col = NEXT (col, +1))
    {
        piece = PIECE_AT (board, row, col);
        piece_color = PIECE_COLOR(piece);
        piece_type = PIECE_TYPE(piece);
        set_attack_vector (attacks->nw_to_se, white_index, row, col, 0);
        set_attack_vector (attacks->nw_to_se, black_index, row, col, 0);

        if (PIECE_COLOR(attacker) != COLOR_NONE)
        {
            player_index_t attacker_index = color_to_player_index(PIECE_COLOR(attacker));
            set_attack_vector (attacks->nw_to_se, attacker_index, row, col, 1);
        }

        if (piece_color != COLOR_NONE)
        {
            if (piece_type == PIECE_BISHOP || piece_type == PIECE_QUEEN)
                attacker = piece;
            else
                attacker = PIECE_AND_COLOR_NONE;
        }
    }

    attacker = PIECE_AND_COLOR_NONE;

    for (row = row - 1, col = col - 1;
        VALID(row) && VALID(col);
        row = NEXT (row, -1), col = NEXT (col, -1))
    {
        piece = PIECE_AT (board, row, col);
        piece_color = PIECE_COLOR(piece);
        piece_type = PIECE_TYPE(piece);

        if (PIECE_COLOR(attacker) != COLOR_NONE)
        {
            player_index_t attacker_index = color_to_player_index(PIECE_COLOR(attacker));
            add_to_attack_vector (attacks->nw_to_se, attacker_index, row, col, 1);
        }

        if (piece_color != COLOR_NONE)
        {
            if (piece_type == PIECE_BISHOP || piece_type == PIECE_QUEEN)
                attacker = piece;
            else
                attacker = PIECE_AND_COLOR_NONE;
        }
    }
}

static void update_ne_to_sw (struct attack_vector *attacks, const struct board *board, coord_t position)
{
    coord_t start_coord = first_ne_to_sw_coord(position);
    uint8_t row, col;
    piece_t piece;
    enum piece_type piece_type;
    enum color piece_color;
    piece_t attacker = PIECE_AND_COLOR_NONE;

    player_index_t white_index = color_to_player_index(COLOR_WHITE);
    player_index_t black_index = color_to_player_index(COLOR_BLACK);

    for (row = ROW(start_coord), col = COLUMN(start_coord);
         VALID(row) && VALID(col);
         row = NEXT (row, +1), col = NEXT (col, -1))
    {
        piece = PIECE_AT (board, row, col);
        piece_color = PIECE_COLOR(piece);
        piece_type = PIECE_TYPE(piece);

        set_attack_vector (attacks->ne_to_sw, white_index, row, col, 0);
        set_attack_vector (attacks->ne_to_sw, black_index, row, col, 0);

        if (PIECE_COLOR(attacker) != COLOR_NONE)
        {
            player_index_t attacker_index = color_to_player_index(PIECE_COLOR(attacker));
            set_attack_vector (attacks->ne_to_sw, attacker_index, row, col, 1);
        }

        if (piece_color != COLOR_NONE)
        {
            if (piece_type == PIECE_BISHOP || piece_type == PIECE_QUEEN)
                attacker = piece;
            else
                attacker = PIECE_AND_COLOR_NONE;
        }
    }

    attacker = PIECE_AND_COLOR_NONE;

    for (row = row - 1, col = col + 1;
         VALID(row) && VALID(col);
         row = NEXT (row, -1), col = NEXT (col, +1))
    {
        piece = PIECE_AT (board, row, col);
        piece_color = PIECE_COLOR(piece);
        piece_type = PIECE_TYPE(piece);

        if (PIECE_COLOR(attacker) != COLOR_NONE)
        {
            player_index_t attacker_index = color_to_player_index(PIECE_COLOR(attacker));
            add_to_attack_vector (attacks->ne_to_sw, attacker_index, row, col, 1);
        }

        if (piece_color != COLOR_NONE)
        {
            if (piece_type == PIECE_BISHOP || piece_type == PIECE_QUEEN)
                attacker = piece;
            else
                attacker = PIECE_AND_COLOR_NONE;
        }
    }
}

static void update_horizontal (struct attack_vector *attacks, const struct board *board, coord_t coord)
{
    uint8_t row, col;
    piece_t piece;
    enum piece_type piece_type;
    enum color piece_color;
    piece_t attacker = PIECE_AND_COLOR_NONE;

    player_index_t white_index = color_to_player_index(COLOR_WHITE);
    player_index_t black_index = color_to_player_index(COLOR_BLACK);

    for (row = ROW(coord), col = 0; VALID(col); col = NEXT (col, +1))
    {
        piece = PIECE_AT (board, row, col);
        piece_color = PIECE_COLOR(piece);
        piece_type = PIECE_TYPE(piece);

        set_attack_vector (attacks->horizontals, white_index, row, col, 0);
        set_attack_vector (attacks->horizontals, black_index, row, col, 0);

        if (PIECE_COLOR(attacker) != COLOR_NONE)
        {
            player_index_t attacker_index = color_to_player_index(PIECE_COLOR(attacker));
            set_attack_vector (attacks->horizontals, attacker_index, row, col, 1);
        }

        if (piece_color != COLOR_NONE)
        {
            if (piece_type == PIECE_ROOK || piece_type == PIECE_QUEEN)
                attacker = piece;
            else
                attacker = PIECE_AND_COLOR_NONE;
        }
    }

    attacker = PIECE_AND_COLOR_NONE;

    for (row = ROW(coord), col = LAST_COLUMN; VALID(col); col = NEXT (col, -1))
    {
        piece = PIECE_AT (board, row, col);
        piece_color = PIECE_COLOR(piece);
        piece_type = PIECE_TYPE(piece);

        if (PIECE_COLOR(attacker) != COLOR_NONE)
        {
            player_index_t attacker_index = color_to_player_index(PIECE_COLOR(attacker));
            add_to_attack_vector (attacks->horizontals, attacker_index, row, col, 1);
        }

        if (piece_color != COLOR_NONE)
        {
            if (piece_type == PIECE_ROOK || piece_type == PIECE_QUEEN)
                attacker = piece;
            else
                attacker = PIECE_AND_COLOR_NONE;
        }
    }
}

static void update_vertical (struct attack_vector *attacks, const struct board *board, coord_t coord)
{
    uint8_t row, col;
    piece_t piece;
    enum piece_type piece_type;
    enum color piece_color;
    piece_t attacker = PIECE_AND_COLOR_NONE;

    player_index_t white_index = color_to_player_index(COLOR_WHITE);
    player_index_t black_index = color_to_player_index(COLOR_BLACK);

    for (row = 0, col = COLUMN(coord); VALID(row); row = NEXT (row, +1))
    {
        piece = PIECE_AT (board, row, col);
        piece_color = PIECE_COLOR(piece);
        piece_type = PIECE_TYPE(piece);

        set_attack_vector (attacks->verticals, white_index, row, col, 0);
        set_attack_vector (attacks->verticals, black_index, row, col, 0);

        if (PIECE_COLOR(attacker) != COLOR_NONE)
        {
            player_index_t attacker_index = color_to_player_index(PIECE_COLOR(attacker));
            set_attack_vector (attacks->verticals, attacker_index, row, col, 1);
        }

        if (piece_color != COLOR_NONE)
        {
            if (piece_type == PIECE_ROOK || piece_type == PIECE_QUEEN)
                attacker = piece;
            else
                attacker = PIECE_AND_COLOR_NONE;
        }
    }

    attacker = PIECE_AND_COLOR_NONE;

    for (row = LAST_ROW, col = COLUMN(coord); VALID(row); row = NEXT (row, -1))
    {
        piece = PIECE_AT (board, row, col);
        piece_color = PIECE_COLOR(piece);
        piece_type = PIECE_TYPE(piece);

        if (PIECE_COLOR(attacker) != COLOR_NONE)
        {
            player_index_t attacker_index = color_to_player_index(PIECE_COLOR(attacker));
            add_to_attack_vector (attacks->verticals, attacker_index, row, col, 1);
        }

        if (piece_color != COLOR_NONE)
        {
            if (piece_type == PIECE_ROOK || piece_type == PIECE_QUEEN)
                attacker = piece;
            else
                attacker = PIECE_AND_COLOR_NONE;
        }
    }
}

static void attack_vector_change (struct attack_vector *attacks, const struct board *board,
                                  enum color who, coord_t coord, enum piece_type piece, int change)
{
    if (piece == PIECE_BISHOP || piece == PIECE_QUEEN ||
            get_nw_to_se_relevant_count (attacks, coord) > 0)
    {
        update_nw_to_se (attacks, board, coord);
    }

    if (piece == PIECE_BISHOP || piece == PIECE_QUEEN ||
        get_ne_to_sw_relevant_count (attacks, coord) > 0)
    {
        update_ne_to_sw (attacks, board, coord);
    }

    if (piece == PIECE_ROOK || piece == PIECE_QUEEN ||
        get_horizontal_relevant_count (attacks, coord) > 0)
    {
        update_horizontal (attacks, board, coord);
    }

    if (piece == PIECE_ROOK || piece == PIECE_QUEEN ||
        get_vertical_relevant_count (attacks, coord) > 0)
    {
        update_vertical (attacks, board, coord);
    }

    switch (piece)
    {
        case PIECE_PAWN:
            update_pawn (attacks, who, coord, change);
            break;

        case PIECE_KNIGHT:
            update_knight (attacks, who, coord, change);
            break;

        case PIECE_KING:
            update_king (attacks, who, coord, change);
            break;

        case PIECE_BISHOP:
            update_bishop (attacks, who, coord, change);
            break;

        case PIECE_ROOK:
            update_rook (attacks, who, coord, change);
            break;

        case PIECE_QUEEN:
            update_bishop (attacks, who, coord, change);
            update_rook (attacks, who, coord, change);
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

void attack_vector_do_move (struct attack_vector *attacks, struct board *board, enum color color,
                            enum piece_type piece_type, move_t *move)
{
    coord_t src = MOVE_SRC(*move);
    coord_t dst = MOVE_DST(*move);

    attack_vector_remove (attacks, board, color, src, piece_type);

    if (is_capture_move(move))
    {
        piece_t piece = move_get_taken(move);

        attack_vector_remove (attacks, board, PIECE_COLOR(piece), dst, PIECE_TYPE(piece));
    }

    attack_vector_add (attacks, board, color, dst, piece_type);
}

void attack_vector_undo_move (struct attack_vector *attacks, struct board *board, enum color color,
                              enum piece_type piece_type, move_t *move)
{
    coord_t src = MOVE_SRC(*move);
    coord_t dst = MOVE_DST(*move);

    attack_vector_remove (attacks, board, color, dst, piece_type);

    if (is_capture_move(move))
    {
        piece_t piece = move_get_taken(move);

        attack_vector_add (attacks, board, PIECE_COLOR(piece), dst, PIECE_TYPE(piece));
    }

    attack_vector_add (attacks, board, color, src, piece_type);
}

uint8_t attack_vector_count (const struct attack_vector *attacks, enum color who, coord_t coord)
{
    uint8_t row = ROW(coord);
    uint8_t col = COLUMN(coord);
    assert (VALID(row) && VALID(col));
    player_index_t player_index = color_to_player_index(who);

    return get_other (attacks, player_index, row, col) +
           get_attack_vector (attacks->nw_to_se, player_index, row, col) +
           get_attack_vector (attacks->ne_to_sw, player_index, row, col) +
           get_attack_vector (attacks->horizontals, player_index, row, col) +
           get_attack_vector (attacks->verticals, player_index, row, col);
}
