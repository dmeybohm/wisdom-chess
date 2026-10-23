#pragma once

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/piece.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/coord.hpp"
#include "wisdom-chess/lib/expected.hpp"
#include "wisdom-chess/engine/move_timer.hpp"
#include "wisdom-chess/engine/move_list.hpp"
#include "wisdom-chess/engine/game_status.hpp"

namespace wisdom
{
    class BoardBuilder;
    class Logger;
    class Board;
    class History;
    class TranspositionTable;

    enum class ProposedDrawType;

    enum class Player
    {
        Human,
        ChessEngine
    };

    // Who decides that the game is drawn.
    enum class DrawArbiter
    {
        // The game. It proposes a draw that can be claimed, and the search
        // goes by what the players answered.
        GameEngine,

        // The caller, which never answers a proposal and gives the search
        // its limits.
        External
    };

    using Players = array<Player, Num_Players>;
    // Type alias to avoid needing complete MoveTimer definition in header
    using PeriodicFunction = std::function<void(nonnull<MoveTimer>)>;

    class Game
    {
    public:
        // Factory functions - preferred way to create games
        [[nodiscard]] static auto
        createStandardGame()
            -> Game;

        [[nodiscard]] static auto
        createGame (const Players& players)
            -> Game;

        [[nodiscard]] static auto
        createGame (Player white_player, Player black_player)
            -> Game;

        // A FEN string that does not parse is a precondition failure. Text
        // from outside the program goes through tryCreateGameFromFen().
        [[nodiscard]] static auto createGameFromFen (const string& fen)
            -> Game;

        [[nodiscard]] static auto tryCreateGameFromFen (const string& fen)
            -> expected<Game, ParseError>;

        [[nodiscard]] static auto
        createGameFromFen (const string& fen, const Players& players)
            -> Game;

        [[nodiscard]] static auto
        createGameFromBoard (const BoardBuilder& builder)
            -> Game;

        [[nodiscard]] static auto
        createGameFromBoard (const BoardBuilder& builder, const Players& players)
            -> Game;

        Game (const Game& other);
        auto operator= (const Game& other) -> Game&;

        Game (Game&& other) noexcept;
        auto operator= (Game&& other) noexcept -> Game&;

        ~Game();

    public:

        // Searches for the best move using the caller's transposition table.
        // The table is search state, not game state: the caller owns it and
        // decides when it is cleared or reused between searches.
        [[nodiscard]] auto findBestMove (
            shared_ptr<Logger> logger,
            nonnull<TranspositionTable> transposition_table
        ) const
            -> optional<Move>;

        void move (Move move);

        [[nodiscard]] auto getCurrentTurn() const noexcept -> Color;

        void setCurrentTurn (Color new_turn);

        // The board and history themselves, for the search and the tests.
        // A frontend asks the game about them through the queries below.
        [[nodiscard]] auto
        getBoard() const& noexcept
            -> const Board&;

        [[nodiscard]] auto
        getBoard() const&&
            -> Board& = delete;

        [[nodiscard]] auto getHistory() const& noexcept -> const History&;
        [[nodiscard]] auto getHistory() const&& -> const History& = delete;

        [[nodiscard]] auto pieceAt (Coord coord) const noexcept -> optional<ColoredPiece>;

        [[nodiscard]] auto pieceAt (int row, int column) const noexcept -> optional<ColoredPiece>;

        // Whether the king of `who` is attacked.
        [[nodiscard]] auto isInCheck (Color who) const noexcept -> bool;

        [[nodiscard]] auto isLegalMove (Move move) const noexcept -> bool;

        // The legal moves for the side to move.
        [[nodiscard]] auto getLegalMoves() const noexcept -> MoveList;

        // The moves played so far, in order.
        [[nodiscard]] auto getMoveHistory() const& noexcept -> const vector<Move>&;
        [[nodiscard]] auto getMoveHistory() const&& -> const vector<Move>& = delete;

        [[nodiscard]] auto toFenString() const -> string;

        // The board drawn as text, one rank per line.
        [[nodiscard]] auto boardAsString() const -> string;

        [[nodiscard]] auto getCurrentPlayer() const noexcept -> Player;

        void setWhitePlayer (Player player) noexcept;

        void setBlackPlayer (Player player) noexcept;

        [[nodiscard]] auto getPlayer (Color color) const -> Player;

        void setPlayers (const Players& players) noexcept;

        [[nodiscard]] auto getPlayers() const noexcept -> Players;

        [[nodiscard]] auto getMaxDepth() const noexcept -> int;

        void setMaxDepth (int max_depth);

        [[nodiscard]] auto getSearchTimeout() const noexcept -> chrono::milliseconds;

        void setSearchTimeout (chrono::milliseconds timeout);

        [[nodiscard]] auto
        mapCoordinatesToMove (Coord src, Coord dst, optional<Piece> promoted) const noexcept
            -> optional<Move>;

        void setPeriodicFunction (const PeriodicFunction& periodic_function);

        [[nodiscard]] auto getStatus() const noexcept -> GameStatus;

        [[nodiscard]] auto getDrawArbiter() const noexcept -> DrawArbiter;

        // Leaves the draws to the caller, with the limits at which the
        // search is to count one.
        void setExternalDrawArbiter (DrawLimits draw_limits);

        // The limits the search applies to this game.
        [[nodiscard]] auto getDrawLimits() const noexcept -> DrawLimits;

        [[nodiscard]] auto computerWantsDraw (Color who) const -> bool;

        void setProposedDrawStatus (
            ProposedDrawType draw_type,
            Color who,
            DrawStatus draw_status
        );

        void setProposedDrawStatus (
            ProposedDrawType draw_type,
            Color who,
            bool accepted
        );

        void setProposedDrawStatus (
            ProposedDrawType draw_type,
            pair<DrawStatus, DrawStatus> draw_statuses
        );

    private:
        // Private implementation functions
        class Impl;
        explicit Game (unique_ptr<Impl> impl) noexcept;

    private:
        unique_ptr<Impl> my_pimpl;
    };
}
