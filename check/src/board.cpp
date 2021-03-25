#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <iostream>

#include "board.hpp"
#include "validate.hpp"

namespace wisdom
{


// board length in characters
    constexpr int BOARD_LENGTH = 31;

    static void board_init_from_positions (Board &board, const std::vector<BoardPositions> &positions);

    std::vector<BoardPositions> initial_board_position ()
    {
        std::vector<Piece> back_rank =
                {
                        Piece::Rook, Piece::Knight, Piece::Bishop, Piece::Queen, Piece::King,
                        Piece::Bishop, Piece::Knight, Piece::Rook,
                };

        std::vector<Piece> pawns =
                {
                        Piece::Pawn, Piece::Pawn, Piece::Pawn, Piece::Pawn, Piece::Pawn,
                        Piece::Pawn, Piece::Pawn, Piece::Pawn,
                };

        std::vector<BoardPositions> init_board =
                {
                        { 0, Color::Black, back_rank, },
                        { 1, Color::Black, pawns, },
                        { 6, Color::White, pawns, },
                        { 7, Color::White, back_rank, },
                };

        return init_board;
    }

    Board::Board ()
    {
        board_init_from_positions (*this, initial_board_position ());
    }

    Board::Board (const std::vector<BoardPositions> &positions)
    {
        board_init_from_positions (*this, positions);
    }

    static CastlingState init_castle_state (Board &board, Color who)
    {
        int row = (who == Color::White ? 7 : 0);
        int king_col = 4;
        ColoredPiece prospective_king;
        ColoredPiece prospective_queen_rook;
        ColoredPiece prospective_king_rook;

        CastlingState state = CASTLE_NONE;

        // todo set CASTLED flag if rook/king in right position:
        prospective_king = piece_at (board, row, king_col);
        prospective_queen_rook = piece_at (board, row, 0);
        prospective_king_rook = piece_at (board, row, 7);

        if (piece_type (prospective_king) != Piece::King ||
            piece_color (prospective_king) != who ||
            piece_type (prospective_queen_rook) != Piece::Rook ||
            piece_color (prospective_queen_rook) != who
                )
        {
            state |= CASTLE_QUEENSIDE;
        }
        if (piece_type (prospective_king) != Piece::King ||
            piece_color (prospective_king) != who ||
            piece_type (prospective_king_rook) != Piece::Rook ||
            piece_color (prospective_king_rook) != who
                )
        {
            state |= CASTLE_KINGSIDE;
        }

        return state;
    }

    static void board_init_from_positions (Board &board, const std::vector<BoardPositions> &positions)
    {
        int8_t row;

        for (CastlingState &i : board.castled)
            i = CASTLE_NONE;

        for (const auto &coord : All_Coords_Iterator)
            board_set_piece (board, coord, piece_and_color_none);

        position_init (&board.position);

        for (auto &position : positions)
        {
            auto &pieces = position.pieces;
            Color color = position.piece_color;
            row = position.rank;

            for (uint8_t col = 0; col < Num_Columns && col < pieces.size (); col++)
            {
                ColoredPiece new_piece;

                if (pieces[col] == Piece::None)
                    continue;

                new_piece = make_piece (color, pieces[col]);
                Coord place = make_coord (row, col);
                board_set_piece (board, place, new_piece);

                board.material.add (new_piece);
                position_add (&board.position, color, place, new_piece);

                if (pieces[col] == Piece::King)
                    king_position_set (board, color, place);
            }
        }

        board.castled[COLOR_INDEX_WHITE] = init_castle_state (board, Color::White);
        board.castled[COLOR_INDEX_BLACK] = init_castle_state (board, Color::Black);

        board.en_passant_target[COLOR_INDEX_WHITE] = No_En_Passant_Coord;
        board.en_passant_target[COLOR_INDEX_BLACK] = No_En_Passant_Coord;

        board.code = BoardCode { board };
    }

    void Board::print_to_file (std::ostream &out) const
    {
        out << this->to_string ();
        out.flush ();
    }

    void Board::print () const
    {
        this->print_to_file (std::cout);
    }

    void Board::dump () const
    {
        this->print_to_file (std::cerr);
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
        for (col = 0; col < Num_Columns; col++)
        {
            result += " ";
            result += col_name;
            result += "  ";
            col_name++;
        }

        result += "\n";
    }

    std::string Board::to_string () const
    {
        std::string result;
        int8_t row, col;

        char row_coord = '8';

        add_divider (result);
        for (row = 0; row < Num_Rows; row++)
        {
            for (col = 0; col < Num_Columns; col++)
            {
                ColoredPiece piece = piece_at (*this, row, col);

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
                    case Piece::Pawn: result += "p";
                        break;
                    case Piece::Knight: result += "N";
                        break;
                    case Piece::Bishop: result += "B";
                        break;
                    case Piece::Rook: result += "R";
                        break;
                    case Piece::Queen: result += "Q";
                        break;
                    case Piece::King: result += "K";
                        break;
                    case Piece::None: result += " ";
                        break;
                    default: assert (0);
                        break;
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
}