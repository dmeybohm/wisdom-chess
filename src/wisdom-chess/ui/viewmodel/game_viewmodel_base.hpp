#pragma once

#include <string>

#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/ui/viewmodel/viewmodel_types.hpp"
#include "wisdom-chess/ui/viewmodel/viewmodel_settings.hpp"

namespace wisdom::ui
{
    //
    // GameViewModelBase provides shared game state management for all UI frontends.
    //
    // Architecture:
    //   - Public non-virtual getters expose current state (inCheck, moveStatus, etc.)
    //   - Protected virtual hooks notify subclasses when state changes (on*Changed callbacks)
    //   - Protected setters update state and fire the corresponding hook
    //   - getGame() is the one required override, providing access to the Game object
    //
    // Two levels of customization:
    //   1. getGame() (required): provide access to the owned Game object
    //   2. on*Changed() callbacks (optional): react to state changes (e.g. emit Qt signals)
    //
    // updateDisplayedGameState() orchestrates the status update pipeline:
    //   resets transient state, runs GameStatusUpdate dispatch on the current game status,
    //   checks for king threats, then calls onDisplayedGameStateUpdated().
    //
    // formatBold() is a formatting hook for status message generation.
    // Default returns "<b>text</b>" (HTML). Console overrides to return plain text.
    //
    // setProposedDrawStatus() handles async draw proposals (GUI pattern where a human
    // player is asked via UI dialog). The console uses synchronous draw handling instead,
    // resolving proposals in its game loop before calling updateDisplayedGameState().
    //
    class GameViewModelBase
    {
    protected:
        GameViewModelBase() = default;

    public:
        virtual ~GameViewModelBase() = default;

        GameViewModelBase (const GameViewModelBase&) = delete;
        auto operator= (const GameViewModelBase&) -> GameViewModelBase& = delete;

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
        thirdRepetitionDrawStatus() const
            -> DrawByRepetitionStatus;

        [[nodiscard]] auto
        fiftyMovesDrawStatus() const
            -> DrawByRepetitionStatus;

        [[nodiscard]] auto
        currentTurn() const
            -> wisdom::Color;

    protected:
        [[nodiscard]] virtual auto
        getGame()
            -> observer_ptr<Game> = 0;

        [[nodiscard]] virtual auto
        getGame() const
            -> observer_ptr<const Game> = 0;

        // Formatting hook for status messages. Override to change bold formatting.
        [[nodiscard]] virtual auto
        formatBold (const std::string& text) const
            -> std::string;

        virtual void onInCheckChanged() {}
        virtual void onMoveStatusChanged() {}
        virtual void onGameOverStatusChanged() {}
        virtual void onCurrentTurnChanged() {}
        virtual void onThirdRepetitionDrawStatusChanged() {}
        virtual void onFiftyMovesDrawStatusChanged() {}
        virtual void onDisplayedGameStateUpdated() {}

        void setInCheck (bool value);
        void setMoveStatus (std::string value);
        void setGameOverStatus (std::string value);
        void setCurrentTurn (wisdom::Color value);
        void setThirdRepetitionDrawStatus (DrawByRepetitionStatus value);
        void setFiftyMovesDrawStatus (DrawByRepetitionStatus value);

        // Reset the state for a new game.
        void resetStateForNewGame();

        // Update the displayed state of the game.
        void updateDisplayedGameState();

        // Set the proposed draw status:
        void setProposedDrawStatus (
            wisdom::ProposedDrawType drawType,
            DrawByRepetitionStatus status
        );

    private:
        friend class ViewModelStatusUpdate;

        bool my_in_check = false;
        std::string my_move_status;
        std::string my_game_over_status;
        wisdom::Color my_current_turn = wisdom::Color::White;
        DrawByRepetitionStatus my_third_repetition_draw_status = DrawByRepetitionStatus::NotReached;
        DrawByRepetitionStatus my_fifty_moves_draw_status = DrawByRepetitionStatus::NotReached;
    };
}
