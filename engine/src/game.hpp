#pragma once

#include "global.hpp"
#include "piece.hpp"
#include "move.hpp"
#include "move_list.hpp"
#include "board.hpp"
#include "history.hpp"
#include "move_timer.hpp"
#include "game_status.hpp"

namespace wisdom
{
    class Board;
    class History;
    class BoardBuilder;
    class Logger;

    enum class Player
    {
        Human,
        ChessEngine
    };

    using Players = array<Player, Num_Players>;

    class Game
    {
    public:
        Game();

        explicit Game (const Players& players);

        Game (Player white_player, Player black_player);

        explicit Game (Color current_turn);

        explicit Game (const BoardBuilder& builder);

        explicit Game (const BoardBuilder& builder, const Players& players);

        // All other constructors must call this one:
        explicit Game (const BoardBuilder& builder, const Players& players, Color current_turn);

        // Default copy:
        Game (const Game& other) = default;
        Game& operator= (const Game& other) = default;

        // Default move:
        Game (Game&& other) noexcept = default;
        Game& operator= (Game&& other) noexcept = default;

        static auto load (const string& filename, const Players& players) -> optional<Game>;

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

        [[nodiscard]] auto getCurrentPlayer() const -> Player
        {
            return getPlayer (my_current_board.getCurrentTurn());
        }

        void setWhitePlayer (Player player)
        {
            my_players[colorIndex (Color::White)] = player;
        }

        void setBlackPlayer (Player player)
        {
            my_players[colorIndex (Color::Black)] = player;
        }

        [[nodiscard]] auto getPlayer (Color color) const -> Player
        {
            auto index = colorIndex (color);
            return my_players[index];
        }

        void setPlayers (const Players& players)
        {
            my_players = players;
        }

        [[nodiscard]] auto getPlayers() const -> Players
        {
            return my_players;
        }

        [[nodiscard]] auto getMaxDepth() const -> int
        {
            return my_max_depth;
        }

        void setMaxDepth (int max_depth)
        {
            my_max_depth = max_depth;
        }

        [[nodiscard]] auto getSearchTimeout() const -> std::chrono::seconds
        {
            return my_move_timer.getSeconds();
        }

        void setSearchTimeout (std::chrono::seconds seconds)
        {
            my_move_timer.setSeconds (seconds);
        }

        [[nodiscard]] auto
        mapCoordinatesToMove (Coord src, Coord dst, optional<Piece> promoted) const
            -> optional<Move>
        {
            return ::wisdom::mapCoordinatesToMove (
                my_current_board,
                getCurrentTurn(),
                src,
                dst,
                promoted
            );
        }

        void setPeriodicFunction (const MoveTimer::PeriodicFunction& periodic_function)
        {
            my_move_timer.setPeriodicFunction (periodic_function);
        }

        [[nodiscard]] auto status() const -> GameStatus;

        [[nodiscard]] auto computerWantsDraw (Color who) const -> bool;

        void setProposedDrawStatus (ProposedDrawType draw_type, Color who, DrawStatus draw_status);

        void setProposedDrawStatus (ProposedDrawType draw_type, Color who, bool accepted);

        void setProposedDrawStatus (
            ProposedDrawType draw_type, 
            std::pair<DrawStatus, DrawStatus> draw_statuses
        );

    private:
        void updateThreefoldRepetitionDrawStatus();
        void updateFiftyMovesWithoutProgressDrawStatus();

    private:
        Board my_current_board {};
        History my_history;
        MoveTimer my_move_timer { Default_Max_Search_Seconds };
        int my_max_depth { Default_Max_Depth };

        Players my_players = { Player::Human, Player::ChessEngine };

        BothPlayersDrawStatus my_third_repetition_draw { 
            DrawStatus::NotReached,
            DrawStatus::NotReached 
        };
        BothPlayersDrawStatus my_fifty_moves_without_progress_draw { 
            DrawStatus::NotReached, 
            DrawStatus::NotReached 
        };
    };
}
