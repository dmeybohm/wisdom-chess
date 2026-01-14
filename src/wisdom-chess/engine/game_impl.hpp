#pragma once

#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/history.hpp"
#include "wisdom-chess/engine/move_timer.hpp"
#include "wisdom-chess/engine/game_status.hpp"
#include "wisdom-chess/engine/transposition_table.hpp"

namespace wisdom
{
    class Game::Impl
    {
    public:
        // Main constructor that maintains all invariants
        explicit Impl (const BoardBuilder& builder, const Players& players, Color current_turn);

        // Delegating constructors
        Impl();
        explicit Impl (const Players& players);
        explicit Impl (Player white_player, Player black_player);
        explicit Impl (Color current_turn);
        explicit Impl (const BoardBuilder& builder);
        explicit Impl (const BoardBuilder& builder, const Players& players);

        Board my_current_board {};
        History my_history;
        MoveTimer my_move_timer { Default_Max_Search_Seconds };
        int my_max_depth { Default_Max_Depth };
        mutable TranspositionTable my_transposition_table = TranspositionTable::fromMegabytes (TranspositionTable::Default_Size_In_Megabytes);

        Players my_players = { Player::Human, Player::ChessEngine };

        BothPlayersDrawStatus my_third_repetition_draw {
            DrawStatus::NotReached,
            DrawStatus::NotReached
        };
        BothPlayersDrawStatus my_fifty_moves_without_progress_draw {
            DrawStatus::NotReached,
            DrawStatus::NotReached
        };

        void updateThreefoldRepetitionDrawStatus();
        void updateFiftyMovesWithoutProgressDrawStatus();
    };
}
