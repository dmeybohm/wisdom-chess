#ifndef CHESSENGINE_H
#define CHESSENGINE_H

#include <QObject>
#include <chrono>
#include <functional>
#include <memory>

#include "chess_game.hpp"
#include "game.hpp"
#include "logger.hpp"
#include "move.hpp"
#include "move_timer.hpp"

class QmlEngineGameStatusUpdate;

//
// Represents the computer player.
//
class ChessEngine : public QObject
{
    Q_OBJECT

public:
    ChessEngine (std::shared_ptr<ChessGame> game, int game_id, QObject* parent = nullptr);

    static constexpr wisdom::Logger::LogLevel Log_Level =
#ifdef NDEBUG
        wisdom::Logger::LogLevel_Info
#else
        wisdom::Logger::LogLevel_Debug
#endif
        ;

    struct ChessEngineLogger : wisdom::Logger
    {
        void debug (const std::string& string) const override
        {
        }

        void info (const std::string& string) const override;
    };

public slots:
    // Startup the engine. If it's the engine's turn to move, make a move.
    void init();

    // Exit the thread. Called upon application exit to cleanup.
    void quit();

    // Receive events about the opponent move.
    void opponentMoved (wisdom::Move move, wisdom::Color who);

    // Receive our own move:
    void receiveEngineMoved (wisdom::Move move, wisdom::Color who, int gameId);

    // Receive the draw status:
    void receiveDrawStatus (wisdom::ProposedDrawType drawType, wisdom::Color player, bool accepted);

    // Update the whole chess game state. The ownership of the game is taken.
    void reloadGame (std::shared_ptr<ChessGame> newGame, int newGameId);

    // Update the config of the game. Also update the notifier in case we
    // had to interrupt the engine.
    void updateConfig (ChessGame::Config config,
                       const wisdom::MoveTimer::PeriodicFunction& notifier);

signals:
    // The engine made a move.
    void engineMoved (wisdom::Move move, wisdom::Color who, int gameId);

    // There are no available moves.
    void noMovesAvailable();

    // Send draw response:
    void updateDrawStatus (wisdom::ProposedDrawType drawType, wisdom::Color player, bool accepted);

private:
    std::shared_ptr<ChessGame> my_game;

    bool my_is_game_over = false;

    // Identify games so that signals from them can be filtered due to being async.
    int my_game_id;

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
    void handlePotentialDrawPosition (wisdom::ProposedDrawType proposedDrawType, wisdom::Color who);

    friend class QmlEngineGameStatusUpdate;
};

#endif // CHESSENGINE_H
