#include <QDebug>
#include <QThread>
#include <mutex>

#include "check.hpp"
#include "chess_engine.hpp"
#include "logger.hpp"

using namespace wisdom;
using gsl::not_null;
using std::optional;
using std::shared_ptr;
using wisdom::GameStatus;
using wisdom::ProposedDrawType;

void ChessEngine::ChessEngineLogger::info (const std::string& line) const
{
    qDebug() << line.c_str();
}

ChessEngine::ChessEngine (shared_ptr<ChessGame> game, int gameId, QObject* parent) :
        QObject { parent }, my_game { std::move (game) }, my_game_id { gameId }
{
}

void ChessEngine::init()
{
    findMove();
}

void ChessEngine::opponentMoved (Move move, Color who)
{
    auto game = my_game->state();
    game->move (move);
    findMove();
}

void ChessEngine::receiveEngineMoved (wisdom::Move move, wisdom::Color who, int gameId)
{
    if (gameId == this->my_game_id)
    {
        // Do another move if the engine is hooked up to itself:
        init();
    }
}

class QmlEngineGameStatusUpdate : public GameStatusUpdate
{
private:
    observer_ptr<ChessEngine> myParent;

    void handleGameOver()
    {
        myParent->my_is_game_over = true;
        emit myParent->noMovesAvailable();
    }

public:
    explicit QmlEngineGameStatusUpdate (observer_ptr<ChessEngine> parent) : myParent { parent }
    {
    }

    void checkmate() override
    {
        handleGameOver();
    }

    void stalemate() override
    {
        handleGameOver();
    }

    void insufficientMaterial() override
    {
        handleGameOver();
    }

    void thirdRepetitionDrawAccepted() override
    {
        handleGameOver();
    }

    void fifthRepetitionDraw() override
    {
        handleGameOver();
    }

    void fiftyMovesWithoutProgressAccepted() override
    {
        handleGameOver();
    }

    void seventyFiveMovesWithNoProgress() override
    {
        handleGameOver();
    }

    void thirdRepetitionDrawReached() override
    {
        auto gameState = myParent->my_game->state();
        auto who = gameState->getCurrentTurn();
        myParent->handlePotentialDrawPosition (ProposedDrawType::ThreeFoldRepetition, who);
    }

    void fiftyMovesWithoutProgressReached() override
    {
        auto gameState = myParent->my_game->state();
        auto who = gameState->getCurrentTurn();
        myParent->handlePotentialDrawPosition (ProposedDrawType::FiftyMovesWithoutProgress, who);
    }
};

auto ChessEngine::gameStatusTransition() -> wisdom::GameStatus
{
    QmlEngineGameStatusUpdate status_manager { this };
    status_manager.update (my_game->state()->status());
    return my_game->state()->status();
}

void ChessEngine::findMove()
{
    auto gameState = my_game->state();
    auto output = ChessEngineLogger {};

    if (my_is_game_over)
    {
        return;
    }

    auto player = gameState->getCurrentPlayer();
    if (player != Player::ChessEngine)
    {
        return;
    }

    auto nextStatus = gameStatusTransition();
    if (nextStatus != GameStatus::Playing)
    {
        // The game is now over - or we're waiting for a response on a draw proposal.
        return;
    }

    // Wait for animation to finish
    QThread::usleep (200000); // 200 ms

    auto who = gameState->getCurrentTurn();

    auto& board = gameState->getBoard();
    auto& history = gameState->getHistory();

    qDebug() << "Searching for move";
    auto optionalMove = gameState->findBestMove (output);

    // TODO: we could have timed out or the thread was interrupted, and we should distinguish
    // between these two cases. If we couldn't find any move in the time, should select a move
    // at random, and otherwise exit.
    if (optionalMove.has_value())
    {
        gameState->move (*optionalMove);
        emit engineMoved (*optionalMove, who, my_game_id);
    }
    else
    {
        emit noMovesAvailable();
    }
}

void ChessEngine::handlePotentialDrawPosition (wisdom::ProposedDrawType proposedDrawType,
                                               wisdom::Color who)
{
    auto gameState = my_game->state();

    auto acceptDraw = gameState->computerWantsDraw (who);
    gameState->setProposedDrawStatus (proposedDrawType, who, acceptDraw);

    emit updateDrawStatus (proposedDrawType, who, acceptDraw);
    if (acceptDraw)
    {
        my_is_game_over = true;
        emit noMovesAvailable();
    }

    auto opponent = colorInvert (who);
    auto opponentPlayer = gameState->getPlayer (opponent);
    if (opponentPlayer == Player::ChessEngine)
    {
        auto opponentAcceptsDraw = gameState->computerWantsDraw (opponent);
        gameState->setProposedDrawStatus (proposedDrawType, opponent, opponentAcceptsDraw);
        emit updateDrawStatus (proposedDrawType, opponent, opponentAcceptsDraw);
        if (opponentAcceptsDraw)
        {
            my_is_game_over = true;
            emit noMovesAvailable();
        }
        else
        {
            // if the computer is playing itself, resume searching:
            if (gameStatusTransition() == GameStatus::Playing)
            {
                findMove();
            }
        }
    }
}

void ChessEngine::receiveDrawStatus (wisdom::ProposedDrawType drawType, wisdom::Color player,
                                     bool accepted)
{
    auto gameState = my_game->state();
    gameState->setProposedDrawStatus (drawType, player, accepted);

    auto nextStatus = gameStatusTransition();
    if (nextStatus == GameStatus::Playing)
    {
        findMove(); // resume playing.
    }
}

void ChessEngine::reloadGame (shared_ptr<ChessGame> newGame, int newGameId)
{
    my_game = std::move (newGame);
    my_game_id = newGameId;
    my_is_game_over = false;

    // Possibly resume searching for the next move:
    init();
}

void ChessEngine::updateConfig (ChessGame::Config config,
                                const wisdom::MoveTimer::PeriodicFunction& notifier)
{
    my_game->setConfig (config);

    // Update the notifier:
    my_game->setPeriodicFunction (notifier);

    // Possibly resume searching for the next move:
    init();
}

void ChessEngine::quit()
{
    QThread::currentThread()->quit();
}
