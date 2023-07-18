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

    static OutputFormat& makeOutputFormat (const string& filename)
    {
        if (filename.find (".fen") != string::npos)
            return fen_output_format;
        else
            return wisdom_game_output_format;
    }

    Game::Game ()
        : Game { BoardBuilder::from_default_position(),  { { Player::Human, Player::ChessEngine } } }
    {
    }

    Game::Game (const Players& players)
        : Game { BoardBuilder::from_default_position(), { { players[0], players[1] } } }
    {}

    Game::Game (Player white_player, Player black_player)
        : Game { BoardBuilder::from_default_position(), { { white_player, black_player } } }
    {
    }

    Game::Game (Color current_turn)
        : Game { BoardBuilder::from_default_position(), { Player::Human, Player::ChessEngine } }
    {
        setCurrentTurn (current_turn);
    }

    Game::Game (const BoardBuilder& builder)
        : Game (builder, { Player::Human, Player::ChessEngine })
    {
    }

    // All other constructors must call this one:
    Game::Game (const BoardBuilder& builder, const Players& players)
        : my_current_board { builder }
        , my_players { players }
    {
        add_current_board_to_history();
    }

    void Game::move (Move move)
    {
        my_current_board = my_current_board.with_move (getCurrentTurn (), move);
        add_current_board_to_history();
    }

    void Game::add_current_board_to_history ()
    {
        my_previous_boards.push_back (make_unique<Board> (my_current_board));
        my_history->add_position (my_previous_boards.back().get());
    }

    void Game::save (const string& input) const
    {
        OutputFormat& output = makeOutputFormat (input);
        output.save (input, my_current_board, *my_history, getCurrentTurn ());
    }

    auto Game::status () const -> GameStatus
    {
        if (is_checkmated (my_current_board, getCurrentTurn (), *my_move_generator))
            return GameStatus::Checkmate;

        if (is_stalemated (my_current_board, getCurrentTurn (), *my_move_generator))
            return GameStatus::Stalemate;

        if (my_history->is_third_repetition (my_current_board))
        {
            auto third_repetition_status = my_history->get_threefold_repetition_status ();
            switch (third_repetition_status)
            {
            case DrawStatus::Declined:
                break;

            case DrawStatus::NotReached:
                return GameStatus::ThreefoldRepetitionReached;

            case DrawStatus::Accepted:
                return GameStatus::ThreefoldRepetitionAccepted;
            }
        }

        if (my_history->is_fifth_repetition (getBoard ()))
            return GameStatus::FivefoldRepetitionDraw;

        if (History::has_been_fifty_moves_without_progress (getBoard ()))
        {
            auto fifty_moves_status = my_history->get_fifty_moves_without_progress_status ();
            switch (fifty_moves_status)
            {
                case DrawStatus::Declined:
                    break;

                case DrawStatus::NotReached:
                    return GameStatus::FiftyMovesWithoutProgressReached;

                case DrawStatus::Accepted:
                    return GameStatus::FiftyMovesWithoutProgressAccepted;
            }
        }

        if (History::has_been_seventy_five_moves_without_progress (getBoard ()))
            return GameStatus::SeventyFiveMovesWithoutProgressDraw;

        const auto& material = my_current_board.get_material ();
        if (material.checkmate_is_possible (my_current_board) == Material::CheckmateIsPossible::No)
            return GameStatus::InsufficientMaterialDraw;

        return GameStatus::Playing;
    }

    auto Game::findBestMove (const Logger& logger, Color whom) const
        -> optional<Move>
    {
        if (whom == Color::None)
            whom = getCurrentTurn ();

        MoveTimer overdue_timer { my_search_timeout };
        if (my_periodic_function.has_value ())
            overdue_timer.set_periodic_function (*my_periodic_function);

        IterativeSearch iterative_search {
            my_current_board, *my_history, logger, overdue_timer,
            my_max_depth
        };
        SearchResult result = iterative_search.iteratively_deepen (whom);

        // If user cancelled the search, discard the results.
        if (iterative_search.is_cancelled ())
            return {};

        return result.move;
    }

    auto Game::load (const string& filename, const Players& players)
        -> optional<Game>
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

            Move move = move_parse (input_buf, result.getCurrentTurn ());
            result.move (move);
        }

        return result;
    }

    auto Game::getCurrentTurn () const -> Color
    {
        return my_current_board.get_current_turn ();
    }

    void Game::setCurrentTurn (Color new_turn)
    {
        my_current_board.set_current_turn (new_turn);
    }

    auto Game::getBoard () const& -> const Board&
    {
        return my_current_board;
    }

    auto Game::getHistory () const& -> History&
    {
        return *my_history;
    }

    auto Game::getMoveGenerator () const& -> not_null<MoveGenerator*>
    {
        return my_move_generator.get ();
    }

    auto Game::computerWantsDraw (Color who) const -> bool
    {
        int score = evaluate (my_current_board, who, 1, *my_move_generator);
        return score <= Min_Draw_Score;
    }

    static auto drawDesiresToRepetitionStatus (BothPlayersDrawStatus draw_desires)
         -> DrawStatus
    {
        assert (both_players_replied (draw_desires));

        DrawStatus status;
        bool white_wants_draw = draw_desires.first == DrawStatus::Accepted;
        bool black_wants_draw = draw_desires.second == DrawStatus::Accepted;

        bool accepted = white_wants_draw || black_wants_draw;
        return accepted ? DrawStatus::Accepted : DrawStatus::Declined;
    }

    void Game::update_threefold_repetition_draw_status ()
    {
        auto status = drawDesiresToRepetitionStatus (my_third_repetition_draw);
        my_history->set_threefold_repetition_status (status);
    }

    void Game::update_fifty_moves_without_progress_draw_status ()
    {
        auto status = drawDesiresToRepetitionStatus (my_fifty_moves_without_progress_draw);
        my_history->set_fifty_moves_without_progress_status (status);
    }

    void Game::setProposedDrawStatus (ProposedDrawType draw_type, Color who,
                                         DrawStatus draw_status)
    {
        switch (draw_type)
        {
            case ProposedDrawType::ThreeFoldRepetition:
                my_third_repetition_draw = update_draw_status (
                        my_third_repetition_draw, who, draw_status
                );
                if (both_players_replied (my_third_repetition_draw))
                    update_threefold_repetition_draw_status ();
                break;

            case ProposedDrawType::FiftyMovesWithoutProgress:
                my_fifty_moves_without_progress_draw = update_draw_status (
                        my_fifty_moves_without_progress_draw, who, draw_status
                );
                if (both_players_replied (my_fifty_moves_without_progress_draw))
                    update_fifty_moves_without_progress_draw_status ();
                break;
        }
    }

    void Game::setProposedDrawStatus (ProposedDrawType draw_type, Color who, bool accepted)
    {
        setProposedDrawStatus (draw_type, who,
                               accepted ? DrawStatus::Accepted : DrawStatus::Declined);
    }

    void Game::setProposedDrawStatus (ProposedDrawType draw_type,
                                         std::pair<DrawStatus, DrawStatus> draw_statuses)
    {
        setProposedDrawStatus (draw_type, Color::White, draw_statuses.first);
        setProposedDrawStatus (draw_type, Color::Black, draw_statuses.second);
    }

}
