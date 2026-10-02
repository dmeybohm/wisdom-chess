#include <fstream>

#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/game_impl.hpp"
#include "wisdom-chess/engine/str.hpp"
#include "wisdom-chess/engine/output_format.hpp"
#include "wisdom-chess/engine/move_timer.hpp"
#include "wisdom-chess/engine/search.hpp"
#include "wisdom-chess/engine/transposition_table.hpp"
#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"

namespace wisdom
{
    namespace
    {
        FenOutputFormat fen_output_format;
        WisdomGameOutputFormat wisdom_game_output_format;

        auto
        makeOutputFormat (const string& filename)
            -> OutputFormat&
        {
            if (filename.find (".fen") != string::npos)
                return fen_output_format;
            else
                return wisdom_game_output_format;
        }
    }

    // Main constructor that maintains all invariants
    Game::Impl::Impl (const BoardBuilder& builder, const Players& players, Color current_turn)
        : current_board { builder }
        , players { players }
    {
        current_board = current_board.withCurrentTurn (current_turn);
        history = History::fromInitialBoard (current_board);
    }

    Game::Impl::Impl()
        : Impl { BoardBuilder::fromDefaultPosition(), Players { Player::Human, Player::ChessEngine }, Color::White }
    {
    }

    Game::Impl::Impl (const Players& players)
        : Impl { BoardBuilder::fromDefaultPosition(), players, Color::White }
    {
    }

    Game::Impl::Impl (Player white_player, Player black_player)
        : Impl { BoardBuilder::fromDefaultPosition(), Players { white_player, black_player }, Color::White }
    {
    }

    Game::Impl::Impl (Color current_turn)
        : Impl { BoardBuilder::fromDefaultPosition(), Players { Player::Human, Player::ChessEngine }, current_turn }
    {
    }

    Game::Impl::Impl (const BoardBuilder& builder)
        : Impl { builder, Players { Player::Human, Player::ChessEngine }, builder.getCurrentTurn() }
    {
    }

    Game::Impl::Impl (const BoardBuilder& builder, const Players& players)
        : Impl { builder, players, builder.getCurrentTurn() }
    {
    }

    Game::Game (unique_ptr<Impl> impl)
        : my_pimpl { std::move (impl) }
    {
    }

    Game::Game (const Game& other)
        : my_pimpl { make_unique<Impl> (*other.my_pimpl) }
    {
    }

    auto Game::operator= (const Game& other) -> Game&
    {
        if (this != &other)
        {
            *my_pimpl = *other.my_pimpl;
        }
        return *this;
    }

    Game::Game (Game&& other) noexcept = default;

    auto Game::operator= (Game&& other) noexcept -> Game&
    {
        if (this != &other)
        {
            my_pimpl = std::move (other.my_pimpl);
        }
        return *this;
    }

    Game::~Game() = default;

    auto Game::createStandardGame() -> Game
    {
        return Game { make_unique<Impl>() };
    }

    auto Game::createGame (const Players& players) -> Game
    {
        return Game { make_unique<Impl> (players) };
    }

    auto Game::createGame (Player white_player, Player black_player) -> Game
    {
        return Game { make_unique<Impl> (white_player, black_player) };
    }

    auto Game::createGameFromFen (const string& fen) -> Game
    {
        FenParser parser { fen };
        return parser.build();
    }

    auto Game::createGameFromFen (const string& fen, const Players& players) -> Game
    {
        FenParser parser { fen };
        auto game = parser.build();
        game.setPlayers (players);
        return game;
    }

    auto Game::createGameFromBoard (const BoardBuilder& builder) -> Game
    {
        return Game { make_unique<Impl> (builder) };
    }

    auto Game::createGameFromBoard (const BoardBuilder& builder, const Players& players) -> Game
    {
        return Game { make_unique<Impl> (builder, players) };
    }

    auto Game::loadGame (const string& filename, const Players& players) -> optional<Game>
    {
        return load (filename, players);
    }

    void Game::move (Move move)
    {
        my_pimpl->current_board = my_pimpl->current_board.withMove (getCurrentTurn(), move);
        my_pimpl->history.addPosition (my_pimpl->current_board, move);
    }

    void Game::save (const string& input) const
    {
        OutputFormat& output = makeOutputFormat (input);
        output.save (input, my_pimpl->current_board, my_pimpl->history, getCurrentTurn());
    }

