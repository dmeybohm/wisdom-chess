#pragma once

#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/history.hpp"
#include "wisdom-chess/engine/move_timer.hpp"
#include "wisdom-chess/engine/game_status.hpp"

namespace wisdom
{
    class Game::Impl
    {
    public:
        // Main constructor that maintains all invariants
        explicit Impl (const BoardBuilder& builder, const Players& players, Color current_turn);

        Impl();
        explicit Impl (const Players& players);
        explicit Impl (Player white_player, Player black_player);
        explicit Impl (Color current_turn);
        explicit Impl (const BoardBuilder& builder);
        explicit Impl (const BoardBuilder& builder, const Players& players);

        Board current_board {};
        History history;
        MoveTimer move_timer { Default_Max_Search_Seconds };
        int max_depth { Default_Max_Depth };

        Players players = { Player::Human, Player::ChessEngine };
        DrawArbiter draw_arbiter = DrawArbiter::GameEngine;

        BothPlayersDrawStatus third_repetition_draw {
            DrawStatus::NotReached,
            DrawStatus::NotReached
        };
        BothPlayersDrawStatus fifty_moves_without_progress_draw {
            DrawStatus::NotReached,
            DrawStatus::NotReached
        };

        void updateThreefoldRepetitionDrawStatus();
        void updateFiftyMovesWithoutProgressDrawStatus();
    };
}
