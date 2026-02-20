#include "wisdom-chess/engine/game_status.hpp"

// Put destructor here to put vtable in this translation unit:
wisdom::GameStatusUpdate::~GameStatusUpdate() = default;

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

// Default implementations delegating to hooks:

void wisdom::GameStatusUpdate::checkmate()
{
    onGameEnded (GameStatus::Checkmate);
}

void wisdom::GameStatusUpdate::stalemate()
{
    onGameEnded (GameStatus::Stalemate);
}

void wisdom::GameStatusUpdate::insufficientMaterial()
{
    onGameEnded (GameStatus::InsufficientMaterialDraw);
}

void wisdom::GameStatusUpdate::thirdRepetitionDrawReached()
{
    onDrawProposed (ProposedDrawType::ThreeFoldRepetition);
}

void wisdom::GameStatusUpdate::thirdRepetitionDrawAccepted()
{
    onGameEnded (GameStatus::ThreefoldRepetitionAccepted);
}

void wisdom::GameStatusUpdate::fifthRepetitionDraw()
{
    onGameEnded (GameStatus::FivefoldRepetitionDraw);
}

void wisdom::GameStatusUpdate::fiftyMovesWithoutProgressReached()
{
    onDrawProposed (ProposedDrawType::FiftyMovesWithoutProgress);
}

void wisdom::GameStatusUpdate::fiftyMovesWithoutProgressAccepted()
{
    onGameEnded (GameStatus::FiftyMovesWithoutProgressAccepted);
}

void wisdom::GameStatusUpdate::seventyFiveMovesWithNoProgress()
{
    onGameEnded (GameStatus::SeventyFiveMovesWithoutProgressDraw);
}

// Hook defaults (no-op):

void wisdom::GameStatusUpdate::onGameEnded ([[maybe_unused]] GameStatus status)
{
}

void wisdom::GameStatusUpdate::onDrawProposed ([[maybe_unused]] ProposedDrawType type)
{
}
