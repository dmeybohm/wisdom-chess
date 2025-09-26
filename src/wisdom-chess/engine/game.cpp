#include <fstream>
#include <iostream>

#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/game_impl.hpp"
#include "wisdom-chess/engine/str.hpp"
#include "wisdom-chess/engine/output_format.hpp"
#include "wisdom-chess/engine/move_timer.hpp"
#include "wisdom-chess/engine/search.hpp"
#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"

namespace wisdom
{
    static FenOutputFormat fen_output_format;
    static WisdomGameOutputFormat wisdom_game_output_format;

    static auto
    makeOutputFormat (const string& filename)
        -> OutputFormat&
    {
        if (filename.find (".fen") != string::npos)
            return fen_output_format;
        else
            return wisdom_game_output_format;
    }

    // Game::Impl constructors
    // Main constructor that maintains all invariants
    Game::Impl::Impl (const BoardBuilder& builder, const Players& players, Color current_turn)
        : my_current_board { builder }
        , my_players { players }
    {
        my_current_board = my_current_board.withCurrentTurn (current_turn);
        my_history = History::fromInitialBoard (my_current_board);
    }

    // Delegating constructors
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

    // Private constructor for factory functions
    Game::Game (unique_ptr<Impl> impl)
        : my_pimpl { std::move (impl) }
    {
    }

    // Copy constructor
    Game::Game (const Game& other)
        : my_pimpl { make_unique<Impl> (*other.my_pimpl) }
    {
    }

    // Copy assignment
    Game& Game::operator= (const Game& other)
    {
        if (this != &other)
        {
            *my_pimpl = *other.my_pimpl;
        }
        return *this;
    }

    // Move constructor
    Game::Game (Game&& other) noexcept = default;

    // Move assignment
    Game& Game::operator= (Game&& other) noexcept
    {
        if (this != &other)
        {
            my_pimpl = std::move (other.my_pimpl);
        }
        return *this;
    }

    // Destructor
    Game::~Game() = default;

