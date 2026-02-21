#include "wisdom-chess/engine/game_status.hpp"

// Put destructor here to put vtable in this translation unit:
wisdom::GameStatusUpdate::~GameStatusUpdate() = default;

void wisdom::GameStatusUpdate::update (GameStatus status)
{
    using enum GameStatus;
    switch (status)
    {
        case Playing:
            break;

        case Checkmate:
            checkmate();
            break;

        case Stalemate:
            stalemate();
            break;

        case ThreefoldRepetitionReached:
            thirdRepetitionDrawReached();
            break;

        case ThreefoldRepetitionAccepted:
            thirdRepetitionDrawAccepted();
            break;

        case FivefoldRepetitionDraw:
            fifthRepetitionDraw();
            break;

        case FiftyMovesWithoutProgressReached:
            fiftyMovesWithoutProgressReached();
            break;

        case FiftyMovesWithoutProgressAccepted:
            fiftyMovesWithoutProgressAccepted();
            break;

        case SeventyFiveMovesWithoutProgressDraw:
            seventyFiveMovesWithNoProgress();
            break;

        case InsufficientMaterialDraw:
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
