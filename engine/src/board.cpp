#include "board.hpp"
#include "validate.hpp"
#include "variation_glimpse.hpp"

#include <iostream>

namespace wisdom
{
    // board length in characters
    constexpr int BOARD_LENGTH = 31;

    std::vector<BoardPositions> initial_board_position ()
    {
        std::vector<Piece> back_rank = {
                Piece::Rook, Piece::Knight, Piece::Bishop, Piece::Queen, Piece::King,
                Piece::Bishop, Piece::Knight, Piece::Rook,
        };

        std::vector<Piece> pawns = {
                Piece::Pawn, Piece::Pawn, Piece::Pawn, Piece::Pawn, Piece::Pawn,
                Piece::Pawn, Piece::Pawn, Piece::Pawn,
        };

        std::vector<BoardPositions> init_board = {
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

    static CastlingState init_castle_state (Board &board, Color who)
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

    Board::Board (const std::vector<BoardPositions> &positions)
    {
        int row;

        for (CastlingState &i : this->castled)
            i = Castle_None;

        for (const auto &coord : All_Coords_Iterator)
            board_set_piece (*this, coord, Piece_And_Color_None);

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
                Coord place = make_coord (row, static_cast<int>(col));
                board_set_piece (*this, place, new_piece);

                this->material.add (new_piece);
                this->position.add (color, place, new_piece);

                if (pieces[col] == Piece::King)
                    king_position_set (*this, color, place);
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

    static void add_divider (std::string &result)
    {
        int col;
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

    std::string Board::to_string () const
    {
        std::string result;
        int row, col;

        char row_coord = '8';

        add_divider (result);
        for (row = 0; row < Num_Rows; row++)
        {
            for (col = 0; col < Num_Columns; col++)
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

    void Board::add_evaluation_to_transposition_table (int score, Color who, int relative_depth,
                                                       const VariationGlimpse &variation_glimpse)
    {
        RelativeTransposition evaluation { *this, score, relative_depth, variation_glimpse };
        my_transpositions.add (evaluation, who);
    }

    // If the relative depth of the looked up transposition has been searched deeper than the
    // depth we're looking for, then return the transposition.
    std::optional<RelativeTransposition> Board::check_transposition_table (Color who, int relative_depth)
    {
        auto optional_transposition = my_transpositions.lookup (this->code.hash_code (), who);

        if (optional_transposition.has_value ())
        {
            // Check the board codes are equal:
            if (optional_transposition->board_code != this->code)
            {
                my_transposition_dupe_hashes++;
                return std::nullopt;
            }

            if (optional_transposition->relative_depth >= relative_depth)
            {
                my_transposition_misses++;
                return optional_transposition;
            }
        }

        my_transposition_hits++;
        return std::nullopt;
    }

    [[nodiscard]] std::string Board::castled_string (Color color) const
    {
        ColorIndex index = color_index (color);
        std::string castled_state;

        auto convert = [color](char ch) -> char {
            return color == Color::Black ? static_cast<char> (tolower(ch)) : ch;
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

    [[nodiscard]] std::string Board::to_fen_string (Color turn) const
    {
        std::string output;

        for (int row = 0; row < Num_Rows; row++)
        {
            std::string row_string;
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
                    char ch = static_cast<char> (toupper (piece_char (piece)));
                    if (piece_color(piece) == Color::Black)
                        ch = static_cast<char> (tolower(ch));
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

        std::string castled_white = castled_string (Color::White);
        std::string castled_black = castled_string (Color::Black);

        std::string castled = castled_white + castled_black;
        if (castled.length() == 0)
            castled = "-";

        output += castled;

        if (this->en_passant_target[Color_Index_White] != No_En_Passant_Coord)
            output += " " + wisdom::to_string (this->en_passant_target[Color_Index_White]) + " ";
        else if (this->en_passant_target[Color_Index_Black] != No_En_Passant_Coord)
            output += " " + wisdom::to_string (this->en_passant_target[Color_Index_Black]) + " ";
        else
            output += " - ";

        output += std::to_string (this->half_move_clock) + " " + std::to_string (this->full_moves);
        return output;
    }

}