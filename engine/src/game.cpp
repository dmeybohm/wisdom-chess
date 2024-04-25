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

    Game::Game()
        : Game { BoardBuilder::fromDefaultPosition(), { { Player::Human, Player::ChessEngine } } }
    {
    }

    Game::Game (const Players& players)
        : Game { BoardBuilder::fromDefaultPosition(), { { players[0], players[1] } } }
    {}

    Game::Game (Player white_player, Player black_player)
        : Game { BoardBuilder::fromDefaultPosition(), { { white_player, black_player } } }
    {
    }

    Game::Game (Color current_turn)
        : Game {
                BoardBuilder::fromDefaultPosition(),
                { Player::Human, Player::ChessEngine },
                current_turn
        }
    {
    }

    Game::Game (const BoardBuilder& builder)
        : Game (builder, { Player::Human, Player::ChessEngine })
    {
    }

    Game::Game (const BoardBuilder& builder, const Players& players)
        : Game { builder, players, builder.getCurrentTurn() }
    {
    }

    // All other constructors must call this one:
    Game::Game (const BoardBuilder& builder, const Players& players, Color current_turn)
        : my_current_board { builder }
        , my_players { players }
    {
        setCurrentTurn (current_turn);
        my_history = History::fromInitialBoard (my_current_board);
    }

    void Game::move (Move move)
    {
        my_current_board = my_current_board.withMove (getCurrentTurn(), move);
        my_history.addPosition (my_current_board, move);
    }

    void Game::save (const string& input) const
    {
        OutputFormat& output = makeOutputFormat (input);
        output.save (input, my_current_board, my_history, getCurrentTurn());
    }

    auto Game::status() const -> GameStatus
    {
        if (isCheckmated (my_current_board, getCurrentTurn(), my_move_generator))
            return GameStatus::Checkmate;

        if (isStalemated (my_current_board, getCurrentTurn(), my_move_generator))
            return GameStatus::Stalemate;

        if (my_history.isThirdRepetition (my_current_board))
        {
            auto third_repetition_status = my_history.getThreefoldRepetitionStatus();
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

        if (my_history.isFifthRepetition (getBoard()))
            return GameStatus::FivefoldRepetitionDraw;

        if (History::hasBeenFiftyMovesWithoutProgress (getBoard()))
        {
            auto fifty_moves_status = my_history.getFiftyMovesWithoutProgressStatus();
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

        if (History::hasBeenSeventyFiveMovesWithoutProgress (getBoard()))
            return GameStatus::SeventyFiveMovesWithoutProgressDraw;

        const auto& material = my_current_board.getMaterial();
        if (material.checkmateIsPossible (my_current_board) == Material::CheckmateIsPossible::No)
            return GameStatus::InsufficientMaterialDraw;

        return GameStatus::Playing;
    }

    auto Game::findBestMove (const Logger& logger, Color whom) const
        -> optional<Move>
    {
        if (whom == Color::None)
            whom = getCurrentTurn();

        MoveTimer overdue_timer { my_search_timeout };
        if (my_periodic_function.has_value())
            overdue_timer.setPeriodicFunction (*my_periodic_function);

        IterativeSearch iterative_search {
            my_current_board, my_history, logger, overdue_timer,
            my_max_depth
        };
        SearchResult result = iterative_search.iterativelyDeepen (whom);

        // If user cancelled the search, discard the results.
        if (iterative_search.isCancelled())
            return {};

        return result.move;
    }

    auto Game::load (const string& filename, const Players& players)
        -> optional<Game>
    {
        string input_buf;
        std::ifstream istream;

        istream.open (filename, std::ios::in);

        if (istream.fail())
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

            Move move = moveParse (input_buf, result.getCurrentTurn());
            result.move (move);
        }

        return result;
    }

    auto Game::getCurrentTurn() const -> Color
    {
        return my_current_board.getCurrentTurn();
    }

    void Game::setCurrentTurn (Color new_turn)
    {
        my_current_board = my_current_board.withCurrentTurn (new_turn);
    }

    auto Game::getBoard() const& -> const Board&
    {
        return my_current_board;
    }

    auto Game::getHistory() & -> History&
    {
        return my_history;
    }

    auto Game::getMoveGenerator() const& -> MoveGenerator&
    {
        return my_move_generator;
    }

    auto Game::computerWantsDraw (Color who) const -> bool
    {
        int score = evaluate (my_current_board, who, 1, my_move_generator);
        return score <= Min_Draw_Score;
    }

    static auto drawDesiresToRepetitionStatus (BothPlayersDrawStatus draw_desires)
         -> DrawStatus
    {
        assert (bothPlayersReplied (draw_desires));

        DrawStatus status;
        bool white_wants_draw = draw_desires.first == DrawStatus::Accepted;
        bool black_wants_draw = draw_desires.second == DrawStatus::Accepted;

        bool accepted = white_wants_draw || black_wants_draw;
        return accepted ? DrawStatus::Accepted : DrawStatus::Declined;
    }

    void Game::updateThreefoldRepetitionDrawStatus()
    {
        auto status = drawDesiresToRepetitionStatus (my_third_repetition_draw);
        my_history.setThreefoldRepetitionStatus (status);
    }

    void Game::updateFiftyMovesWithoutProgressDrawStatus()
    {
        auto status = drawDesiresToRepetitionStatus (my_fifty_moves_without_progress_draw);
        my_history.setFiftyMovesWithoutProgressStatus (status);
    }

    void Game::setProposedDrawStatus (ProposedDrawType draw_type, Color who, DrawStatus draw_status)
    {
        switch (draw_type)
        {
            case ProposedDrawType::ThreeFoldRepetition:
                my_third_repetition_draw
                    = updateDrawStatus (my_third_repetition_draw, who, draw_status);
                if (bothPlayersReplied (my_third_repetition_draw))
                    updateThreefoldRepetitionDrawStatus();
                break;

            case ProposedDrawType::FiftyMovesWithoutProgress:
                my_fifty_moves_without_progress_draw
                    = updateDrawStatus (my_fifty_moves_without_progress_draw, who, draw_status);
                if (bothPlayersReplied (my_fifty_moves_without_progress_draw))
                    updateFiftyMovesWithoutProgressDrawStatus();
                break;
        }
    }

    void Game::setProposedDrawStatus (ProposedDrawType draw_type, Color who, bool accepted)
    {
        setProposedDrawStatus (
            draw_type,
            who,
            accepted ? DrawStatus::Accepted : DrawStatus::Declined
        );
    }

    void Game::setProposedDrawStatus (
        ProposedDrawType draw_type, 
        std::pair<DrawStatus, DrawStatus> draw_statuses
    ) {
        setProposedDrawStatus (draw_type, Color::White, draw_statuses.first);
        setProposedDrawStatus (draw_type, Color::Black, draw_statuses.second);
    }
}
