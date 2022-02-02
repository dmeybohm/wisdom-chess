#include "board.hpp"
#include "validate.hpp"
#include "variation_glimpse.hpp"
#include "check.hpp"

#include <iostream>
#include <ostream>
#include <random>
#include <algorithm>

namespace wisdom
{
    // board length in characters
    constexpr int BOARD_LENGTH = 31;

    static int swap_pawns (const Board& board, int source_row, int source_col,
                           Color promote_color, int new_row, int new_col,
                           array<ColoredPiece, Num_Columns * Num_Rows>& shuffle_pieces,
                           int swaps)
    {
        auto last_row_piece = shuffle_pieces[source_col + (source_row * Num_Columns)];

        if (piece_type (last_row_piece) == Piece::Pawn)
        {
            if (piece_color (last_row_piece) == promote_color)
            {
                auto removed = Piece_And_Color_None;
                shuffle_pieces[source_col + (source_row * Num_Columns)] = removed;
            }
            else
            {
                std::swap (shuffle_pieces[source_col + source_row * Num_Columns],
                           shuffle_pieces[new_col + new_row * Num_Columns]);
                swaps++;
            }
        }
        return swaps;
    }

    auto Board::initial_board_position () -> vector<BoardPositions>
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
        int8_t row = (who == Color::White ? Last_Row : First_Row);
        int8_t king_col = King_Column;

        CastlingState state = Castle_None;
        ColoredPiece prospective_king = board.piece_at (row, king_col);
        ColoredPiece prospective_queen_rook = board.piece_at (row, 0);
        ColoredPiece prospective_king_rook = board.piece_at (row, 7);

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

