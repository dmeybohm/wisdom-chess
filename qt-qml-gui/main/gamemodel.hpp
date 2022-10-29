#ifndef WISDOM_CHESS_GAMEMODEL_H
#define WISDOM_CHESS_GAMEMODEL_H

#include <QObject>
#include <QThread>
#include <QString>
#include <QTimer>

#include "ui_types.hpp"
#include "chessgame.hpp"
#include "ui_settings.hpp"
#include "game_settings.hpp"

class GameModel : public QObject
{
    using DrawStatus = wisdom::ui::DrawByRepetitionStatus;

    Q_OBJECT

    Q_PROPERTY(wisdom::ui::Color currentTurn
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

    Q_PROPERTY(bool inCheck
               READ inCheck
               WRITE setInCheck
               NOTIFY inCheckChanged)

    Q_PROPERTY(UISettings uiSettings
               READ uiSettings
               WRITE setUISettings
               NOTIFY uiSettingsChanged)

    Q_PROPERTY(GameSettings gameSettings
               READ gameSettings
               WRITE setGameSettings
               NOTIFY gameSettingsChanged)

    Q_PROPERTY(wisdom::ui::DrawByRepetitionStatus thirdRepetitionDrawStatus
                       READ thirdRepetitionDrawStatus
                       WRITE setThirdRepetitionDrawStatus
                       NOTIFY thirdRepetitionDrawStatusChanged)

    Q_PROPERTY(wisdom::ui::DrawByRepetitionStatus fiftyMovesDrawStatus
                       READ fiftyMovesDrawStatus
                       WRITE setFiftyMovesDrawStatus
                       NOTIFY fiftyMovesDrawStatusChanged)

public:
    explicit GameModel(QObject *parent = nullptr);
    ~GameModel() override;

    Q_INVOKABLE void start();
    Q_INVOKABLE bool needsPawnPromotion(int srcRow, int srcColumn, int dstRow, int dstColumn);
    Q_INVOKABLE void restart();

    [[nodiscard]] auto currentTurn() const -> wisdom::ui::Color;
    void setCurrentTurn(wisdom::ui::Color newColor);

    void setGameOverStatus(const QString& newStatus);
    [[nodiscard]] auto gameOverStatus() const -> QString;

    void setMoveStatus(const QString& newStatus);
    [[nodiscard]] auto moveStatus() const -> QString;

    void setInCheck(bool newInCheck);
    [[nodiscard]] auto inCheck() const -> bool;

    void setThirdRepetitionDrawStatus(wisdom::ui::DrawByRepetitionStatus drawStatus);
    [[nodiscard]] auto thirdRepetitionDrawStatus() const
        -> wisdom::ui::DrawByRepetitionStatus;

    void setFiftyMovesDrawStatus(wisdom::ui::DrawByRepetitionStatus drawStatus);
    [[nodiscard]] auto fiftyMovesDrawStatus() const
        -> wisdom::ui::DrawByRepetitionStatus;

    void setUISettings(const UISettings& settings);
    [[nodiscard]] auto uiSettings() const -> const UISettings&;
    Q_INVOKABLE UISettings cloneUISettings();

    [[nodiscard]] auto gameSettings() const -> const GameSettings&;
    void setGameSettings(const GameSettings &newGameSettings);
    Q_INVOKABLE GameSettings cloneGameSettings();

signals:
    // The game object here is readonly.
    void gameStarted(gsl::not_null<const ChessGame*> game);

    // A new game state was created. This game is sent to the new thread.
    // Note this is subtely different from gameStarted - the pointer argument
    // here is meant for transferring ownership.
    void gameUpdated(std::shared_ptr<ChessGame> chessGame, int newGameId);

    void humanMoved(wisdom::Move move, wisdom::Color who);
    void engineMoved(wisdom::Move move, wisdom::Color who, int gameId);
    void engineConfigChanged(ChessGame::Config config,
                             wisdom::MoveTimer::PeriodicFunction newFunc);

    void currentTurnChanged();
    void gameOverStatusChanged();
    void moveStatusChanged();
    void inCheckChanged();

    void uiSettingsChanged();
    void gameSettingsChanged();

    // Use a property to communicate to QML and the human player:
    void thirdRepetitionDrawStatusChanged();
    void fiftyMovesDrawStatusChanged();

    // Send draw response:
    void updateDrawStatus(wisdom::ProposedDrawType drawType, wisdom::Color player,
                          bool accepted);

    // Termination of the thread has started.
    void terminationStarted();

public slots:
    void movePiece(int srcRow, int srcColumn,
                   int dstRow, int dstColumn);

    void engineThreadMoved(wisdom::Move move, wisdom::Color who,
                           int engineId);

    void promotePiece(int srcRow, int srcColumn,
                      int dstRow, int dstColumn, wisdom::ui::PieceType pieceType);

    void receiveChessEngineDrawStatus(wisdom::ProposedDrawType drawType,
        wisdom::Color who, bool accepted);

    void applicationExiting();

    void updateEngineConfig();

private:
    // The game is duplicated across the main thread and the chess engine thread.
    // So, the main thread has a copy of the game and so does the engine.
    // When updates to the engine occur, signals are sent between the threads to synchronize.
    std::unique_ptr<ChessGame> myChessGame;

    // The chess game id. We could sometimes receive moves from a previous game that were
    // queued because the signals are asynchronous. This lets us discard those signals.
    std::atomic<int> myGameId = 1;

    // We identify each configuration by an Id so that when we change configs,
    // The chess engine thread can be interrupted to load the new config sooner.
    std::atomic<int> myConfigId = 1;

    // The chess engine runs in this thread:
    QThread* myChessEngineThread = nullptr;

    wisdom::ui::Color myCurrentTurn;
    QString myGameOverStatus {};
    QString myMoveStatus {};
    bool myInCheck = false;

    UISettings myUISettings {};
    GameSettings myGameSettings {};

    DrawStatus myThirdRepetitionDrawStatus = DrawStatus::NotReached;
    DrawStatus myFiftyMovesDrawStatus = DrawStatus::NotReached;

    void init();
    void setupNewEngineThread();

    void movePieceWithPromotion(int srcRow, int srcColumn,
                                int dstRow, int dstColumn, std::optional<wisdom::Piece> piece);

    // Returns the color of the next turn:
    auto updateChessEngineForHumanMove(wisdom::Move selectedMove) -> wisdom::Color;

    // Build the notifier that is used to interrupt the thread.
    [[nodiscard]] auto buildNotifier() const
        -> wisdom::MoveTimer::PeriodicFunction;

    // Update the current turn to the new color.
    void updateCurrentTurn(wisdom::Color newColor);

    // Emit appropriate player moved signal, or delay it for a draw proposal.
    void handleMove(wisdom::Player playerType, wisdom::Move move, wisdom::Color who);

    // Update the displayed state of the game.
    void updateDisplayedGameState();

    // Set up and trigger the state update.
    void notifyInternalGameStateUpdated();

    // Set the proposed draw type:
    void setProposedDrawStatus(wisdom::ProposedDrawType drawType, DrawStatus status);

    // Get the configuration for the game.
    auto gameConfig() const -> ChessGame::Config;

    // Update the internal game state after user changes config or starts a new game:
    void updateInternalGameState();

    // Reset the state for a new game.
    void resetStateForNewGame();
};

#endif // WISDOM_CHESS_GAMEMODEL_H
