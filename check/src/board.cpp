#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <iostream>

#include "board.h"
#include "debug.h"
#include "validate.h"

// board length in characters
constexpr unsigned int BOARD_LENGTH = 31;

DEFINE_DEBUG_CHANNEL (board, 0);

static void board_init_from_positions (board &board,
                                       const struct board_positions *positions);

static enum piece_type back_rank[] =
{
	PIECE_ROOK,   PIECE_KNIGHT, PIECE_BISHOP, PIECE_QUEEN, PIECE_KING,
	PIECE_BISHOP, PIECE_KNIGHT, PIECE_ROOK, PIECE_LAST
};

static enum piece_type pawns[] =
{
	PIECE_PAWN, PIECE_PAWN, PIECE_PAWN, PIECE_PAWN, PIECE_PAWN,
	PIECE_PAWN, PIECE_PAWN, PIECE_PAWN, PIECE_LAST
};

static struct board_positions init_board[] =
{
	{ 0, COLOR_BLACK, back_rank, },
	{ 1, COLOR_BLACK, pawns,     },
	{ 6, COLOR_WHITE, pawns,     },
	{ 7, COLOR_WHITE, back_rank, },
	{ 0, COLOR_NONE, nullptr }
};

board::board()
{
    board_init_from_positions (*this, init_board);
}

board::board (const struct board_positions *positions)
{
    board_init_from_positions (*this, positions);
}

board::board (const board &board)
{
    *this = board;
}

static castle_state_t init_castle_state (board &board, enum color who)
{
    int row = (who == COLOR_WHITE ? 7 : 0);
    int king_col = 4;
    piece_t prospective_king;
    piece_t prospective_queen_rook;
    piece_t prospective_king_rook;

    castle_state_t state = CASTLE_NONE;

    // todo set CASTLED flag if rook/king in right position:
    prospective_king = PIECE_AT (board, row, king_col);
    prospective_queen_rook = PIECE_AT (board, row, 0);
    prospective_king_rook = PIECE_AT (board, row, 7);

    if (PIECE_TYPE(prospective_king) != PIECE_KING ||
        PIECE_COLOR(prospective_king) != who ||
        PIECE_TYPE(prospective_queen_rook) != PIECE_ROOK ||
        PIECE_COLOR(prospective_queen_rook) != who
    ) {
        state |= CASTLE_QUEENSIDE;
    }
    if (PIECE_TYPE(prospective_king) != PIECE_KING ||
        PIECE_COLOR(prospective_king) != who ||
        PIECE_TYPE(prospective_king_rook) != PIECE_ROOK ||
        PIECE_COLOR(prospective_king_rook) != who
    ) {
        state |= CASTLE_KINGSIDE;
    }

    return state;
}

void board_init_from_positions (board &board, const struct board_positions *positions)
{
    const struct board_positions *ptr;
    uint8_t row, col;
    coord_iterator my_coord_iterator;

    for (uint8_t &i : board.castled)
        i = CASTLE_NONE;

    for (const auto& coord : my_coord_iterator)
        board_set_piece (board, coord, PIECE_AND_COLOR_NONE);

    material_init (&board.material);
    position_init (&board.position);
    
    for (ptr = positions; ptr->pieces != nullptr; ptr++)
    {
        enum piece_type *pieces = ptr->pieces;
        enum color color = ptr->piece_color;
        row = ptr->rank;

        for (col = 0; col < NR_COLUMNS && pieces[col] != PIECE_LAST; col++)
        {
            piece_t new_piece;

            if (pieces[col] == PIECE_NONE)
                continue;

            new_piece = MAKE_PIECE (color, pieces[col]);
            coord_t place = coord_create (row, col);
            board_set_piece (board, place, new_piece);

            material_add (&board.material, new_piece);
            position_add (&board.position, color, place, new_piece);

            if (pieces[col] == PIECE_KING)
                king_position_set (board, color, place);
        }
    }

    board.castled[COLOR_INDEX_WHITE] = init_castle_state (board, COLOR_WHITE);
    board.castled[COLOR_INDEX_BLACK] = init_castle_state (board, COLOR_BLACK);

    board.en_passant_target[COLOR_INDEX_WHITE] = no_en_passant_coord;
    board.en_passant_target[COLOR_INDEX_BLACK] = no_en_passant_coord;

	board_hash_init (&board.hash, board);
}

void board::print_to_file (std::ostream &out) const
{
	out << this->to_string();
	out.flush();
}

void board::print ()
{
	this->print_to_file(std::cout);
}

void board::dump ()
{
    this->print_to_file(std::cerr);
}

board_iterator board::begin () const
{
    return board_iterator { *this, 0, 0 };
}

board_iterator board::end () const
{
    return board_iterator { *this, NR_ROWS, 0 };
}

static void add_divider (std::string &result)
{
    uint8_t col;
    int i;

    result += " ";

    for (col = 0; col < BOARD_LENGTH; col += 4)
    {
        for (i = 0; i < 3; i++)
            result += '-';
        result += ' ';
    }

    result += "\n";
}

static void add_coords (std::string &result)
{
    uint8_t col;

    result += " ";

    char col_name = 'a';
    for (col = 0; col < NR_COLUMNS; col++)
    {
        result += " ";
        result += col_name;
        result += "  ";
        col_name++;
    }

    result += "\n";
}

std::string board::to_string() const
{
    std::string result;
    uint8_t row, col;

    char row_coord = '8';

    add_divider (result);
    for (row = 0; row < NR_ROWS; row++)
    {
        for (col = 0; col < NR_COLUMNS; col++)
        {
            piece_t piece = PIECE_AT (*this, row, col);

            if (!col)
                result += "|";

            if (PIECE_TYPE(piece) != PIECE_NONE &&
                PIECE_COLOR(piece) == COLOR_BLACK)
            {
                result += "*";
            }
            else
            {
                result += " ";
            }

            switch (PIECE_TYPE(piece))
            {
                case PIECE_PAWN:    result += "p"; break;
                case PIECE_KNIGHT:  result += "N"; break;
                case PIECE_BISHOP:  result += "B"; break;
                case PIECE_ROOK:    result += "R"; break;
                case PIECE_QUEEN:   result += "Q"; break;
                case PIECE_KING:    result += "K"; break;
                case PIECE_NONE:    result += " "; break;
                default:            assert (0);  break;
            }

            result += " |";
        }

        result += " ";
        result += row_coord;
        row_coord--;
        result += "\n";

        add_divider (result);;
    }

    add_coords (result);
    return result;
}