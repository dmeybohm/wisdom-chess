#ifndef EVOLVE_CHESS_COORD_H
#define EVOLVE_CHESS_COORD_H

/* lower three bits are the column, upper three are the row */
typedef unsigned char coord_t;

/* three bits for the row and column each */
#define COORD_MASK   (0x08-1)
#define COORD_SHIFT  3

static inline unsigned char ROW (coord_t pos)
{
	return ((unsigned char) pos >> COORD_SHIFT) & COORD_MASK;
}

static inline unsigned char COLUMN (coord_t pos)
{
	return ((unsigned char) pos) & COORD_MASK;
}

static inline coord_t coord_create (unsigned char row, unsigned char col)
{
	return (row << COORD_SHIFT) | (col & COORD_MASK);
}

#endif /* EVOLVE_CHESS_COORD_H */
