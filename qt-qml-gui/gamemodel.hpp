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
#include "gamestate.hpp"

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

    Q_PROPERTY(bool thirdRepetitionDrawProposed
               READ thirdRepetitionDrawProposed
               WRITE setThirdRepetitionDrawProposed
               NOTIFY thirdRepetitionDrawProposedChanged)

    Q_PROPERTY(bool fiftyMovesWithoutProgressDrawProposed
               READ fiftyMovesWithoutProgressDrawProposed
               WRITE setFiftyMovesWithoutProgressDrawProposed
               NOTIFY fiftyMovesWithoutProgressDrawProposedChanged)

    Q_PROPERTY(bool inCheck
               READ inCheck
               WRITE setInCheck
               NOTIFY inCheckChanged)

    Q_PROPERTY(bool whiteIsComputer
               READ whiteIsComputer
               WRITE setWhiteIsComputer
               NOTIFY whiteIsComputerChanged)

    Q_PROPERTY(bool blackIsComputer
               READ blackIsComputer
               WRITE setBlackIsComputer
               NOTIFY blackIsComputerChanged)

    Q_PROPERTY(int maxDepth
               READ maxDepth
               WRITE setMaxDepth
               NOTIFY maxDepthChanged)

    Q_PROPERTY(int maxSearchTime
               READ maxSearchTime
               WRITE setMaxSearchTime
               NOTIFY maxSearchTimeChanged)

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

    void setThirdRepetitionDrawProposed(bool drawProposedToHuman);
    auto thirdRepetitionDrawProposed() -> bool;

    void setFiftyMovesWithoutProgressDrawProposed(bool drawProposedToHuman);
    auto fiftyMovesWithoutProgressDrawProposed() -> bool;

    void setWhiteIsComputer(bool newWhiteIsComputer);
    auto whiteIsComputer() -> bool;

    void setBlackIsComputer(bool newBlackIsComputer);
    auto blackIsComputer() -> bool;

    void setMaxDepth(int maxDepth);
    auto maxDepth() -> int;

    void setMaxSearchTime(int maxSearchTime);
    auto maxSearchTime() -> int;

signals:
    // The game object here is readonly.
    void gameStarted(gsl::not_null<const ChessGame*> game);

    // A new game state was created. This game is sent to the new thread.
    // Note this is subtely different from gameStarted - the pointer argument
    // here is meant for transferring ownership.
    void gameUpdated(std::shared_ptr<ChessGame> chessGame, int newGameId);

    void humanMoved(wisdom::Move move, wisdom::Color who);
    void engineMoved(wisdom::Move move, wisdom::Color who, int gameId);
    void engineConfigChanged(ChessGame::Config config);

    void currentTurnChanged();
    void gameOverStatusChanged();
    void moveStatusChanged();
    void inCheckChanged();
    void whiteIsComputerChanged();
    void blackIsComputerChanged();
    void maxDepthChanged();
    void maxSearchTimeChanged();

    // Use a property to communicate to QML and the human player:
    void thirdRepetitionDrawProposedChanged();
    void fiftyMovesWithoutProgressDrawProposedChanged();

    // Send draw response:
    void updateDrawStatus(wisdom::ProposedDrawType drawType, wisdom::Color player,
        bool accepted);

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

    void humanWantsThreefoldRepetitionDraw(bool accepted);
    void humanWantsFiftyMovesWithoutProgressDraw(bool accepted);

    void receiveChessEngineDrawStatus(wisdom::ProposedDrawType drawType,
        wisdom::Color who, bool accepted);

    void applicationExiting();

    void updateEngineConfig();

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
    bool myThirdRepetitionDrawProposed = false;
    bool myFiftyMovesWithProgressDrawProposed = false;
    bool myWhiteIsComputer = false;
    bool myBlackIsComputer = true;
    int myMaxDepth;
    int myMaxSearchTime;

    void init();
    void setupNewEngineThread();

    void movePieceWithPromotion(int srcRow, int srcColumn,
                                int dstRow, int dstColumn, std::optional<wisdom::Piece> piece);

    // Propose a draw to either the human or computer.
    void proposeDraw(wisdom::Player player, wisdom::ProposedDrawType drawType);

    // Returns the color of the next turn:
    auto updateChessEngineForHumanMove(wisdom::Move selectedMove) -> wisdom::Color;

    // Update the current turn to the new color.
    void updateCurrentTurn(wisdom::Color newColor);

    // Emit appropriate player moved signal, or delay it for a draw proposal.
    void handleMove(wisdom::Player playerType, wisdom::Move move, wisdom::Color who);

    // Update the internal game state after user changes config or starts a new game:
    void updateInternalGameState();

    // Update the displayed state of the game.
    void updateDisplayedGameState();

    // Set up and trigger the state update.
    void notifyInternalGameStateUpdated();

    // Set the proposed draw type:
    void setProposedDrawTypeAcceptance(wisdom::ProposedDrawType drawType, bool accepted);

    // Get the configuration for the game.
    ChessGame::Config gameConfig() const;
};

#endif // GAMEMODEL_H
