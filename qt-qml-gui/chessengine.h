#ifndef CHESSENGINE_H
#define CHESSENGINE_H

#include <QObject>
#include <memory>

#include "chessgame.h"
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
    ChessEngine(std::unique_ptr<ChessGame> game,
                QObject *parent = nullptr);

public slots:
    // Startup the engine. If it's the engine's turn to move, make a move.
    void init();

    // Receive events about the opponent move.
    void opponentMoved(wisdom::Move move, wisdom::Color who);

    // Receive our own move:
    void receiveEngineMoved(wisdom::Move move, wisdom::Color who);

    // Receive draw proposal:
    void drawProposed();


signals:
    // The engine made a mode.
    void engineMoved(wisdom::Move move, wisdom::Color who);

    // There are no available moves.
    void noMovesAvailable();

    // Send draw response:
    void drawProposalResponse(bool response);

private:
    std::unique_ptr<ChessGame> myGame;
    void findMove();
};

#endif // CHESSENGINE_H
