#include "game.hpp"
#include "board_builder.hpp"
#include "str.hpp"
#include "output_format.hpp"
#include "move_timer.hpp"
#include "search.hpp"

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

    Game::Game (Color current_turn, Color computer_player) :
            my_current_turn { current_turn },
            my_computer_player { computer_player }
    {}

    Game::Game (Color current_turn, Color computer_player, BoardBuilder builder) :
            my_board { builder.build () },
            my_current_turn { current_turn },
            my_computer_player { computer_player }
    {}

    void Game::move (Move move)
    {
        // do the move
        do_move (*my_board, my_current_turn, move);

        // add this move to the history
        my_history->add_position_and_move (*my_board, move);

        // take our my_current_turn
        my_current_turn = color_invert (my_current_turn);
    }

    void Game::save (const std::string &input) const
    {
        OutputFormat &output = make_output_format (input);
        output.save (input, *my_board, *my_history, my_current_turn);
    }

    void Game::set_analytics (std::unique_ptr<analysis::Analytics> new_analytics)
    {
        this->my_analytics = std::move (new_analytics);
    }

    std::optional<Move> Game::find_best_move (Logger &logger, Color whom) const
    {
        if (whom == Color::None)
            whom = this->my_computer_player;

        MoveTimer overdue_timer { Max_Search_Seconds };
        IterativeSearch iterative_search { *this->my_board, *this->my_history, logger,
                                           *this->my_analytics, overdue_timer, Max_Depth };
        SearchResult result = iterative_search.iteratively_deepen (whom);
        return result.move;
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

        Game result { Color::White, player };

        while (std::getline (istream, input_buf))
        {
            Coord dst;
            ColoredPiece piece;

            input_buf = chomp (input_buf);

            if (input_buf == "stop")
                break;

            Move move = move_parse (input_buf, result.my_current_turn);

            //
            // We need to check if there's a piece at the destination, and
            // set the move as taking it. Otherwise, we'll trip over some
            // consistency checks that make sure we don't erase pieces.
            //
            dst = move_dst (move);
            piece = result.my_board->piece_at (dst);

            // TODO: not sure if we have to handle en-passant here.

            if (piece_type (piece) != Piece::None)
            {
                assert (piece_color (piece) != result.my_current_turn);

                // for historical reasons, we automatically convert to capture move
                // here. but should probably throw an exception instead.
                if (!is_capture_move (move))
                    move = copy_move_with_capture (move);
            }

            result.move (move);
        }

        return result;
    }

    Color Game::get_computer_player () const
    {
        return my_computer_player;
    }

    void Game::set_computer_player (Color player)
    {
        this->my_computer_player = player;
    }

    Color Game::get_current_turn () const
    {
        return this->my_current_turn;
    }

    Board &Game::get_board () const
    {
        return *my_board;
    }

    History &Game::get_history () const
    {
        return *my_history;
    }
}