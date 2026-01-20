#pragma once

#include <atomic>
#include <string>

#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/ui/viewmodel/viewmodel_types.hpp"
#include "wisdom-chess/ui/viewmodel/viewmodel_settings.hpp"

namespace wisdom::ui
{
    class ViewModelStatusUpdate;

    class GameViewModelBase
    {
        friend class ViewModelStatusUpdate;

    public:
        GameViewModelBase();
        explicit GameViewModelBase (int initial_game_id);
        virtual ~GameViewModelBase() = default;

        GameViewModelBase (const GameViewModelBase&) = delete;
        auto operator= (const GameViewModelBase&) -> GameViewModelBase& = delete;

        [[nodiscard]] virtual auto
        getGame()
            -> observer_ptr<Game> = 0;

        [[nodiscard]] virtual auto
        getGame() const
            -> observer_ptr<const Game> = 0;

        [[nodiscard]] auto
        needsPawnPromotion (int srcRow, int srcCol, int dstRow, int dstCol) const
            -> bool;

        [[nodiscard]] auto
        isLegalMove (Move selectedMove) const
            -> bool;

        [[nodiscard]] auto
        inCheck() const
            -> bool;

        [[nodiscard]] auto
        moveStatus() const
            -> const std::string&;

        [[nodiscard]] auto
        gameOverStatus() const
            -> const std::string&;

        [[nodiscard]] auto
        gameId() const
            -> int;

        [[nodiscard]] auto
        thirdRepetitionDrawStatus() const
            -> DrawByRepetitionStatus;

        [[nodiscard]] auto
        fiftyMovesDrawStatus() const
            -> DrawByRepetitionStatus;

        [[nodiscard]] auto
        currentTurn() const
            -> wisdom::Color;

    protected:
        virtual void onInCheckChanged() {}
        virtual void onMoveStatusChanged() {}
        virtual void onGameOverStatusChanged() {}
        virtual void onCurrentTurnChanged() {}
        virtual void onThirdRepetitionDrawStatusChanged() {}
        virtual void onFiftyMovesDrawStatusChanged() {}

        void setInCheck (bool value);
        void setMoveStatus (std::string value);
        void setGameOverStatus (std::string value);
        void setCurrentTurn (wisdom::Color value);
        void setThirdRepetitionDrawStatus (DrawByRepetitionStatus value);
        void setFiftyMovesDrawStatus (DrawByRepetitionStatus value);

        void incrementGameId();
        void resetStateForNewGame();

        void updateDisplayedGameState();

        void setProposedDrawStatus (
            wisdom::ProposedDrawType drawType,
            DrawByRepetitionStatus status
        );

    private:
        std::atomic<int> my_game_id { 1 };
        bool my_in_check = false;
        std::string my_move_status;
        std::string my_game_over_status;
        wisdom::Color my_current_turn = wisdom::Color::White;
        DrawByRepetitionStatus my_third_repetition_draw_status = DrawByRepetitionStatus::NotReached;
        DrawByRepetitionStatus my_fifty_moves_draw_status = DrawByRepetitionStatus::NotReached;
    };
}