    Board::Board (const vector<BoardPositions>& positions)
    {
        int8_t row;

        for (CastlingState& i : this->my_castled)
            i = Castle_None;

        int8_t col;
        FOR_EACH_ROW_AND_COL(row, col)
            set_piece (make_coord (row, col), Piece_And_Color_None);

        this->my_position = Position {};

        for (auto& pos : positions)
        {
            auto &pieces = pos.pieces;
            Color color = pos.piece_color;
            row = pos.rank;

            for (col = 0; col < Num_Columns && col < pieces.size (); col++)
            {
                if (pieces[col] == Piece::None)
                    continue;

                ColoredPiece new_piece = make_piece (color, pieces[col]);
                Coord place = make_coord (row, col);
                set_piece (place, new_piece);

                this->my_material.add (new_piece);
                this->my_position.add (color, place, new_piece);

                if (pieces[col] == Piece::King)
                    this->set_king_position (color, place);
            }
        }

        this->my_castled[Color_Index_White] = init_castle_state (*this, Color::White);
        this->my_castled[Color_Index_Black] = init_castle_state (*this, Color::Black);

        this->my_en_passant_target[Color_Index_White] = No_En_Passant_Coord;
        this->my_en_passant_target[Color_Index_Black] = No_En_Passant_Coord;

        this->my_code = BoardCode { *this };
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
        for (int8_t row = 0; row < Num_Rows; row++)
        {
            for (int8_t col = 0; col < Num_Columns; col++)
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

        if (this->my_castled[index] == Castle_Castled)
            castled_state = "";
        else if (this->my_castled[index] == Castle_None)
            castled_state.append(1, convert('K')), castled_state.append(1, convert('Q'));
        else if (this->my_castled[index] == Castle_Kingside)
            castled_state += "Q";
        else if (this->my_castled[index] == Castle_Queenside)
            castled_state += "K";
        else
            castled_state += "";

        return castled_state;
    }

    [[nodiscard]] auto Board::to_fen_string (Color turn) const -> string
    {
        string output;

        for (int8_t row = 0; row < Num_Rows; row++)
        {
            string row_string;
            int none_count = 0;

            for (int8_t col = 0; col < Num_Columns; col++)
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

        if (this->my_en_passant_target[Color_Index_White] != No_En_Passant_Coord)
            output += " " + wisdom::to_string (this->my_en_passant_target[Color_Index_White]) + " ";
        else if (this->my_en_passant_target[Color_Index_Black] != No_En_Passant_Coord)
            output += " " + wisdom::to_string (this->my_en_passant_target[Color_Index_Black]) + " ";
        else
            output += " - ";

        output += std::to_string (this->my_half_move_clock) + " " + std::to_string (this->my_full_move_clock);
        return output;
    }

    auto operator== (const Board &a, const Board &b) -> bool
    {
        int8_t row, col;

        FOR_EACH_ROW_AND_COL(row, col)
        {
            auto coord = make_coord (row, col);
            if (a.piece_at (coord) != b.piece_at (coord))
                return false;
        }

        for (auto color = Color_Index_White; color <= Color_Index_Black; color++)
        {
            if (a.my_en_passant_target[color] != b.my_en_passant_target[color])
                return false;

            if (a.my_castled[color] != b.my_castled[color])
                return false;
        }

        return true;
    }

    void Board::randomize_positions ()
    {
        std::random_device random_device;
        std::mt19937 rng (random_device());

        // randomize the positions:
        array<ColoredPiece, Num_Columns * Num_Rows> shuffle_pieces {};

        int8_t row, col;
        FOR_EACH_ROW_AND_COL (row, col)
        {
            shuffle_pieces[col + (row * Num_Columns)] = my_squares[row][col];
        }

        std::shuffle (&shuffle_pieces[0], &shuffle_pieces[0] + (Num_Rows * Num_Columns), rng);

        // ensure no pawns on the final rank - move same color ones,
        // promote opposite color ones.
        std::uniform_int_distribution<> no_first_row_dist { 1, 7 };
        std::uniform_int_distribution<> no_last_row_dist { 0, 6 };
        std::uniform_int_distribution<> any_row_or_col { 0, 7 };
        std::uniform_int_distribution<> no_first_or_last_dist { 1, 6 };
        std::uniform_int_distribution<> remove_chance { 0, 100 };

        auto swaps = 0;
        do
        {
            swaps = 0;
            for (auto source_col = 0; source_col < Num_Columns; source_col++)
            {
                int first_source_row = 0;
                int last_source_row = Num_Rows - 1;

                auto new_black_row = no_first_row_dist (rng);
                auto new_black_col = any_row_or_col (rng);

                swaps = swap_pawns (*this, first_source_row, source_col, Color::White,
                                    new_black_row, new_black_col, shuffle_pieces, swaps);
                auto new_white_row = no_last_row_dist (rng);
                auto new_white_col = any_row_or_col (rng);

                swaps = swap_pawns (*this, last_source_row, source_col, Color::Black, new_white_row,
                                    new_white_col, shuffle_pieces, swaps);
            }
        } while (swaps > 0);

        FOR_EACH_ROW_AND_COL (row, col)
        {
            ColoredPiece piece = shuffle_pieces[col + (row * Num_Columns)];
            if (piece_type (piece) == Piece::King)
            {
                my_king_pos[color_index (piece_color (piece))] = make_coord (row, col);
            }
//            else if (remove_chance (rng) > 50)
//            {
//                piece = Piece_And_Color_None;
//            }
            my_squares[row][col] = piece;
        }
//        std::cout << "After convert: "
//                  << "\n";
//        std::cout << to_string () << "\n";

        // update the king positions:
        // if both kings are in check, regenerate.
        auto iterations = 0;
        iterations++;
        while (is_king_threatened (*this, Color::White, my_king_pos[Color_Index_White])
               && is_king_threatened (*this, Color::Black, my_king_pos[Color_Index_Black])
               && iterations < 1000)
        {
            // swap king positions, but don't put on the last / first row to avoid regenerating
            // pawns:
            Coord source_white_king_pos = my_king_pos[Color_Index_White];
            int8_t new_row = no_first_or_last_dist (rng);
            int8_t new_col = no_first_or_last_dist (rng);
            std::swap (my_squares[Row (source_white_king_pos)][Column (source_white_king_pos)],
                       my_squares[new_row][new_col]);
            my_king_pos[Color_Index_White] = make_coord (new_row, new_col);
            iterations++;
        }

        if (iterations >= 1000)
        {
            std::cout << to_string () << "\n";
            throw Error { "Too many iterations trying to generate a random board." };
        }
        // update the board code:
        my_code = BoardCode { *this };
    }

}
