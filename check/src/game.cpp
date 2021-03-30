#include <cassert>
#include <iostream>
#include <fstream>

#include "piece.hpp"
#include "board.hpp"
#include "move.hpp"
#include "move_tree.hpp"
#include "str.hpp"
#include "game.hpp"
#include "output_format.hpp"

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

    bool Game::save (const std::string &input) const
    {
        FenOutputFormat fen_output_format;
        WisdomGameOutputFormat wisdom_game_output_format;
        OutputFormat *output;

        if (input.find (".fen") != std::string::npos)
            output = &fen_output_format;
        else
            output = &wisdom_game_output_format;

        output->save (input, board, history, turn);

        return true; // need to check for failure here
    }

    std::optional<Game> Game::load (const std::string &filename, Color player)
    {
        std::string input_buf;
        std::ifstream istream;

        istream.open (filename, std::ios::in);

        if (istream.fail ())
        {
            std::cout << "Failed reading " << filename << "\n";
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

            Move move = move_parse (input_buf, result.turn);

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