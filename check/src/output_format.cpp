#include "board.hpp"
#include "output_format.hpp"
#include "history.hpp"

#include <iostream>
#include <fstream>
#include <cctype>

namespace wisdom
{
    void FenOutputFormat::save (const std::string &filename, const Board &board,
                                [[maybe_unused]] const History &history, Color turn)
    {
        std::string output;

        for (int8_t row = 0; row < Num_Rows; row++)
        {
            std::string row_string;
            int none_count = 0;

            for (int8_t col = 0; col < Num_Columns; col++)
            {
                ColoredPiece piece = piece_at (board, row, col);
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

        std::string castled_white = castled_string (board, Color::White);
        std::string castled_black = castled_string (board, Color::Black);

        std::string castled = castled_white + castled_black;
        if (castled.length() == 0)
            castled = "-";

        output += castled;

        if (board.en_passant_target[Color_Index_White] != No_En_Passant_Coord)
            output += " " + to_string (board.en_passant_target[Color_Index_White]) + " ";
        else if (board.en_passant_target[Color_Index_Black] != No_En_Passant_Coord)
            output += " " + to_string (board.en_passant_target[Color_Index_Black]) + " ";
        else
            output += " - ";

        output += std::to_string (board.half_move_clock) + " " + std::to_string (board.full_moves);

        std::ofstream file;
        file.open (filename);
        file << output << "\n";
        file.close ();
    }

    std::string FenOutputFormat::castled_string (const Board &board, Color color) const
    {
        ColorIndex index = color_index (color);
        std::string castled_state;

        auto convert = [color](char ch) -> char {
            return color == Color::Black ? static_cast<char> (tolower(ch)) : ch;
        };

        if (board.castled[index] == Castle_None)
            castled_state.append(1, convert('K')), castled_state.append(1, convert('Q'));
        else if (board.castled[index] == Castle_Kingside)
            castled_state += "Q";
        else if (board.castled[index] == Castle_Queenside)
            castled_state += "K";
        else
            castled_state += "";

        return castled_state;
    }

    void WisdomGameOutputFormat::save (const std::string &filename,
                                       [[maybe_unused]] const Board &board, const History &history,
                                       [[maybe_unused]] Color turn)
    {
        history.get_move_history ().save (filename);
    }
}
