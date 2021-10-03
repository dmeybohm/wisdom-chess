#include "board.hpp"
#include "validate.hpp"
#include "variation_glimpse.hpp"

#include <iostream>
#include <ostream>

namespace wisdom
{
    // board length in characters
    constexpr int BOARD_LENGTH = 31;

    auto initial_board_position () -> vector<BoardPositions>
    {
        vector<Piece> back_rank = {
                Piece::Rook, Piece::Knight, Piece::Bishop, Piece::Queen, Piece::King,
                Piece::Bishop, Piece::Knight, Piece::Rook,
        };

        vector<Piece> pawns = {
                Piece::Pawn, Piece::Pawn, Piece::Pawn, Piece::Pawn, Piece::Pawn,
                Piece::Pawn, Piece::Pawn, Piece::Pawn,
        };

        vector<BoardPositions> init_board = {
                { 0, Color::Black, back_rank, },
                { 1, Color::Black, pawns, },
                { 6, Color::White, pawns, },
                { 7, Color::White, back_rank, },
        };

        return init_board;
    }

    Board::Board () : Board { initial_board_position() }
    {
    }

    static auto init_castle_state (Board &board, Color who) -> CastlingState
    {
        int row = (who == Color::White ? 7 : 0);
        int king_col = 4;
        ColoredPiece prospective_king;
        ColoredPiece prospective_queen_rook;
        ColoredPiece prospective_king_rook;

        CastlingState state = Castle_None;

        prospective_king = piece_at (board, row, king_col);
        prospective_queen_rook = piece_at (board, row, 0);
        prospective_king_rook = piece_at (board, row, 7);

        if (piece_type (prospective_king) != Piece::King ||
            piece_color (prospective_king) != who ||
            piece_type (prospective_queen_rook) != Piece::Rook ||
            piece_color (prospective_queen_rook) != who)
        {
            state |= Castle_Queenside;
        }
        if (piece_type (prospective_king) != Piece::King ||
            piece_color (prospective_king) != who ||
            piece_type (prospective_king_rook) != Piece::Rook ||
            piece_color (prospective_king_rook) != who)
        {
            state |= Castle_Kingside;
        }

        return state;
    }

    Board::Board (const vector<BoardPositions> &positions)
    {
        int row;

        for (CastlingState &i : this->castled)
            i = Castle_None;

        for (const auto &coord : All_Coords_Iterator)
            set_piece (coord, Piece_And_Color_None);

        this->position = Position {};

        for (auto &pos : positions)
        {
            auto &pieces = pos.pieces;
            Color color = pos.piece_color;
            row = pos.rank;

            for (std::size_t col = 0; col < Num_Columns && col < pieces.size (); col++)
            {
                ColoredPiece new_piece;

                if (pieces[col] == Piece::None)
                    continue;

                new_piece = make_piece (color, pieces[col]);
                Coord place = make_coord (row, gsl::narrow<int>(col));
                set_piece ( place, new_piece);

                this->material.add (new_piece);
                this->position.add (color, place, new_piece);

                if (pieces[col] == Piece::King)
                    this->set_king_position (color, place);
            }
        }

        this->castled[Color_Index_White] = init_castle_state (*this, Color::White);
        this->castled[Color_Index_Black] = init_castle_state (*this, Color::Black);

        this->en_passant_target[Color_Index_White] = No_En_Passant_Coord;
        this->en_passant_target[Color_Index_Black] = No_En_Passant_Coord;

        this->code = BoardCode { *this };
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

    static void add_divider (string &result)
    {
        result += " ";

        for (int col = 0; col < BOARD_LENGTH; col += 4)
        {
            for (int i = 0; i < 3; i++)
                result += '-';
            result += ' ';
        }

        result += "\n";
    }

    static void add_coords (string &result)
    {
        int col;

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

    auto Board::to_string () const -> string
    {
        string result;

        char row_coord = '8';

        add_divider (result);
        for (int row = 0; row < Num_Rows; row++)
        {
            for (int col = 0; col < Num_Columns; col++)
            {
                ColoredPiece piece = this->piece_at (row, col);

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

    [[nodiscard]] auto Board::castled_string (Color color) const -> string
    {
        ColorIndex index = color_index (color);
        string castled_state;

        auto convert = [color](char ch) -> char {
            return color == Color::Black ? gsl::narrow_cast<char> (tolower(ch)) : ch;
        };

        if (this->castled[index] == Castle_Castled)
            castled_state = "";
        else if (this->castled[index] == Castle_None)
            castled_state.append(1, convert('K')), castled_state.append(1, convert('Q'));
        else if (this->castled[index] == Castle_Kingside)
            castled_state += "Q";
        else if (this->castled[index] == Castle_Queenside)
            castled_state += "K";
        else
            castled_state += "";

        return castled_state;
    }

    [[nodiscard]] auto Board::to_fen_string (Color turn) const -> string
    {
        string output;

        for (int row = 0; row < Num_Rows; row++)
        {
            string row_string;
            int none_count = 0;

            for (int col = 0; col < Num_Columns; col++)
            {
                ColoredPiece piece = this->piece_at (row, col);
                if (piece == Piece_And_Color_None)
                {
                    none_count++;
                }
                else
                {
                    if (none_count > 0)
                        row_string += std::to_string (none_count);
                    none_count = 0;
                    char ch = gsl::narrow_cast<char> (toupper (piece_char (piece)));
                    if (piece_color(piece) == Color::Black)
                        ch = gsl::narrow_cast<char> (tolower(ch));
                    row_string.append(1, ch);
                }
            }

            if (none_count > 0)
                row_string += std::to_string (none_count);

            if (row < 7)
                row_string += "/";

            output += row_string;
        }

        output += turn == Color::White ? " w " : " b ";

        string castled_white = castled_string (Color::White);
        string castled_black = castled_string (Color::Black);

        string both_castled = castled_white + castled_black;
        if (both_castled.length() == 0)
            both_castled = "-";

        output += both_castled;

        if (this->en_passant_target[Color_Index_White] != No_En_Passant_Coord)
            output += " " + wisdom::to_string (this->en_passant_target[Color_Index_White]) + " ";
        else if (this->en_passant_target[Color_Index_Black] != No_En_Passant_Coord)
            output += " " + wisdom::to_string (this->en_passant_target[Color_Index_Black]) + " ";
        else
            output += " - ";

        output += std::to_string (this->half_move_clock) + " " + std::to_string (this->full_moves);
        return output;
    }

    bool board_equals (const Board &a, const Board &b)
    {
        for (auto coord : All_Coords_Iterator)
        {
            if (a.piece_at (coord) != b.piece_at (coord))
                return false;
        }

        // todo check more
        return true;
    }

    auto Board::get_code () const& -> const BoardCode&
    {
        return code;
    }


}
