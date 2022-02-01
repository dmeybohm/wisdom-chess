#include <string.h>
#include <ctype.h>

#include "move.h"
#include "coord.h"
#include "board.h"

extern char col_to_char (int col);
extern char row_to_char (int row);

static inline unsigned char char_to_row (char chr)
{
	return 8 - (tolower (chr) - '0');
}

static inline unsigned char char_to_col (char chr)
{
	return tolower (chr) - 'a';
}

char *move_str (move_t move)
{
	coord_t src, dst;
	static char buf[128];

	src = MOVE_SRC (move);
	dst = MOVE_DST (move);

	if (is_castling_move (&move))
	{
		if (COLUMN (dst) - COLUMN (src) < 0)
		{
			/* queenside */
			strcpy (buf, "O-O-O");
		}
		else
		{
			/* kingside */
			strcpy (buf, "O-O");
		}

		return buf;
	}
		
	buf[0] = col_to_char (COLUMN (src));
	buf[1] = row_to_char (ROW (src));
	
	if (is_capture_move (&move))
		buf[2] = 'x';
	else
		buf[2] = ' ';

	buf[3] = col_to_char (COLUMN (dst));
	buf[4] = row_to_char (ROW (dst));
	buf[5] = 0;

	if (is_promoting_move (&move))
		strcat (buf, "(Promoted)");

	return buf;
}

static int castle_parse (char *str, color_t who, move_t *move)
{
	unsigned char src_row, dst_col;

	if (who == COLOR_WHITE)
		src_row = LAST_ROW;
	else if (who == COLOR_BLACK)
		src_row = 0;
	else
		assert (0);

	if (!strncasecmp (str, "O-O-O", strlen ("O-O-O")))
		dst_col = KING_COLUMN - 2;
	else if (!strncasecmp (str, "O-O", strlen ("O-O")))
		dst_col = KING_COLUMN + 2;
	else
		return 0;

	*move = move_create (src_row, KING_COLUMN, src_row, dst_col);

	move_set_castling (move);

	return 1;
}

int move_parse (char *str, color_t who, move_t *move)
{
	int         src_row, src_col;
	int         dst_row, dst_col;
	int         en_passant       = 0;
	piece_t     promoted         = MAKE_PIECE (COLOR_NONE, PIECE_NONE);
	char       *tok;

	if (strlen (str) < 1)
		return 0;

	if (tolower (str[0]) == 'o')
		return castle_parse (str, who, move);

	if (strlen (str) < 4)
		return 0;

	/* convert between row/col and coordinate */
	src_col = char_to_col (str[0]);
	src_row = char_to_row (str[1]);

	dst_col = char_to_col (str[2]);
	dst_row = char_to_row (str[3]);

	tok = strtok (str, " \n\t");
	tok = strtok (NULL, " \n\t");

	*move = move_create (src_row, src_col, dst_row, dst_col);

	if (tok)
	{
		if (!strcasecmp (tok, "ep"))
			en_passant = 1;
		else if (!strcasecmp (tok, "q"))
			promoted = MAKE_PIECE (who, PIECE_QUEEN);
		else if (!strcasecmp (tok, "n"))
			promoted = MAKE_PIECE (who, PIECE_KNIGHT);
		else if (!strcasecmp (tok, "b"))
			promoted = MAKE_PIECE (who, PIECE_BISHOP);
		else if (!strcasecmp (tok, "r"))
			promoted = MAKE_PIECE (who, PIECE_ROOK);
	}

	if (PIECE_TYPE (promoted) != PIECE_NONE)
		*move = move_promote (*move, promoted);

	if (en_passant)
		move_set_en_passant (move);

	if (strtok (NULL, " \n\t"))
		return 0;

	return 1;
}

/* vi: set ts=4 sw=4: */
