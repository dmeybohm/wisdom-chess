#include "game_status.hpp"

void wisdom::GameStatusManager::update_for_status (GameStatus status)
{
    switch (status)
    {
        case GameStatus::Playing:
            playing();
            break;

        case GameStatus::Checkmate:
            checkmate();
            break;

        case GameStatus::Stalemate:
            stalemate();
            break;

        case GameStatus::ThreefoldRepetitionReached:
            third_repetition_draw_reached();
            break;

        case GameStatus::ThreefoldRepetitionAccepted:
            third_repetition_draw_accepted();
            break;

        case GameStatus::FivefoldRepetitionDraw:
            fifth_repetition_draw();
            break;

        case GameStatus::FiftyMovesWithoutProgressReached:
            fifty_moves_without_progress_reached();
            break;

        case GameStatus::FiftyMovesWithoutProgressAccepted:
            fifty_moves_without_progress_accepted();
            break;

        case GameStatus::SeventyFiveMovesWithoutProgressDraw:
            seventy_five_moves_with_no_progress();
            break;

        case GameStatus::InsufficientMaterialDraw:
            insufficient_material();
            break;
    }
}