    // Factory function implementations
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
        my_pimpl->my_current_board = my_pimpl->my_current_board.withMove (getCurrentTurn(), move);
        my_pimpl->my_history.addPosition (my_pimpl->my_current_board, move);
    }

    void Game::save (const string& input) const
    {
        OutputFormat& output = makeOutputFormat (input);
        output.save (input, my_pimpl->my_current_board, my_pimpl->my_history, getCurrentTurn());
    }

    auto Game::status() const -> GameStatus
    {
        if (isCheckmated (my_pimpl->my_current_board, getCurrentTurn()))
            return GameStatus::Checkmate;

        if (isStalemated (my_pimpl->my_current_board, getCurrentTurn()))
            return GameStatus::Stalemate;

        if (my_pimpl->my_history.isThirdRepetition (my_pimpl->my_current_board))
        {
            auto third_repetition_status = my_pimpl->my_history.getThreefoldRepetitionStatus();
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

        if (my_pimpl->my_history.isFifthRepetition (getBoard()))
            return GameStatus::FivefoldRepetitionDraw;

        if (History::hasBeenFiftyMovesWithoutProgress (getBoard()))
        {
            auto fifty_moves_status = my_pimpl->my_history.getFiftyMovesWithoutProgressStatus();
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

        const auto& material = my_pimpl->my_current_board.getMaterial();
        if (material.checkmateIsPossible (my_pimpl->my_current_board) == Material::CheckmateIsPossible::No)
            return GameStatus::InsufficientMaterialDraw;

        return GameStatus::Playing;
    }

    auto Game::findBestMove (shared_ptr<Logger> logger, Color whom) const
        -> optional<Move>
    {
        if (whom == Color::None)
            whom = getCurrentTurn();

        IterativeSearch iterative_search {
            my_pimpl->my_current_board,
            my_pimpl->my_history,
            std::move (logger),
            my_pimpl->my_move_timer,
            my_pimpl->my_max_depth
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

        Game result = Game::createGame (players);

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
        return my_pimpl->my_current_board.getCurrentTurn();
    }

    void Game::setCurrentTurn (Color new_turn)
    {
        my_pimpl->my_current_board = my_pimpl->my_current_board.withCurrentTurn (new_turn);
    }

    auto Game::getBoard() const& -> const Board&
    {
        return my_pimpl->my_current_board;
    }

    auto Game::getHistory() & -> History&
    {
        return my_pimpl->my_history;
    }

    auto Game::computerWantsDraw (Color who) const -> bool
    {
        int score = evaluate (my_pimpl->my_current_board, who, 1);
        return score <= Min_Draw_Score;
    }

    static auto 
    drawDesiresToRepetitionStatus (BothPlayersDrawStatus draw_desires)
         -> DrawStatus
    {
        assert (bothPlayersReplied (draw_desires));

        DrawStatus status;
        bool white_wants_draw = draw_desires.first == DrawStatus::Accepted;
        bool black_wants_draw = draw_desires.second == DrawStatus::Accepted;

        bool accepted = white_wants_draw || black_wants_draw;
        return accepted ? DrawStatus::Accepted : DrawStatus::Declined;
    }

    void Game::Impl::updateThreefoldRepetitionDrawStatus()
    {
        auto status = drawDesiresToRepetitionStatus (my_third_repetition_draw);
        my_history.setThreefoldRepetitionStatus (status);
    }

    void Game::Impl::updateFiftyMovesWithoutProgressDrawStatus()
    {
        auto status = drawDesiresToRepetitionStatus (my_fifty_moves_without_progress_draw);
        my_history.setFiftyMovesWithoutProgressStatus (status);
    }

    void Game::setProposedDrawStatus (ProposedDrawType draw_type, Color who, DrawStatus draw_status)
    {
        switch (draw_type)
        {
            case ProposedDrawType::ThreeFoldRepetition:
                my_pimpl->my_third_repetition_draw
                    = updateDrawStatus (my_pimpl->my_third_repetition_draw, who, draw_status);
                if (bothPlayersReplied (my_pimpl->my_third_repetition_draw))
                    my_pimpl->updateThreefoldRepetitionDrawStatus();
                break;

            case ProposedDrawType::FiftyMovesWithoutProgress:
                my_pimpl->my_fifty_moves_without_progress_draw
                    = updateDrawStatus (my_pimpl->my_fifty_moves_without_progress_draw, who, draw_status);
                if (bothPlayersReplied (my_pimpl->my_fifty_moves_without_progress_draw))
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
        std::pair<DrawStatus, DrawStatus> draw_statuses
    ) {
        setProposedDrawStatus (draw_type, Color::White, draw_statuses.first);
        setProposedDrawStatus (draw_type, Color::Black, draw_statuses.second);
    }

    auto Game::getCurrentPlayer() const -> Player
    {
        return my_pimpl->my_players[colorIndex (getCurrentTurn())];
    }

    void Game::setWhitePlayer (Player player)
    {
        my_pimpl->my_players[colorIndex (Color::White)] = player;
    }

    void Game::setBlackPlayer (Player player)
    {
        my_pimpl->my_players[colorIndex (Color::Black)] = player;
    }

    auto Game::getPlayer (Color color) const -> Player
    {
        return my_pimpl->my_players[colorIndex (color)];
    }

    void Game::setPlayers (const Players& players)
    {
        my_pimpl->my_players = players;
    }

    auto Game::getPlayers() const -> Players
    {
        return my_pimpl->my_players;
    }

    auto Game::getMaxDepth() const -> int
    {
        return my_pimpl->my_max_depth;
    }

    void Game::setMaxDepth (int max_depth)
    {
        my_pimpl->my_max_depth = max_depth;
    }

    auto Game::getSearchTimeout() const -> std::chrono::seconds
    {
        return my_pimpl->my_move_timer.getSeconds();
    }

    void Game::setSearchTimeout (std::chrono::seconds seconds)
    {
        my_pimpl->my_move_timer.setSeconds (seconds);
    }

    auto Game::mapCoordinatesToMove (Coord src, Coord dst, optional<Piece> promoted) const
        -> optional<Move>
    {
        return ::wisdom::mapCoordinatesToMove (my_pimpl->my_current_board, getCurrentTurn(), src, dst, promoted);
    }

    void Game::setPeriodicFunction (const PeriodicFunction& periodic_function)
    {
        my_pimpl->my_move_timer.setPeriodicFunction (periodic_function);
    }
}
