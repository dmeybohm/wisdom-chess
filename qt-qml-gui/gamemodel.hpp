#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <QObject>
#include <QThread>
#include <QString>
#include <QTimer>

#include "chesscolor.hpp"
#include "chessengine.hpp"
#include "chessgame.hpp"
#include "piecesmodel.hpp"

class GameModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(wisdom::chess::ChessColor currentTurn
               READ currentTurn
               WRITE setCurrentTurn
               NOTIFY currentTurnChanged)
    Q_PROPERTY(QString gameOverStatus
               READ gameOverStatus
               WRITE setGameOverStatus
               NOTIFY gameOverStatusChanged)
    Q_PROPERTY(QString moveStatus
               READ moveStatus
               WRITE setMoveStatus
               NOTIFY moveStatusChanged)
    Q_PROPERTY(bool drawProposedToHuman
               READ drawProposedToHuman
               WRITE setDrawProposedToHuman
               NOTIFY drawProposedToHumanChanged)
    Q_PROPERTY(bool inCheck
               READ inCheck
               WRITE setInCheck
               NOTIFY inCheckChanged)

public:
    explicit GameModel(QObject *parent = nullptr);
    ~GameModel() override;

    Q_INVOKABLE void start();
    Q_INVOKABLE bool needsPawnPromotion(int srcRow, int srcColumn, int dstRow, int dstColumn);
    Q_INVOKABLE void restart();

    wisdom::chess::ChessColor currentTurn();
    void setCurrentTurn(wisdom::chess::ChessColor newColor);

    void setGameOverStatus(const QString& newStatus);
    auto gameOverStatus() -> QString;

    void setMoveStatus(const QString& newStatus);
    auto moveStatus() -> QString;

    void setInCheck(const bool newInCheck);
    auto inCheck() -> bool;

    void setDrawProposedToHuman(bool drawProposedToHuman);
    auto drawProposedToHuman() -> bool;

signals:
    // The game object here is readonly.
    void gameStarted(gsl::not_null<const ChessGame*> game);

    // A new game state was created. This game is sent to the new thread.
    // Note this is subtely different from gameStarted - the pointer argument
    // here is meant for transferring ownership.
    void gameUpdated(std::shared_ptr<ChessGame> chessGame, int newGameId);

    void humanMoved(wisdom::Move move, wisdom::Color who);
    void engineMoved(wisdom::Move move, wisdom::Color who, int gameId);

    void currentTurnChanged();
    void gameOverStatusChanged();
    void moveStatusChanged();
    void inCheckChanged();

    // Use a property to communicate to QML and the human player:
    void drawProposedToHumanChanged();

    // Use a raw signal to communicate to the thread engine on the other thread.
    // We don't want to send on toggling the boolean.
    void proposeDrawToEngine();

    // Termination of the thread has started.
    void terminationStarted();

public slots:
    void movePiece(int srcRow, int srcColumn,
                   int dstRow, int dstColumn);

    void engineThreadMoved(wisdom::Move move, wisdom::Color who,
                           int engineid);

    void promotePiece(int srcRow, int srcColumn, int dstRow, int dstColumn, QString pieceType);

    void applicationExiting();

    void updateGameStatus();

    void drawProposalResponse(bool accepted);

private:
    // The game is duplicated across the main thread and the chess engine thread.
    // So, the main thread has a copy of the game and so does the engine.
    // When updates to the engine occur, the game is sent via a signal and the engine
    // replaces the new copy.
    std::unique_ptr<ChessGame> myChessGame;

    // The chess game id. We could sometimes receive moves from a previous game that were
    // queued because the signals are asynchronous. This lets us discard those signals.
    std::atomic<int> myGameId = 1;

    // The chess engine runs in this thread, and grabs the game mutext as needed:
    QThread* myChessEngineThread;
    wisdom::chess::ChessColor myCurrentTurn;
    QString myGameOverStatus {};
    QString myMoveStatus {};
    bool myInCheck = false;
    bool myDrawProposedToHuman = false;

    // last move before the draw proposal
    std::optional<std::function<void()>> myLastDelayedMoveSignal {};
    QTimer* myDelayedMoveTimer = nullptr;

    // Track whether draw was proposed to only propose a draw once per game:
    bool myDrawEverProposed = false;

    void init();
    void setupNewEngineThread();

    void movePieceWithPromotion(int srcRow, int srcColumn,
                                int dstRow, int dstColumn, std::optional<wisdom::Piece> piece);

    // Propose a draw to either the human or computer.
    void proposeDraw(wisdom::Player player);

    // Returns the color of the next turn:
    auto updateChessEngineForHumanMove(wisdom::Move selectedMove) -> wisdom::Color;

    // Update the current turn to the new color.
    void updateCurrentTurn(wisdom::Color newColor);

    // Emit appropriate player moved signal, or delay it for a draw proposal.
    void checkForDrawAndEmitPlayerMoved(wisdom::Player playerType, wisdom::Move move,
                                        wisdom::Color who);
};

#endif // GAMEMODEL_H
