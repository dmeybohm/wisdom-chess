#pragma once

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/piece.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/coord.hpp"
#include "wisdom-chess/engine/move_timer.hpp"
#include "wisdom-chess/engine/history.hpp"
#include "wisdom-chess/engine/game_status.hpp"

namespace wisdom
{
    class BoardBuilder;
    class Logger;
    class Board;

    enum class DrawStatus;
    enum class ProposedDrawType;

    enum class Player
    {
        Human,
        ChessEngine
    };

    using Players = array<Player, Num_Players>;
    // Type alias to avoid needing complete MoveTimer definition in header
    using PeriodicFunction = std::function<void(not_null<MoveTimer*>)>;

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

        [[nodiscard]] static auto createGameFromFen (const string& fen)
			-> Game;

        [[nodiscard]] static auto
        createGameFromFen (const string& fen, const Players& players)
			-> Game;

        [[nodiscard]] static auto
        createGameFromBoard (const BoardBuilder& builder)
            -> Game;

        [[nodiscard]] static auto
        createGameFromBoard (const BoardBuilder& builder, const Players& players)
			-> Game;

        [[nodiscard]] static auto
        loadGame (const string& filename, const Players& players)
            -> optional<Game>;

        // Copy constructor and assignment
        Game (const Game& other);
        Game& operator= (const Game& other);

        // Move constructor and assignment
        Game (Game&& other) noexcept;
        Game& operator= (Game&& other) noexcept;

        // Destructor
        ~Game();

    public:

        void save (const string& filename) const;

        [[nodiscard]] auto findBestMove (
            shared_ptr<Logger> logger,
            Color whom = Color::None
        ) const
            -> optional<Move>;

        void move (Move move);

        [[nodiscard]] auto getCurrentTurn() const -> Color;

        void setCurrentTurn (Color new_turn);

        [[nodiscard]] auto getBoard() const& -> const Board&;
        [[nodiscard]] auto getBoard() const&& -> Board& = delete;

        [[nodiscard]] auto getHistory() & -> History&;
        [[nodiscard]] auto getHistory() && -> History& = delete;

        [[nodiscard]] auto getCurrentPlayer() const -> Player;

        void setWhitePlayer (Player player);

        void setBlackPlayer (Player player);

        [[nodiscard]] auto getPlayer (Color color) const -> Player;

        void setPlayers (const Players& players);

        [[nodiscard]] auto getPlayers() const -> Players;

        [[nodiscard]] auto getMaxDepth() const -> int;

        void setMaxDepth (int max_depth);

        [[nodiscard]] auto getSearchTimeout() const -> std::chrono::seconds;

        void setSearchTimeout (std::chrono::seconds seconds);

        [[nodiscard]] auto
        mapCoordinatesToMove (Coord src, Coord dst, optional<Piece> promoted) const
            -> optional<Move>;

        void setPeriodicFunction (const PeriodicFunction& periodic_function);

        [[nodiscard]] auto status() const -> GameStatus;

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
            std::pair<DrawStatus, DrawStatus> draw_statuses
        );

    private:
        // Private implementation functions
        class Impl;
        explicit Game (unique_ptr<Impl> impl);
        static auto load (const string& filename, const Players& players) -> optional<Game>;

    private:
        unique_ptr<Impl> my_pimpl;
    };
}
