#include "piece.h"

char *piece_str (piece_t piece)
{
	static char buf[16];

	if (PIECE_TYPE (piece) != PIECE_NONE)
	{
		if (PIECE_COLOR (piece) == COLOR_WHITE)
			buf[0] = 'W', buf[1] = 'h'; 
		else
			buf[0] = 'B', buf[1] = 'l';
		buf[2] = ' ';
		
	}

	switch (PIECE_TYPE (piece))
	{
	 case PIECE_KING: buf[3] = 'K'; break;
	 case PIECE_QUEEN: buf[3] = 'Q'; break;
	 case PIECE_ROOK: buf[3] = 'R'; break;
	 case PIECE_KNIGHT: buf[3] = 'N'; break;
	 case PIECE_BISHOP: buf[3] = 'B'; break;
	 case PIECE_PAWN: buf[3] = 'P'; break;
	 case PIECE_NONE: buf[3] = '*'; break;

	 default: buf[3] = 'X'; break;
	}

	buf[4] = 0;

	return buf;
}

/* vi: set ts=4 sw=4: */
