#include "game.hpp"
#include "board_builder.hpp"
#include "str.hpp"
#include "output_format.hpp"

#include <fstream>
#include <iostream>

namespace wisdom
{
    static FenOutputFormat fen_output_format;
    static WisdomGameOutputFormat wisdom_game_output_format;

    static OutputFormat& make_output_format (const std::string &filename)
    {
        if (filename.find (".fen") != std::string::npos)
            return fen_output_format;
        else
            return wisdom_game_output_format;
    }

    Game::Game (Color turn_, Color computer_player) :
        player { computer_player },
        turn { turn_ }
    {}

    Game::Game (Color turn_, Color computer_player, BoardBuilder builder) :
        board { builder.build () },
        player { computer_player },
        turn { turn_ }
    {}

    void Game::move (Move move)
    {
        // do the move
        do_move (*board, turn, move);

        // add this move to the history
        history->add_position_and_move (*board, move);

        // take our turn
        turn = color_invert (turn);
    }

    void Game::save (const std::string &input) const
    {
        OutputFormat &output = make_output_format (input);
        output.save (input, *board, *history, turn);
    }

    void Game::set_analytics (std::unique_ptr<analysis::Analytics> new_analytics)
    {
        this->analytics = std::move (new_analytics);
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
            piece = result.board->piece_at (dst);

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

        std::optional<Game> optional_result = std::move (result);
        return optional_result;
    }
}