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
        my_players { { Player::Human, Player::ChessEngine } }
    {}

    Game::Game (const Players& players) :
        my_players { players }
    {}

    Game::Game (Player white_player, Player black_player) :
        my_players { { white_player, black_player } }
    {}

    Game::Game (Color current_turn) :
        my_players { { Player::Human, Player::ChessEngine } }
    {
        set_current_turn (current_turn);
    }

    Game::Game (const BoardBuilder& builder) :
        my_board { builder.build () },
        my_players { { Player::Human, Player::ChessEngine } }
    {}

    void Game::move (Move move)
    {
        // do the move
        auto undo_state = my_board->make_move (get_current_turn (), move);

        // add this move to the history
        my_history->add_position_and_move (*my_board, move, undo_state);
    }

    void Game::save (const string& input) const
    {
        OutputFormat &output = make_output_format (input);
        output.save (input, *my_board, *my_history, get_current_turn ());
    }

    void Game::set_analytics (unique_ptr<analysis::Analytics> new_analytics)
    {
        this->my_analytics = std::move (new_analytics);
    }

    auto Game::find_best_move (const Logger& logger, Color whom) const -> optional<Move>
    {
        if (whom == Color::None)
            whom = get_current_turn ();

        MoveTimer overdue_timer { my_search_timeout };
        if (my_periodic_function.has_value ())
            overdue_timer.set_periodic_function (*my_periodic_function);

        IterativeSearch iterative_search {
            *my_board, *my_history, logger, overdue_timer,
            my_max_depth, *my_analytics,
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

            Move move = move_parse (input_buf, result.get_current_turn ());

            Coord dst = move_dst (move);
            ColoredPiece piece = result.my_board->piece_at (dst);

            result.move (move);
        }

        return result;
    }

    auto Game::get_current_turn () const -> Color
    {
        return my_board->get_current_turn ();
    }

    void Game::set_current_turn (Color new_turn)
    {
        my_board->set_current_turn (new_turn);
    }

    auto Game::get_board () const& -> Board&
    {
        // todo - I think this may be copying
        return *my_board;
    }

    auto Game::get_history () const& -> History&
    {
        // todo - I think this may be copying
        return *my_history;
    }

    auto Game::get_move_generator () const& -> gsl::not_null<MoveGenerator*>
    {
        return my_move_generator.get ();
    }
}
