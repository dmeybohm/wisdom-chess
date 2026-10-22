#pragma once

#include "wisdom-chess/engine/global.hpp"

namespace wisdom
{
    // A player's answer to a proposed draw.
    enum class DrawStatus
    {
        NotReached = 0,
        Accepted,
        Declined
    };

    // The occurrences of a position, and the halfmoves without progress,
    // at which a position counts as a draw.
    struct DrawLimits
    {
        int repetitions;
        int half_moves_without_progress;

        [[nodiscard]] friend auto
        operator== (const DrawLimits&, const DrawLimits&)
            -> bool = default;
    };

    // The limits at which a player may claim a draw.
    inline constexpr DrawLimits Claimable_Draw_Limits {
        .repetitions = 3,
        .half_moves_without_progress = 100,
    };

    // The limits at which a game is drawn without a claim.
    inline constexpr DrawLimits Automatic_Draw_Limits {
        .repetitions = 5,
        .half_moves_without_progress = 150,
    };

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

        GameStatusUpdate (const GameStatusUpdate&) = delete;
        auto operator= (const GameStatusUpdate&) -> GameStatusUpdate& = delete;

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
