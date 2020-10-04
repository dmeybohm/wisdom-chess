#include <memory.h>

#include "board_positions.h"

struct board_positions *board_positions_dup (struct board_positions *positions, size_t size)
{
    struct board_positions *result;

    result = malloc (sizeof(*positions) * (size + 1));

    for (int i = 0; i < size; i++)
    {
        struct board_positions *src_position = &positions[i];
        struct board_positions *dst_position = &result[i];

        *dst_position = *src_position;

        // get length of pieces and duplicate:
        size_t pieces_size = 0;
        enum piece_type *pptr;
        for (pptr = src_position->pieces; *pptr != PIECE_LAST; pptr++, pieces_size++)
        {
            // do nothing;
        }

        dst_position->pieces = malloc (sizeof(*pptr) * (pieces_size + 1));
        memcpy (dst_position->pieces, src_position->pieces, sizeof(*pptr) * (pieces_size + 1));
    }

    // terminate
    positions[size].piece_color = COLOR_NONE;
    positions[size].pieces = NULL;
    positions[size].rank = 0;

    return result;
}

void board_positions_free (struct board_positions *board_positions)
{
    for (struct board_positions *position = board_positions; board_positions->pieces != NULL; board_positions++)
    {
        free (position->pieces);
    }

    free (board_positions);
}