    auto Game::getStatus() const -> GameStatus
    {
        if (isCheckmated (my_pimpl->current_board))
            return GameStatus::Checkmate;

        if (isStalemated (my_pimpl->current_board))
            return GameStatus::Stalemate;

        if (my_pimpl->history.isThirdRepetition (my_pimpl->current_board))
        {
            auto third_repetition_status = my_pimpl->history.getThreefoldRepetitionStatus();
            using enum DrawStatus;
            switch (third_repetition_status)
            {
                case Declined:
                    break;

                case NotReached:
                    return GameStatus::ThreefoldRepetitionReached;

                case Accepted:
                    return GameStatus::ThreefoldRepetitionAccepted;
            }
        }

        if (my_pimpl->history.isFifthRepetition (getBoard()))
            return GameStatus::FivefoldRepetitionDraw;

        if (History::hasBeenFiftyMovesWithoutProgress (getBoard()))
        {
            auto fifty_moves_status = my_pimpl->history.getFiftyMovesWithoutProgressStatus();
            using enum DrawStatus;
            switch (fifty_moves_status)
            {
                case Declined:
                    break;

                case NotReached:
                    return GameStatus::FiftyMovesWithoutProgressReached;

                case Accepted:
                    return GameStatus::FiftyMovesWithoutProgressAccepted;
            }
        }

        if (History::hasBeenSeventyFiveMovesWithoutProgress (getBoard()))
            return GameStatus::SeventyFiveMovesWithoutProgressDraw;

        const auto& material = my_pimpl->current_board.getMaterial();
        if (material.checkmateIsPossible (my_pimpl->current_board) == Material::CheckmateIsPossible::No)
            return GameStatus::InsufficientMaterialDraw;

        return GameStatus::Playing;
    }

    auto Game::findBestMove (
        shared_ptr<Logger> logger,
        nonnull<TranspositionTable> transposition_table,
        Color whom
    ) const
        -> optional<Move>
    {
        if (whom == Color::None)
            whom = getCurrentTurn();

        IterativeSearch iterative_search = IterativeSearch::create (
            my_pimpl->current_board,
            my_pimpl->history,
            std::move (logger),
            my_pimpl->move_timer,
            my_pimpl->max_depth,
            transposition_table,
            getDrawLimits()
        );
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
            return {};

        Game result = Game::createGame (players);

        while (std::getline (istream, input_buf))
        {
            input_buf = chomp (input_buf);

            if (input_buf == "stop")
                break;

            auto move = moveParseOptional (input_buf, result.getCurrentTurn());
            if (!move.has_value())
                return {};

            result.move (*move);
        }

        return result;
    }

    auto Game::getCurrentTurn() const -> Color
    {
        return my_pimpl->current_board.getCurrentTurn();
    }

    void Game::setCurrentTurn (Color new_turn)
    {
        EXPECTS( isColorValid (new_turn) );
        my_pimpl->current_board = my_pimpl->current_board.withCurrentTurn (new_turn);
        my_pimpl->history.replaceLastPosition (my_pimpl->current_board);
    }

    auto Game::getBoard() const& -> const Board&
    {
        return my_pimpl->current_board;
    }

    auto Game::getHistory() & -> History&
    {
        return my_pimpl->history;
    }

    auto Game::computerWantsDraw (Color who) const -> bool
    {
        EXPECTS( isColorValid (who) );
        int score = evaluate (my_pimpl->current_board, who, 1);
        return score <= Min_Draw_Score;
    }

    namespace
    {
        auto
        drawDesiresToRepetitionStatus (BothPlayersDrawStatus draw_desires)
             -> DrawStatus
        {
            ASSERT( bothPlayersReplied (draw_desires) );

            bool white_wants_draw = draw_desires.first == DrawStatus::Accepted;
            bool black_wants_draw = draw_desires.second == DrawStatus::Accepted;

            bool accepted = white_wants_draw || black_wants_draw;
            return accepted ? DrawStatus::Accepted : DrawStatus::Declined;
        }
    }

    void Game::Impl::updateThreefoldRepetitionDrawStatus()
    {
        auto status = drawDesiresToRepetitionStatus (third_repetition_draw);
        history.setThreefoldRepetitionStatus (status);
    }

    void Game::Impl::updateFiftyMovesWithoutProgressDrawStatus()
    {
        auto status = drawDesiresToRepetitionStatus (fifty_moves_without_progress_draw);
        history.setFiftyMovesWithoutProgressStatus (status);
    }

