#pragma once

#include "wisdom-chess/engine/global.hpp"

namespace wisdom
{
    enum class GameStatus
    {
        Playing,
        Checkmate,
        Stalemate,
        ThreefoldRepetitionReached,
        ThreefoldRepetitionAccepted,
        FivefoldRepetitionDraw,
        FiftyMovesWithoutProgressReached,
        FiftyMovesWithoutProgressAccepted,
        SeventyFiveMovesWithoutProgressDraw,
        InsufficientMaterialDraw,
    };

    enum class ProposedDrawType
    {
        ThreeFoldRepetition,
        FiftyMovesWithoutProgress,
    };

    class Game;

    //
    // GameStatusUpdate provides a two-level dispatch for handling game status changes.
    //
    // Level 1 (fine-grained): Override individual virtual methods (checkmate(), stalemate(), etc.)
    //   for precise control over each status transition.
    //
    // Level 2 (simplified): Override the two hook methods:
    //   - onGameEnded(GameStatus) — called for all 7 game-ending statuses:
    //       Checkmate, Stalemate, InsufficientMaterialDraw, ThreefoldRepetitionAccepted,
    //       FivefoldRepetitionDraw, FiftyMovesWithoutProgressAccepted,
    //       SeventyFiveMovesWithoutProgressDraw
    //   - onDrawProposed(ProposedDrawType) — called for 2 draw-proposal statuses:
    //       ThreefoldRepetitionReached, FiftyMovesWithoutProgressReached
    //
    // The dispatch flow is: update() -> individual virtual method -> hook.
    // Override at whichever level suits your needs. The individual methods have
    // default implementations that delegate to the hooks, so you only need to
    // override at one level.
    //
    class GameStatusUpdate
    {
    public:
        GameStatusUpdate() = default;

        // Put virtual destructor in the .cpp file to put vtable there:
        virtual ~GameStatusUpdate();

        // Run after a status update.
        void update (GameStatus status);

        //
        // Level 1: Fine-grained virtual methods for each status.
        // Default implementations delegate to onGameEnded() or onDrawProposed().
        //

        virtual void checkmate();
        virtual void stalemate();
        virtual void insufficientMaterial();
        virtual void thirdRepetitionDrawReached();
        virtual void thirdRepetitionDrawAccepted();
        virtual void fifthRepetitionDraw();
        virtual void fiftyMovesWithoutProgressReached();
        virtual void fiftyMovesWithoutProgressAccepted();
        virtual void seventyFiveMovesWithNoProgress();

    protected:
        //
        // Level 2: Simplified hooks. Override these for broad handling.
        //
        virtual void onGameEnded (GameStatus status);
        virtual void onDrawProposed (ProposedDrawType type);
    };
}
