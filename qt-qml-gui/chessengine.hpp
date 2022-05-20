#ifndef CHESSENGINE_H
#define CHESSENGINE_H

#include <QObject>
#include <functional>
#include <memory>

#include "chessgame.hpp"
#include "game.hpp"
#include "move.hpp"
#include "move_timer.hpp"

class ChessEngineNotifier;

//
// Represents the computer player.
//
class ChessEngine : public QObject
{
    Q_OBJECT

public:
    ChessEngine(std::shared_ptr<ChessGame> game,
                int gameId,
                QObject *parent = nullptr);

public slots:
    // Startup the engine. If it's the engine's turn to move, make a move.
    void init();

    // Receive events about the opponent move.
    void opponentMoved(wisdom::Move move, wisdom::Color who);

    // Receive our own move:
    void receiveEngineMoved(wisdom::Move move, wisdom::Color who,
                            int gameId);

    // Receive draw proposal:
    void drawProposed();

    // Update the whole chess game state. The ownership of the game is taken.
    void reloadGame(std::shared_ptr<ChessGame> newGame, int newGameId);

signals:
    // The engine made a move.
    void engineMoved(wisdom::Move move, wisdom::Color who, int gameId);

    // There are no available moves.
    void noMovesAvailable();

    // Send draw response:
    void drawProposalResponse(bool response);

private:
    std::shared_ptr<ChessGame> myGame;
    int myGameId;

    void findMove();
};

#endif // CHESSENGINE_H
