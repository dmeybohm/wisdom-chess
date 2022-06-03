#ifndef CHESSENGINE_H
#define CHESSENGINE_H

#include <QObject>
#include <functional>
#include <memory>
#include <chrono>

#include "chessgame.hpp"
#include "game.hpp"
#include "move.hpp"
#include "move_timer.hpp"

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

    // Receive the draw status:
    void receiveDrawStatus(wisdom::ProposedDrawType drawType, wisdom::Color player,
                           bool accepted);

    // Update the whole chess game state. The ownership of the game is taken.
    void reloadGame(std::shared_ptr<ChessGame> newGame, int newGameId);

    // Update the config of the game.
    void updateConfig(ChessGame::Config config, int newGameId);

signals:
    // The engine made a move.
    void engineMoved(wisdom::Move move, wisdom::Color who, int gameId);

    // There are no available moves.
    void noMovesAvailable();

    // Send draw response:
    void updateDrawStatus(wisdom::ProposedDrawType drawType, wisdom::Color player,
                          bool accepted);

private:
    std::shared_ptr<ChessGame> myGame;
    int myGameId;
    bool myIsGameOver = false;

    void findMove();

    // Perform some operations when the game status has updated.
    // Return the status.
    auto gameStatusTransition() -> wisdom::GameStatus;

    // We reached a draw position.
    //
    // If the engine declines a draw, we have to wait for a response from the
    // other player (either the engine itself or the human) before continuing
    // the search.
    //
    void handlePotentialDrawPosition(wisdom::ProposedDrawType proposedDrawType,
                                     wisdom::Color who);

};

#endif // CHESSENGINE_H
