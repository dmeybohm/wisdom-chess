#include <QThread>
#include <QDebug>
#include <mutex>

#include "check.hpp"
#include "chessengine.hpp"
#include "logger.hpp"

using namespace wisdom;
using std::shared_ptr;
using std::optional;
using gsl::not_null;
using wisdom::GameStatus;

ChessEngine::ChessEngine(shared_ptr<ChessGame> game, int gameId, QObject *parent)
    : QObject { parent }
    , myGame { std::move(game) }
    , myGameId { gameId }
{
}

void ChessEngine::init()
{
    findMove();
}

void ChessEngine::opponentMoved(Move move, Color who)
{
    QThread::usleep(500);
    auto game = myGame->state();
    game->move(move);
    findMove();
}

void ChessEngine::receiveEngineMoved(wisdom::Move move, wisdom::Color who,
                                     int gameId)
{
    if (gameId == this->myGameId) {
        // Do another move if the engine is hooked up to itself:
        init();
    }
}

void ChessEngine::findMove()
{
    auto gameState = myGame->state();
    Logger& output = make_standard_logger();

    if (myIsGameOver) {
        return;
    }

    auto player = gameState->get_current_player();
    if (player != Player::ChessEngine) {
        return;
    }

    auto nextStatus = gameState->status();
    auto who = gameState->get_current_turn();

    switch (nextStatus)
    {
    case GameStatus::Playing:
        break;

    case GameStatus::Checkmate:
    case GameStatus::Stalemate:
    case GameStatus::ThreefoldRepetitionAccepted:
    case GameStatus::FiftyMovesWithoutProgressAccepted:
    case GameStatus::FivefoldRepetitionDraw:
    case GameStatus::SeventyFiveMovesWithoutProgressDraw:
    case GameStatus::InsufficientMaterialDraw:
        myIsGameOver = true;
        emit noMovesAvailable();
        return;

    case GameStatus::ThreefoldRepetitionReached:
        if (computerAcceptsDraw(who)) {
            gameState->set_threefold_repetition_draw_status(
                    { true, true }
            );
            myIsGameOver = true;
            emit drawProposalResponse(true);
            emit noMovesAvailable();
            return;
        }
        gameState->set_threefold_repetition_draw_status({ false, false });
        break;

    case GameStatus::FiftyMovesWithoutProgressReached:
        if (computerAcceptsDraw(who)) {
            gameState->set_fifty_moves_without_progress_draw_status(
                    { true, true }
            );
            myIsGameOver = true;
            emit drawProposalResponse(true);
            emit noMovesAvailable();
            return;
        }
        gameState->set_fifty_moves_without_progress_draw_status({ false, false });
        emit drawProposalResponse(false);
        break;
    }

    auto& board = gameState->get_board();
    auto& history = gameState->get_history();

    qDebug() << "Searching for move";
    auto optionalMove = gameState->find_best_move(output);

    // TODO: we could have timed out or the thread was interrupted, and we should distinguish
    // between these two cases. If we couldn't find any move in the time, should select a move
    // at random, and otherwise exit.
    if (optionalMove.has_value()) {
        gameState->move(*optionalMove);
        emit engineMoved(*optionalMove, who, myGameId);
    } else {
        emit noMovesAvailable();
    }
}

void ChessEngine::drawProposed(ProposedDrawType proposalType)
{
}

auto ChessEngine::computerAcceptsDraw(Color computerColor) const -> bool
{
    auto gameState = myGame->state();
    return gameState->computer_wants_draw(computerColor);
}

auto ChessEngine::respondToDrawProposal(Color fromColor, Color toColor,
                                        ProposedDrawType proposalType)
{

    myIsGameOver = true;
    bool accepted = computerAcceptsDraw(toColor);
    setDrawStatus (proposalType, accepted);
    if (accepted) {
        myIsGameOver = true;
    }
    emit drawProposalResponse(accepted);
}

void ChessEngine::setDrawStatus (ProposedDrawType proposalType, bool accepted)
{
    auto gameState = myGame->state();
    switch (proposalType)
    {
        case ProposedDrawType::ThreeFoldRepetition:
            gameState->set_threefold_repetition_draw_status({ accepted, accepted });
            break;
        case ProposedDrawType::FiftyMovesWithoutProgress:
            gameState->set_fifty_moves_without_progress_draw_status({ accepted, accepted });
            break;
    }
}

void ChessEngine::reloadGame(shared_ptr<ChessGame> newGame, int newGameId)
{
    myGame = std::move(newGame);
    myGameId = newGameId;
    myIsGameOver = false;

    // Possibly resume searching for the next move:
    init();
}

void ChessEngine::updateConfig(ChessGame::Config config)
{
    myGame->state()->set_max_depth(config.maxDepth.internalDepth());
    myGame->state()->set_search_timeout(config.maxTime);
}
