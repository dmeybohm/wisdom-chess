#include "game.hpp"
#include "board_builder.hpp"
#include "str.hpp"
#include "output_format.hpp"
#include "move_timer.hpp"
#include "search.hpp"
#include "check.hpp"
#include "evaluate.hpp"

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

    auto Game::status() const -> GameStatus
    {
        if (is_checkmated (*my_board, get_current_turn (), *my_move_generator))
            return GameStatus::CHECKMATE;

        if (my_history->is_third_repetition (*my_board))
        {
            auto third_repetition_status = my_history->get_threefold_repetition_status ();
            switch (third_repetition_status)
            {
            case ThreeFoldRepetitionStatus::BOTH_DECLINED:
                break;

            case ThreeFoldRepetitionStatus::NOT_REACHED:
                return GameStatus::THREEFOLD_REPETITION_REACHED;

            case ThreeFoldRepetitionStatus::BLACK_DECLARED:
            case ThreeFoldRepetitionStatus::WHITE_DECLARED:
            case ThreeFoldRepetitionStatus::BOTH_DECLARED:
                return GameStatus::THREEFOLD_REPETITION_ACCEPTED;
            }
        }

        if (is_stalemated (*my_board, get_current_turn (), *my_move_generator))
            return GameStatus::STALEMATE;

        if (get_history ().is_fifth_repetition (get_board()))
            return GameStatus::FIVEFOLD_REPETITION_DRAW;

        if (History::is_fifty_move_repetition (get_board ()))
            return GameStatus::FIFTY_MOVES_WITHOUT_PROGRESS;

        return GameStatus::PLAYING;
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
        return *my_board;
    }

    auto Game::get_history () const& -> History&
    {
        return *my_history;
    }

    auto Game::get_move_generator () const& -> not_null<MoveGenerator*>
    {
        return my_move_generator.get ();
    }

    auto Game::computer_wants_draw (Color who) const -> bool
    {
        int score = evaluate (*my_board, who, 1, *my_move_generator);
        return score <= Min_Draw_Score;
    }

    void Game::set_threefold_repetition_draw_status (std::pair<bool, bool> draw_desires)
    {
        ThreeFoldRepetitionStatus status;
        bool white_wants_draw = draw_desires.first;
        bool black_wants_draw = draw_desires.second;

        if (white_wants_draw && black_wants_draw)
            status = ThreeFoldRepetitionStatus::BOTH_DECLARED;
        else if (white_wants_draw)
            status = ThreeFoldRepetitionStatus::WHITE_DECLARED;
        else if (black_wants_draw)
            status = ThreeFoldRepetitionStatus::BLACK_DECLARED;
        else
            status = ThreeFoldRepetitionStatus::BOTH_DECLINED;

        my_history->set_threefold_repetition_status (status);
    }
}
