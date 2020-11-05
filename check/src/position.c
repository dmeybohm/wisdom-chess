#include "position.h"
#include "board.h"

static int knight_positions[NR_ROWS][NR_COLUMNS] = {
        { -5, -4, -3, -3, -3, -3, -4, -5  },
        { -4, -2,  0,  0,  0,  0, -2, -4  },
        { -1,  0,  1,  2,  2,  1,  0, -6  },
        { -1,  1,  0,  3,  3,  0,  1, -6  },
        { -2,  0,  2,  3,  3,  2,  0, -3  },
        { -2,  1,  2,  2,  2,  2,  1, -3  },
        { -4, -2,  0,  1,  1,  0,  0, -4  },
        { -5, -4, -3, -3, -3,  0, -4, -5  },
};

static int bishop_positions[NR_ROWS][NR_COLUMNS] = {
        {  0,  0,  0,  0,  0,  0,  0,  0  },
        {  0,  0,  0,  0,  0,  0,  0,  0  },
        {  0,  0,  0,  0,  0,  0,  0,  0  },
        {  0,  0,  2,  0,  0,  2,  0,  0  },
        {  0,  0,  3,  0,  0,  3,  0,  0  },
        {  0,  0,  0,  0,  0,  0,  0,  0  },
        {  0,  1,  0,  0,  0,  0,  1,  0  },
        {  0,  0,  0,  0,  0,  0,  0,  0  },
};

static int rook_positions[NR_ROWS][NR_COLUMNS] = {
        {  0,  0,  0,  0,  0,  0,  0,  0  },
        {  7,  8,  8,  8,  8,  8,  8,  7  },
        {  0,  0,  0,  0,  0,  0,  0,  0  },
        {  0,  0,  0,  0,  0,  0,  0,  0  },
        {  0,  0,  0,  0,  0,  0,  0,  0  },
        {  0,  0,  0,  0,  0,  0,  0,  0  },
        {  0,  0,  0,  0,  0,  0,  0,  0  },
        {  0,  0,  0,  0,  0,  0,  0,  0  },
};

static int pawn_positions[NR_ROWS][NR_COLUMNS] = {
        {  0,  0,  0,  0,  0,  0,  0,  0  },
        {  9,  9,  9,  9,  9,  9,  9,  9  },
        {  0,  0,  1,  5,  5,  1,  0,  0  },
        {  0,  0,  5,  8,  8,  5,  0,  0  },
        {  0,  0,  5,  8,  8,  5,  0,  0  },
        {  0,  0,  1,  5,  5,  1,  0,  0  },
        {  0,  0,  0,  0,  0,  0,  0,  0  },
        {  0,  0,  0,  0,  0,  0,  0,  0  },
};

static int king_positions[NR_ROWS][NR_COLUMNS] = {
        {  0,  0,  0,  0,  0,  0,  0,  0  },
        {  0,  0,  0,  0,  0,  0,  0,  0  },
        {  0,  0,  0,  0,  0,  0,  0,  0  },
        { -3, -4, -4, -5, -5, -4, -4, -3  },
        { -2, -3, -3, -4, -4, -3, -3, -2  },
        { -1, -2, -2, -2, -2, -2, -2, -1  },
        {  2,  2,  0,  0,  0,  0,  0,  0  },
        {  2,  3,  1,  0,  0,  1,  3,  2  },
};

static int queen_positions[NR_ROWS][NR_COLUMNS] = {
        {  0,  0,  0,  0,  0,  0,  0,  0  },
        {  7,  8,  8,  8,  8,  8,  8,  7  },
        {  0,  0,  0,  0,  0,  0,  0,  0  },
        {  0,  0,  0,  0,  0,  0,  0,  0  },
        {  0,  0,  0,  0,  0,  0,  0,  0  },
        {  0,  0,  0,  0,  0,  0,  0,  0  },
        {  0,  0,  0,  0,  0,  0,  0,  0  },
        {  0,  0,  0,  0,  0,  0,  0,  0  },
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

void position_add (struct position *position, color_t who, coord_t coord,piece_t piece)
{
    color_index_t index = color_index(who);
    position->score[index] += change (coord, who, piece);
}

void position_remove (struct position *position, color_t who, coord_t coord, piece_t piece)
{
    color_index_t index = color_index(who);
    position->score[index] -= change (coord, who, piece);
}

void position_do_move (struct position *position, color_t color,
                       piece_t piece, move_t *move)
{
    coord_t src = MOVE_SRC(*move);
    coord_t dst = MOVE_DST(*move);

    position_remove (position, color, src, piece);

    if (is_capture_move(move))
    {
        piece_t taken_piece = move_get_taken(move);
        coord_t taken_piece_coord = dst;

        if (is_en_passant_move(move))
            taken_piece_coord = en_passant_taken_pawn_coord (src, dst);

        position_remove (position, PIECE_COLOR(piece), taken_piece_coord, taken_piece);
    }

    if (is_castling_move(move))
    {
        uint8_t rook_src_row = castling_row_from_color(color);
        uint8_t rook_src_col = is_castling_move_on_king_side(move) ?
                               KING_ROOK_COLUMN : QUEEN_ROOK_COLUMN;
        uint8_t rook_dst_col = is_castling_move_on_king_side(move) ?
                               KING_CASTLED_ROOK_COLUMN : QUEEN_CASTLED_ROOK_COLUMN;

        coord_t src_rook_coord = coord_create (rook_src_row, rook_src_col);
        coord_t dst_rook_coord = coord_create (rook_src_row, rook_dst_col);
        piece_t rook = MAKE_PIECE (color, PIECE_ROOK);

        position_remove (position, color, src_rook_coord, rook);
        position_remove (position, color, dst_rook_coord, rook);
    }

    piece_t dst_piece = is_promoting_move(move) ?
                                     move_get_promoted(move) : piece;

    position_add (position, color, dst, dst_piece);
}

void position_undo_move (struct position *position, color_t color,
                         piece_t piece, move_t *move)
{
    coord_t src = MOVE_SRC(*move);
    coord_t dst = MOVE_DST(*move);

    piece_t dst_piece = is_promoting_move(move) ?
                                     move_get_promoted(move) : piece;

    position_remove (position, color, dst, dst_piece);

    if (is_capture_move(move))
    {
        piece_t taken_piece = move_get_taken(move);
        coord_t taken_piece_coord = dst;

        if (is_en_passant_move(move))
            taken_piece_coord = en_passant_taken_pawn_coord (src, dst);

        position_add (position, PIECE_COLOR(piece), taken_piece_coord, taken_piece);
    }

    if (is_castling_move(move))
    {
        uint8_t rook_src_row = castling_row_from_color(color);
        uint8_t rook_src_col = is_castling_move_on_king_side(move) ?
                               KING_ROOK_COLUMN : QUEEN_ROOK_COLUMN;
        uint8_t rook_dst_col = is_castling_move_on_king_side(move) ?
                               KING_CASTLED_ROOK_COLUMN : QUEEN_CASTLED_ROOK_COLUMN;

        coord_t src_rook_coord = coord_create (rook_src_row, rook_src_col);
        coord_t dst_rook_coord = coord_create (rook_src_row, rook_dst_col);
        piece_t rook = MAKE_PIECE (color, PIECE_ROOK);
        position_remove (position, color, dst_rook_coord, rook);
        position_add (position, color, src_rook_coord, rook);
    }

    position_add (position, color, src, piece);
}

int position_score (struct position *position, color_t who)
{
    color_index_t index = color_index(who);
    color_index_t inverted = color_index(color_invert(who));
    return position->score[index] - position->score[inverted];
}