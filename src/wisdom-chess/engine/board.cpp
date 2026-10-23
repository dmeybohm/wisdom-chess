#include <iostream>
#include <ostream>

#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/lib/str.hpp"
#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"

namespace wisdom
{
    // board length in characters
    static constexpr int Board_Length_In_Chars = 31;

    Board::Board()
        : Board { Default_Board_Builder }
    {
    }

    auto Board::fromFen (const string& fen) -> Board
    {
        FenParser parser { fen };
        return parser.buildBoard();
    }

    Board::Board (const BoardBuilder& builder)
        : my_squares { builder.getSquares() }
        , my_code { BoardCode::fromBoardBuilder (builder) }
        , my_half_move_clock { builder.getHalfMoveClock() }
        , my_full_move_clock { builder.getFullMoveClock() }
        , my_position { Position { *this } }
        , my_material { Material { *this } }
        , my_king_pos { builder.getKingPositions() }
    {
        for (auto coord : allCoords())
        {
            if (auto piece = pieceAt (coord))
                my_occupied[colorIndex (pieceColor (*piece))] |= CoordSet::bit (coord);
        }

        classifyEnPassantTarget();

        ENSURES( hasKingsAtKingPositions() );
        ENSURES( occupiedMatchesSquares() );
    }

    auto
    Board::hasKingsAtKingPositions() const noexcept
        -> bool
    {
        return pieceAt (getKingPosition (Color::White)) == ColoredPiece::make (Color::White, Piece::King)
            && pieceAt (getKingPosition (Color::Black)) == ColoredPiece::make (Color::Black, Piece::King);
    }

    auto
    Board::occupiedMatchesSquares() const noexcept
        -> bool
    {
        for (auto coord : allCoords())
        {
            auto piece = pieceAt (coord);
            for (auto who : { Color::White, Color::Black })
            {
                bool is_own = piece && pieceColor (*piece) == who;
                if (getPieceCoords (who).contains (coord) != is_own)
                    return false;
            }
        }

        return true;
    }

    void Board::dump() const
    {
        std::cerr << *this;
    }

    namespace
    {
        void addDivider (nonnull<string> result)
        {
            *result += " ";

            for (int col = 0; col < Board_Length_In_Chars; col += 4)
            {
                for (int i = 0; i < 3; i++)
                    *result += '-';

                *result += ' ';
            }

            *result += "\n";
        }

        void addCoords (nonnull<string> result)
        {
            *result += " ";

            char col_name = 'a';
            for (int col = 0; col < Num_Columns; col++)
            {
                *result += " ";
                *result += col_name;
                *result += "  ";
                col_name++;
            }

            *result += "\n";
        }
    }

    auto Board::asString() const -> string
    {
        string result;

        char row_coord = '8';

        addDivider (&result);
        for (int8_t row = 0; row < Num_Rows; row++)
        {
            for (int8_t col = 0; col < Num_Columns; col++)
            {
                auto piece = pieceAt (row, col);

                if (!col)
                    result += "|";

                if (piece && pieceColor (*piece) == Color::Black)
                {
                    result += "*";
                }
                else
                {
                    result += " ";
                }

                if (!piece)
                {
                    result += "  |";
                    continue;
                }

                switch (pieceType (*piece))
                {
                    case Piece::Pawn:
                        result += "p";
                        break;
                    case Piece::Knight:
                        result += "N";
                        break;
                    case Piece::Bishop:
                        result += "B";
                        break;
                    case Piece::Rook:
                        result += "R";
                        break;
                    case Piece::Queen:
                        result += "Q";
                        break;
                    case Piece::King:
                        result += "K";
                        break;
                }

                result += " |";
            }

            result += " ";
            result += row_coord;
            row_coord--;
            result += "\n";

            addDivider (&result);
        }

        addCoords (&result);
        return result;
    }

    auto Board::castledString (Color color) const -> string
    {
        string castled_state;

        auto convert = [color] (char ch) -> char
        {
            return color == Color::Black
                ? toLower (ch)
                : ch;
        };

        auto castled = getCastlingEligibility (color);
        if (castled == CastlingEligibility::Both_Sides)
        {
            castled_state += convert ('K');
            castled_state += convert ('Q');
        }
        else if (castled == CastlingRights::Kingside)
            castled_state += convert ('K');
        else if (castled == CastlingRights::Queenside)
            castled_state += convert ('Q');

        return castled_state;
    }

    [[nodiscard]] auto
    Board::toFenString (Color turn) const
        -> string
    {
        string output;

        for (int8_t row = 0; row < Num_Rows; row++)
        {
            string row_string;
            int none_count = 0;

            for (int8_t col = 0; col < Num_Columns; col++)
            {
                auto piece = pieceAt (row, col);
                if (!piece)
                {
                    none_count++;
                }
                else
                {
                    if (none_count > 0)
                        row_string += std::to_string (none_count);

                    none_count = 0;
                    char ch = toUpper (pieceToChar (*piece));
                    if (pieceColor (*piece) == Color::Black)
                        ch = toLower (ch);

                    row_string.append (1, ch);
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
        if (both_castled.empty())
            both_castled = "-";

        output += both_castled;

        auto en_passant_target = getAnyEnPassantTarget();
        if (en_passant_target != nullopt)
            output += " " + wisdom::asString (en_passant_target->coord) + " ";
        else
            output += " - ";

        output += std::to_string (my_half_move_clock) + " " + std::to_string (my_full_move_clock);
        return output;
    }

    auto
    Board::findFirstCoordWithPiece (
        Piece piece_type,
        Coord starting_at
    ) const noexcept
        -> optional<Coord>
    {
        auto coord_begin = std::begin (my_squares);
        auto coord_end = std::end (my_squares);

        auto finder = [piece_type] (const optional<ColoredPiece>& square)
        {
            return square && square->type() == piece_type;
        };
        auto result = std::find_if (
            coord_begin + starting_at.index<std::ptrdiff_t>(),
            coord_end,
            finder
        );
        auto diff = narrow<int> (result - coord_begin);

        return (result != coord_end)
            ? optional<Coord> { Coord::fromIndex (diff) }
            : nullopt;
    }

    auto operator<< (std::ostream& os, const Board& board) -> std::ostream&
    {
        os << board.asString();
        os.flush();
        return os;
    }
}
