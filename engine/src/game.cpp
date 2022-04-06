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

    static OutputFormat& make_output_format (const string& filename)
    {
        if (filename.find (".fen") != string::npos)
            return fen_output_format;
        else
            return wisdom_game_output_format;
    }

    Game::Game () :
        my_players { { Player::Human, Player::ChessEngine } },
        my_current_turn { Color::White }
    {}

    Game::Game (const Players& players) :
        my_players { players },
        my_current_turn { Color::White }
    {}

    Game::Game (Player white_player, Player black_player) :
        my_players { { white_player, black_player } },
        my_current_turn { Color::White }
    {}

    Game::Game (Color current_turn) :
        my_players { { Player::Human, Player::ChessEngine } },
        my_current_turn { current_turn }
    {}

    Game::Game (Color current_turn, const BoardBuilder& builder) :
        my_board { builder.build () },
        my_players { { Player::Human, Player::ChessEngine } },
        my_current_turn { current_turn }
    {}

    void Game::move (Move move)
    {
        // do the move
        my_board->make_move (my_current_turn, move);

        // add this move to the history
        my_history->add_position_and_move (*my_board, move);

        // take our current_turn
        my_current_turn = color_invert (my_current_turn);
    }

    void Game::save (const string& input) const
    {
        OutputFormat &output = make_output_format (input);
        output.save (input, *my_board, *my_history, my_current_turn);
    }

    void Game::set_analytics (unique_ptr<analysis::Analytics> new_analytics)
    {
        this->my_analytics = std::move (new_analytics);
    }

    auto Game::find_best_move (const Logger& logger, Color whom) const -> optional<Move>
    {
        if (whom == Color::None)
            whom = my_current_turn;

        MoveTimer overdue_timer { Max_Search_Seconds };
        overdue_timer.set_periodic_notified (my_periodic_notified);

        IterativeSearch iterative_search {
            *my_board, *my_history, logger, overdue_timer, Max_Depth, *my_analytics,
        };
        SearchResult result = iterative_search.iteratively_deepen (whom);
        return result.move;
    }

    auto Game::load (const string& filename, const Players& players) -> optional<Game>
    {
        string input_buf;
        std::ifstream istream;

        istream.open (filename, std::ios::in);

        if (istream.fail ())
        {
            std::cout << "Failed reading " << filename << "\n";
            return {};
        }

        Game result { players };

        while (std::getline (istream, input_buf))
        {
            input_buf = chomp (input_buf);

            if (input_buf == "stop")
                break;

            Move move = move_parse (input_buf, result.my_current_turn);

            //
            // We need to check if there's a piece at the destination, and
            // set the move as taking it. Otherwise, we'll trip over some
            // consistency checks that make sure we don't erase pieces.
            //
            Coord dst = move_dst (move);
            ColoredPiece piece = result.my_board->piece_at (dst);

            // TODO: not sure if we have to handle en-passant here.

            if (piece_type (piece) != Piece::None)
            {
                assert (piece_color (piece) != result.my_current_turn);

                // for historical reasons, we automatically convert to capture move
                // here. but should probably throw an exception instead.
                if (!is_normal_capture_move (move))
                    move = copy_move_with_capture (move);
            }

            result.move (move);
        }

        return result;
    }

    auto Game::get_current_turn () const -> Color
    {
        return this->my_current_turn;
    }

    void Game::set_current_turn (Color new_turn)
    {
        my_current_turn = new_turn;
    }

    auto Game::get_board () const& -> Board&
    {
        return *my_board;
    }

    auto Game::get_history () const& -> History&
    {
        return *my_history;
    }
}
