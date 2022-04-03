#ifndef CHESSENGINE_H
#define CHESSENGINE_H

#include <QObject>

#include "game.hpp"
#include "move.hpp"
#include "move_timer.hpp"

namespace std {
    class mutex;
}

class ChessEngineNotifier;

//
// Represents the computer player.
//
class ChessEngine : public QObject
{
    Q_OBJECT
public:
    ChessEngine(std::shared_ptr<wisdom::Game> game,
                gsl::not_null<std::mutex*> gameMutex,
                QObject *parent = nullptr);

public slots:
    // Startup the engine. If it's the engine's turn to move, make a move.
    void init();

    // Receive events about the opponent move.
    void opponentMoved();

signals:
    // The engine made a mode.
    void engineMoved(wisdom::Move move, wisdom::Color who);

private:
    std::shared_ptr<wisdom::Game> myGame;
    gsl::not_null<std::mutex*> myGameMutex;
    void findMove();
};

#endif // CHESSENGINE_H
