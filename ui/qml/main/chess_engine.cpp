#include <QDebug>
#include <QThread>
#include <mutex>

#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/logger.hpp"

#include "wisdom-chess/qml/chess_engine.hpp"

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

void
ChessEngine::receiveEngineMoved (
    wisdom::Move move, 
    wisdom::Color who, 
    int gameId
) {
    if (gameId == this->my_game_id)
    {
        // Do another move if the engine is hooked up to itself:
        init();
    }
}

class QmlEngineGameStatusUpdate : public GameStatusUpdate
{
private:
    nonnull_observer_ptr<ChessEngine> my_parent;

    void handleGameOver()
    {
        my_parent->my_is_game_over = true;
        emit my_parent->noMovesAvailable();
    }

public:
    explicit QmlEngineGameStatusUpdate (
        nonnull_observer_ptr<ChessEngine> parent
    ) 
        : my_parent { parent }
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
        auto game_state = my_parent->my_game->state();
        auto who = game_state->getCurrentTurn();
        my_parent->handlePotentialDrawPosition (ProposedDrawType::ThreeFoldRepetition, who);
    }

    void fiftyMovesWithoutProgressReached() override
    {
        auto game_state = my_parent->my_game->state();
        auto who = game_state->getCurrentTurn();
        my_parent->handlePotentialDrawPosition (ProposedDrawType::FiftyMovesWithoutProgress, who);
    }
};

auto 
ChessEngine::gameStatusTransition() 
    -> wisdom::GameStatus
{
    QmlEngineGameStatusUpdate status_manager { this };
    status_manager.update (my_game->state()->status());
    return my_game->state()->status();
}

void ChessEngine::findMove()
{
    auto game_state = my_game->state();
    auto output = make_shared<ChessEngineLogger>();

    if (my_is_game_over)
    {
        return;
    }

    auto player = game_state->getCurrentPlayer();
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

    auto who = game_state->getCurrentTurn();

    auto& board = game_state->getBoard();
    auto& history = game_state->getHistory();

    qDebug() << "Searching for move";
    auto optionalMove = game_state->findBestMove (output);

    // TODO: we could have timed out or the thread was interrupted, and we should distinguish
    // between these two cases. If we couldn't find any move in the time, should select a move
    // at random, and otherwise exit.
    if (optionalMove.has_value())
    {
        game_state->move (*optionalMove);
        emit engineMoved (*optionalMove, who, my_game_id);
    }
    else
    {
        emit noMovesAvailable();
    }
}

void 
ChessEngine::handlePotentialDrawPosition (
    wisdom::ProposedDrawType proposedDrawType, 
    wisdom::Color who
) {
    auto game_state = my_game->state();

    auto acceptDraw = game_state->computerWantsDraw (who);
    game_state->setProposedDrawStatus (proposedDrawType, who, acceptDraw);

    emit updateDrawStatus (proposedDrawType, who, acceptDraw);
    if (acceptDraw)
    {
        my_is_game_over = true;
        emit noMovesAvailable();
    }

    auto opponent = colorInvert (who);
    auto opponentPlayer = game_state->getPlayer (opponent);
    if (opponentPlayer == Player::ChessEngine)
    {
        auto opponentAcceptsDraw = game_state->computerWantsDraw (opponent);
        game_state->setProposedDrawStatus (proposedDrawType, opponent, opponentAcceptsDraw);
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

void
ChessEngine::receiveDrawStatus (
    wisdom::ProposedDrawType drawType, 
    wisdom::Color player, 
    bool accepted
) {
    auto game_state = my_game->state();
    game_state->setProposedDrawStatus (drawType, player, accepted);

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

void 
ChessEngine::updateConfig (
    ChessGame::Config config,
    const wisdom::MoveTimer::PeriodicFunction& notifier
) {
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
