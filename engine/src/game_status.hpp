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

    class GameStatusManager
    {

    public:
        GameStatusManager()  = default;
        virtual ~GameStatusManager () = default;

        // Run after a status update.
        void update_for_status (GameStatus status);

        // Called before/after the status updates:
        virtual void before_status_update (GameStatus status) = 0;
        virtual void after_status_update() = 0;

        //
        // Functions to call when the corresponding state is reached:
        //

        // Playing/checkmate/stalemate:
        virtual void playing() = 0;
        virtual void checkmate() = 0;
        virtual void stalemate() = 0;

        // Insufficient material draw:
        virtual void insufficient_material() = 0;

        // Third repetition draw was reached. Ask each player about a draw.
        virtual void third_repetition_draw_reached() = 0;
        virtual void third_repetition_draw_accepted() = 0;

        // Fifth Repetition draw was reached.
        virtual void fifth_repetition_draw() = 0;

        // Fifty moves without progress. Ask each player about a draw.
        virtual void fifty_moves_without_progress_reached() = 0;
        virtual void fifty_moves_without_progress_accepted() = 0;

        // Seventy-five moves without progress draw reached:
        virtual void seventy_five_moves_with_no_progress() = 0;
    };
}

#endif // WISDOM_CHESS_GAME_STATUS_HPP
