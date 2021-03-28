#include <cassert>
#include <iostream>
#include <fstream>

#include "piece.hpp"
#include "board.hpp"
#include "move.hpp"
#include "move_tree.hpp"
#include "str.hpp"
#include "game.hpp"

namespace wisdom
{
    void Game::move (Move move)
    {
        // do the move
        do_move (board, turn, move);

        // add this move to the history
        history.add_position_and_move (board, move);

        // take our turn
        turn = color_invert (turn);
    }

    static std::string prompt (const std::string &prompt)
    {
        std::string input;
        std::cout << prompt << "? ";

        if (!std::getline (std::cin, input))
            return "";

        return chomp (input);
    }

    bool Game::save () const
    {
        std::string input = prompt ("save to what file");
        if (input.empty ())
            return false;

        history.get_move_history ().save (input);
        return true; // need to check for failure here
    }

    std::optional<Game> Game::load (Color player)
    {
        std::string input_buf;
        Move move;
        std::ifstream istream;

        std::string input_file = prompt ("load what file");
        if (input_file.empty ())
        {
            std::cout << "File is empty\n";
            return {};
        }

        istream.open (input_file, std::ios::in);

        if (istream.fail ())
        {
            std::cout << "Failed reading " << input_file << "\n";
            return {};
        }

        struct Game result { Color::White, player };

        while (std::getline (istream, input_buf))
        {
            Coord dst;
            ColoredPiece piece;

            input_buf = chomp (input_buf);

            if (input_buf == "stop")
                break;

            move = move_parse (input_buf, result.turn);

            //
            // We need to check if there's a piece at the destination, and
            // set the move as taking it. Otherwise, we'll trip over some
            // consistency checks that make sure we don't erase pieces.
            //
            dst = move_dst (move);
            piece = piece_at (result.board, dst);

            // TODO: not sure if we have to handle en-passant here.

            if (piece_type (piece) != Piece::None)
            {
                assert (piece_color (piece) != result.turn);

                // for historical reasons, we automatically convert to capture move
                // here. but should probably throw an exception instead.
                if (!is_capture_move (move))
                    move = copy_move_with_capture (move);
            }

            result.move (move);
        }

        return result;
    }

}