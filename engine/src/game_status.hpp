#ifndef WISDOM_CHESS_GAME_STATUS_HPP
#define WISDOM_CHESS_GAME_STATUS_HPP

#include "global.hpp"

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

    class GameStatusUpdate
    {
    public:
        GameStatusUpdate() = default;
        virtual ~GameStatusUpdate() = default;

        // Run after a status update.
        void update (GameStatus status);

        //
        // Functions to call when the corresponding state is reached:
        //

        // Playing/checkmate/stalemate:
        virtual void checkmate() = 0;
        virtual void stalemate() = 0;

        // Insufficient material draw:
        virtual void insufficientMaterial() = 0;

        // Third repetition draw was reached. Ask each player about a draw.
        virtual void thirdRepetitionDrawReached() = 0;
        virtual void thirdRepetitionDrawAccepted() = 0;

        // Fifth Repetition draw was reached.
        virtual void fifthRepetitionDraw() = 0;

        // Fifty moves without progress. Ask each player about a draw.
        virtual void fiftyMovesWithoutProgressReached() = 0;
        virtual void fiftyMovesWithoutProgressAccepted() = 0;

        // Seventy-five moves without progress draw reached:
        virtual void seventyFiveMovesWithNoProgress() = 0;
    };
}

#endif // WISDOM_CHESS_GAME_STATUS_HPP
