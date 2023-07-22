#include "game_status.hpp"

void wisdom::GameStatusUpdate::update (GameStatus status)
{
    switch (status)
    {
        case GameStatus::Playing:
            break;

        case GameStatus::Checkmate:
            checkmate();
            break;

        case GameStatus::Stalemate:
            stalemate();
            break;

        case GameStatus::ThreefoldRepetitionReached:
            thirdRepetitionDrawReached();
            break;

        case GameStatus::ThreefoldRepetitionAccepted:
            thirdRepetitionDrawAccepted();
            break;

        case GameStatus::FivefoldRepetitionDraw:
            fifthRepetitionDraw();
            break;

        case GameStatus::FiftyMovesWithoutProgressReached:
            fiftyMovesWithoutProgressReached();
            break;

        case GameStatus::FiftyMovesWithoutProgressAccepted:
            fiftyMovesWithoutProgressAccepted();
            break;

        case GameStatus::SeventyFiveMovesWithoutProgressDraw:
            seventyFiveMovesWithNoProgress();
            break;

        case GameStatus::InsufficientMaterialDraw:
            insufficientMaterial();
            break;
    }
}