    void Game::setProposedDrawStatus (ProposedDrawType draw_type, Color who, DrawStatus draw_status)
    {
        EXPECTS( isColorValid (who) );
        switch (draw_type)
        {
            case ProposedDrawType::ThreeFoldRepetition:
                my_pimpl->third_repetition_draw
                    = updateDrawStatus (my_pimpl->third_repetition_draw, who, draw_status);
                if (bothPlayersReplied (my_pimpl->third_repetition_draw))
                    my_pimpl->updateThreefoldRepetitionDrawStatus();
                break;

            case ProposedDrawType::FiftyMovesWithoutProgress:
                my_pimpl->fifty_moves_without_progress_draw
                    = updateDrawStatus (my_pimpl->fifty_moves_without_progress_draw, who, draw_status);
                if (bothPlayersReplied (my_pimpl->fifty_moves_without_progress_draw))
                    my_pimpl->updateFiftyMovesWithoutProgressDrawStatus();
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
        pair<DrawStatus, DrawStatus> draw_statuses
    )
    {
        setProposedDrawStatus (draw_type, Color::White, draw_statuses.first);
        setProposedDrawStatus (draw_type, Color::Black, draw_statuses.second);
    }

    auto Game::getCurrentPlayer() const -> Player
    {
        return my_pimpl->players[colorIndex (getCurrentTurn())];
    }

    void Game::setWhitePlayer (Player player)
    {
        my_pimpl->players[colorIndex (Color::White)] = player;
    }

    void Game::setBlackPlayer (Player player)
    {
        my_pimpl->players[colorIndex (Color::Black)] = player;
    }

    auto Game::getPlayer (Color color) const -> Player
    {
        EXPECTS( isColorValid (color) );
        return my_pimpl->players[colorIndex (color)];
    }

    void Game::setPlayers (const Players& players)
    {
        my_pimpl->players = players;
    }

    auto Game::getPlayers() const -> Players
    {
        return my_pimpl->players;
    }

    auto Game::getDrawArbiter() const -> DrawArbiter
    {
        return my_pimpl->external_draw_limits.has_value()
            ? DrawArbiter::External
            : DrawArbiter::GameEngine;
    }

    void Game::setExternalDrawArbiter (DrawLimits draw_limits)
    {
        EXPECTS( draw_limits.repetitions > 0 );
        EXPECTS( draw_limits.half_moves_without_progress > 0 );
        my_pimpl->external_draw_limits = draw_limits;
    }

    auto Game::getDrawLimits() const -> DrawLimits
    {
        if (my_pimpl->external_draw_limits.has_value())
            return *my_pimpl->external_draw_limits;

        // A declined draw stands until the game is drawn without a claim.
        const auto& history = my_pimpl->history;
        bool repetition_declined = history.getThreefoldRepetitionStatus() == DrawStatus::Declined;
        bool no_progress_declined
            = history.getFiftyMovesWithoutProgressStatus() == DrawStatus::Declined;
        const auto& repetition_limits
            = repetition_declined ? Automatic_Draw_Limits : Claimable_Draw_Limits;
        const auto& no_progress_limits
            = no_progress_declined ? Automatic_Draw_Limits : Claimable_Draw_Limits;

        return {
            .repetitions = repetition_limits.repetitions,
            .half_moves_without_progress = no_progress_limits.half_moves_without_progress,
        };
    }

    auto Game::getMaxDepth() const -> int
    {
        return my_pimpl->max_depth;
    }

    void Game::setMaxDepth (int max_depth)
    {
        EXPECTS( max_depth > 0 );
        my_pimpl->max_depth = max_depth;
    }

    auto Game::getSearchTimeout() const -> chrono::milliseconds
    {
        return my_pimpl->move_timer.getTimeLimit();
    }

    void Game::setSearchTimeout (chrono::milliseconds timeout)
    {
        EXPECTS( timeout > chrono::milliseconds::zero() );
        my_pimpl->move_timer.setTimeLimit (timeout);
    }

    auto Game::mapCoordinatesToMove (Coord src, Coord dst, optional<Piece> promoted) const
        -> optional<Move>
    {
        return ::wisdom::mapCoordinatesToMove (my_pimpl->current_board, getCurrentTurn(), src, dst, promoted);
    }

    void Game::setPeriodicFunction (const PeriodicFunction& periodic_function)
    {
        my_pimpl->move_timer.setPeriodicFunction (periodic_function);
    }
}
