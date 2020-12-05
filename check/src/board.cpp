#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <iostream>

#include "board.h"
#include "debug.h"
#include "validate.h"

// board length in characters
constexpr int BOARD_LENGTH = 31;

DEFINE_DEBUG_CHANNEL (board, 0);

static void board_init_from_positions (board &board,
                                       const struct board_positions *positions);

static void board_init_from_positions (board &board, const std::vector<board_positions> &positions);

std::vector<board_positions> initial_board_position ()
{
    std::vector<Piece> back_rank =
    {
        Piece::Rook,   Piece::Knight, Piece::Bishop, Piece::Queen, Piece::King,
        Piece::Bishop, Piece::Knight, Piece::Rook,
    };

    std::vector<Piece> pawns =
    {
        Piece::Pawn, Piece::Pawn, Piece::Pawn, Piece::Pawn, Piece::Pawn,
        Piece::Pawn, Piece::Pawn, Piece::Pawn,
    };

    std::vector<board_positions> init_board =
    {
        { 0, Color::Black, back_rank, },
        { 1, Color::Black, pawns,     },
        { 6, Color::White, pawns,     },
        { 7, Color::White, back_rank, },
    };

    return init_board;
}

board::board()
{
    board_init_from_positions (*this, initial_board_position());
}

board::board (const std::vector<board_positions> &positions)
{
    board_init_from_positions (*this, positions);
}

board::board (const board &board)
{
    *this = board;
}

static castle_state_t init_castle_state (board &board, Color who)
{
    int row = (who == Color::White ? 7 : 0);
    int king_col = 4;
    piece_t prospective_king;
    piece_t prospective_queen_rook;
    piece_t prospective_king_rook;

    castle_state_t state = CASTLE_NONE;

    // todo set CASTLED flag if rook/king in right position:
    prospective_king = piece_at (board, row, king_col);
    prospective_queen_rook = piece_at (board, row, 0);
    prospective_king_rook = piece_at (board, row, 7);

    if (piece_type (prospective_king) != Piece::King ||
            piece_color (prospective_king) != who ||
            piece_type (prospective_queen_rook) != Piece::Rook ||
            piece_color (prospective_queen_rook) != who
    ) {
        state |= CASTLE_QUEENSIDE;
    }
    if (piece_type (prospective_king) != Piece::King ||
            piece_color (prospective_king) != who ||
            piece_type (prospective_king_rook) != Piece::Rook ||
            piece_color (prospective_king_rook) != who
    ) {
        state |= CASTLE_KINGSIDE;
    }

    return state;
}

static void board_init_from_positions (board &board, const std::vector<board_positions> &positions)
{
    int8_t row;

    for (int8_t &i : board.castled)
        i = CASTLE_NONE;

    for (const auto& coord : all_coords_iterator)
        board_set_piece (board, coord, piece_and_color_none);

    position_init (&board.position);

    for (auto &position : positions)
    {
        auto &pieces = position.pieces;
        Color color = position.piece_color;
        row = position.rank;

        for (uint8_t col = 0; col < NR_COLUMNS && col < pieces.size(); col++)
        {
            piece_t new_piece;

            if (pieces[col] == Piece::None)
                continue;

            new_piece = make_piece (color, pieces[col]);
            coord_t place = coord_create (row, col);
            board_set_piece (board, place, new_piece);

            board.material.add (new_piece);
            position_add (&board.position, color, place, new_piece);

            if (pieces[col] == Piece::King)
                king_position_set (board, color, place);
        }
    }

    board.castled[COLOR_INDEX_WHITE] = init_castle_state (board, Color::White);
    board.castled[COLOR_INDEX_BLACK] = init_castle_state (board, Color::Black);

    board.en_passant_target[COLOR_INDEX_WHITE] = no_en_passant_coord;
    board.en_passant_target[COLOR_INDEX_BLACK] = no_en_passant_coord;

    board_hash_init (board);
}

void board::print_to_file (std::ostream &out) const
{
	out << this->to_string();
	out.flush();
}

void board::print () const
{
	this->print_to_file(std::cout);
}

void board::dump () const
{
    this->print_to_file(std::cerr);
}

static void add_divider (std::string &result)
{
    int8_t col;
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
    int8_t col;

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
    int8_t row, col;

    char row_coord = '8';

    add_divider (result);
    for (row = 0; row < NR_ROWS; row++)
    {
        for (col = 0; col < NR_COLUMNS; col++)
        {
            piece_t piece = piece_at (*this, row, col);

            if (!col)
                result += "|";

            if (piece_type (piece) != Piece::None &&
                    piece_color (piece) == Color::Black)
            {
                result += "*";
            }
            else
            {
                result += " ";
            }

            switch (piece_type (piece))
            {
                case Piece::Pawn:    result += "p"; break;
                case Piece::Knight:  result += "N"; break;
                case Piece::Bishop:  result += "B"; break;
                case Piece::Rook:    result += "R"; break;
                case Piece::Queen:   result += "Q"; break;
                case Piece::King:    result += "K"; break;
                case Piece::None:    result += " "; break;
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