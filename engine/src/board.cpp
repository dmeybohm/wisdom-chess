#include <iostream>
#include <ostream>

#include "board.hpp"
#include "board_builder.hpp"
#include "check.hpp"

namespace wisdom
{
    // board length in characters
    static constexpr int Board_Length_In_Chars = 31;

    Board::Board()
        : Board { BoardBuilder::fromDefaultPosition() }
    {
    }

    Board::Board (const BoardBuilder& builder)
        : my_squares { builder.getSquares() }
        , my_code { BoardCode::fromBoardBuilder (builder) }
        , my_king_pos { builder.getKingPositions() }
        , my_half_move_clock { builder.getHalfMovesClock() }
        , my_full_move_clock { builder.getFullMoves() }
        , my_position { Position { *this } }
        , my_material { Material { *this } }
    {
    }

    void Board::dump() const
    {
        std::cerr << *this;
    }

    static void addDivider (string &result)
    {
        result += " ";

        for (int col = 0; col < Board_Length_In_Chars; col += 4)
        {
            for (int i = 0; i < 3; i++)
                result += '-';
            result += ' ';
        }

        result += "\n";
    }

    static void addCoords (string& result)
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

    auto Board::asString() const -> string
    {
        string result;

        char row_coord = '8';

        addDivider (result);
        for (int8_t row = 0; row < Num_Rows; row++)
        {
            for (int8_t col = 0; col < Num_Columns; col++)
            {
                ColoredPiece piece = pieceAt (row, col);

                if (!col)
                    result += "|";

                if (pieceType (piece) != Piece::None && pieceColor (piece) == Color::Black)
                {
                    result += "*";
                }
                else
                {
                    result += " ";
                }

                switch (pieceType (piece))
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

            addDivider (result);;
        }

        addCoords (result);
        return result;
    }

    [[nodiscard]] auto Board::castledString (Color color) const -> string
    {
        string castled_state;

        auto convert = [color](char ch) -> char {
            return color == Color::Black ? gsl::narrow_cast<char> (tolower(ch)) : ch;
        };

        auto castled = getCastlingEligibility (color);
        if (castled == CastlingEligible::EitherSideEligible)
            castled_state.append(1, convert('K')), castled_state.append(1, convert('Q'));
        else if (castled == CastlingEligible::KingsideIneligible)
            castled_state += "Q";
        else if (castled == CastlingEligible::QueensideIneligible)
            castled_state += "K";
        else
            castled_state += "";

        return castled_state;
    }

    [[nodiscard]] auto Board::toFenString (Color turn) const -> string
    {
        string output;

        for (int8_t row = 0; row < Num_Rows; row++)
        {
            string row_string;
            int none_count = 0;

            for (int8_t col = 0; col < Num_Columns; col++)
            {
                ColoredPiece piece = pieceAt (row, col);
                if (piece == Piece_And_Color_None)
                {
                    none_count++;
                }
                else
                {
                    if (none_count > 0)
                        row_string += std::to_string (none_count);
                    none_count = 0;
                    char ch = gsl::narrow_cast<char> (toupper (pieceChar (piece)));
                    if (pieceColor (piece) == Color::Black)
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

        string castled_white = castledString (Color::White);
        string castled_black = castledString (Color::Black);

        string both_castled = castled_white + castled_black;
        if (both_castled.length() == 0)
            both_castled = "-";

        output += both_castled;

        auto en_passant_targets = getEnPassantTargets();
        if (en_passant_targets[Color_Index_White] != No_En_Passant_Coord)
            output += " " + wisdom::asString (en_passant_targets[Color_Index_White]) + " ";
        else if (en_passant_targets[Color_Index_Black] != No_En_Passant_Coord)
            output += " " + wisdom::asString (en_passant_targets[Color_Index_Black]) + " ";
        else
            output += " - ";

        output += std::to_string (my_half_move_clock) + " " + std::to_string (my_full_move_clock);
        return output;
    }

    static void removeInvalidPawns (const Board& board, int8_t source_row, int8_t source_col,
            array<ColoredPiece, Num_Squares>& shuffle_pieces)
    {
        auto piece = shuffle_pieces[source_col + (source_row * Num_Columns)];
        if (pieceType (piece) == Piece::Pawn)
        {
            shuffle_pieces[source_col + (source_row * Num_Columns)] = Piece_And_Color_None;
        }
    }

    auto Board::withRandomPosition() const -> Board
    {
        Board result = *this;

        std::random_device random_device;
        std::mt19937 rng (random_device());

        array<ColoredPiece, Num_Columns * Num_Rows> shuffle_pieces {};

        using Distribution = std::uniform_int_distribution<>;

        // ensure no pawns on the final rank - move same color ones,
        // promote opposite color ones.
        Distribution no_first_row_dist { 1, 7 };
        Distribution no_last_row_dist { 0, 6 };
        Distribution any_row_or_col { 0, 7 };
        Distribution remove_chance { 0, 100 };

        auto iterations = 0;

        do
        {
            std::copy (std::begin (result.my_squares),
                       std::end (result.my_squares), std::begin (shuffle_pieces));
            std::shuffle (std::begin (shuffle_pieces), std::end (shuffle_pieces), rng);

            for (auto&& coord : result.allCoords())
            {
                ColoredPiece piece = shuffle_pieces[coordIndex (coord)];
                if (pieceType (piece) == Piece::King)
                    result.my_king_pos[colorIndex (pieceColor (piece))] = coord;

                result.my_squares[coordIndex (coord)] = piece;
            }

            // Remove invalid pawns.
            for (int8_t source_col = 0; source_col < Num_Columns; source_col++)
            {
                int8_t first_source_row = 0;
                auto last_source_row = gsl::narrow<int8_t> (Num_Rows - 1);

                removeInvalidPawns (result, first_source_row, source_col, result.my_squares);
                removeInvalidPawns (result, last_source_row, source_col, result.my_squares);
            }
            // if both kings are in check, regenerate.
        } while (isKingThreatened (result, Color::White, result.my_king_pos[Color_Index_White])
           && isKingThreatened (result, Color::Black, result.my_king_pos[Color_Index_Black])
                && ++iterations < 1000);

        if (iterations >= 1000)
        {
            std::cout << "Too many positions : " << asString() << "\n";
            throw Error { "Too many iterations trying to generate a random board." };
        }

        // update the board code:
        result.my_code = BoardCode::fromBoard (result);
        return result;
    }

    auto Board::findFirstCoordWithPiece (ColoredPiece piece, Coord starting_at) const
        -> optional<Coord>
    {
        auto coord_begin = std::begin (my_squares);
        auto coord_end = std::end (my_squares);

        auto finder = [piece](const ColoredPiece& p) {
            return p == piece;
        };
        auto result = std::find_if (coord_begin + coordIndex (starting_at),
                                    coord_end, finder);
        auto diff = gsl::narrow<int> (result - coord_begin);

        return (result != coord_end)
            ? std::make_optional<Coord> (Coord::fromIndex (diff))
            : nullopt;
    }

    auto operator<< (std::ostream& os, const Board& board) -> std::ostream&
    {
        os << board.asString();
        os.flush();
        return os;
    }
}
