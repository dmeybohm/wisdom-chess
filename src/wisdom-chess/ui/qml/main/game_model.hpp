#pragma once

#include <QObject>
#include <QString>
#include <QThread>
#include <QTimer>

#include "wisdom-chess/ui/qml/main/chess_game.hpp"
#include "wisdom-chess/ui/qml/main/game_settings.hpp"
#include "wisdom-chess/ui/qml/main/ui_settings.hpp"
#include "wisdom-chess/ui/qml/main/ui_types.hpp"
#include "wisdom-chess/ui/viewmodel/game_viewmodel_base.hpp"

class QmlGameStatusUpdate;

class GameModel : public QObject, public wisdom::ui::GameViewModelBase
{
    using DrawStatus = wisdom::ui::DrawByRepetitionStatus;

    Q_OBJECT

    Q_PROPERTY (wisdom::ui::Color currentTurn
        READ qmlCurrentTurn
        WRITE setQmlCurrentTurn
        NOTIFY currentTurnChanged)

    Q_PROPERTY (QString gameOverStatus
        READ qmlGameOverStatus
        WRITE setQmlGameOverStatus
        NOTIFY gameOverStatusChanged)

    Q_PROPERTY (QString moveStatus
        READ qmlMoveStatus
        WRITE setQmlMoveStatus
        NOTIFY moveStatusChanged)

    Q_PROPERTY (bool inCheck
        READ qmlInCheck
        WRITE setQmlInCheck
        NOTIFY inCheckChanged)

    Q_PROPERTY (UISettings uiSettings
        READ uiSettings
        WRITE setUISettings
        NOTIFY uiSettingsChanged)

    Q_PROPERTY (GameSettings gameSettings
        READ gameSettings
        WRITE setGameSettings
        NOTIFY gameSettingsChanged)

    Q_PROPERTY (wisdom::ui::DrawByRepetitionStatus thirdRepetitionDrawStatus
        READ thirdRepetitionDrawStatus
        WRITE setQmlThirdRepetitionDrawStatus
        NOTIFY thirdRepetitionDrawStatusChanged)

    Q_PROPERTY (wisdom::ui::DrawByRepetitionStatus fiftyMovesDrawStatus
        READ fiftyMovesDrawStatus
        WRITE setQmlFiftyMovesDrawStatus
        NOTIFY fiftyMovesDrawStatusChanged)

public:
    explicit GameModel (QObject* parent = nullptr);
    ~GameModel() override;

    [[nodiscard]] auto
    gameId() const
        -> int;

    Q_INVOKABLE void start();
    Q_INVOKABLE QString browserOriginUrl();
    Q_INVOKABLE bool needsPawnPromotion (
        int src_row,
        int src_column,
        int dst_row,
        int dst_column
    );
    Q_INVOKABLE void restart();

    [[nodiscard]] auto
    qmlCurrentTurn() const
        -> wisdom::ui::Color;

    void setQmlCurrentTurn (wisdom::ui::Color new_color);

    void setQmlGameOverStatus (const QString& new_status);
    [[nodiscard]] auto
    qmlGameOverStatus() const
        -> QString;

    void setQmlMoveStatus (const QString& new_status);
    [[nodiscard]] auto
    qmlMoveStatus() const
        -> QString;

    void setQmlInCheck (bool new_in_check);
    [[nodiscard]] auto
    qmlInCheck() const
        -> bool;

    void setQmlThirdRepetitionDrawStatus (DrawStatus draw_status);

    void setQmlFiftyMovesDrawStatus (DrawStatus draw_status);

    void setUISettings (const UISettings& settings);
    [[nodiscard]] auto
    uiSettings() const
        -> const UISettings&;
    Q_INVOKABLE UISettings cloneUISettings();

    [[nodiscard]] auto
    gameSettings() const
        -> const GameSettings&;
    void setGameSettings (const GameSettings& new_game_settings);
    Q_INVOKABLE GameSettings cloneGameSettings();

signals:
    // The game object here is readonly.
    void gameStarted (
        gsl::not_null<const ChessGame*> game
    );

    // A new game state was created. This game is sent to the new thread.
    // Note this is subtely different from gameStarted - the pointer argument
    // here is meant for transferring ownership.
    void gameUpdated (
        std::shared_ptr<ChessGame> chessGame,
        int newGameId
    );

    void humanMoved (
        wisdom::Move move,
        wisdom::Color who
    );
    void engineMoved (
        wisdom::Move move,
        wisdom::Color who,
        int gameId
    );
    void engineConfigChanged (
        ChessGame::Config config,
        wisdom::MoveTimer::PeriodicFunction newFunc
    );

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
    void updateDrawStatus (
        wisdom::ProposedDrawType drawType,
        wisdom::Color player,
        bool accepted
    );

    // Termination of the thread has started.
    void terminationStarted();

public slots:
    void movePiece (
        int src_row,
        int src_column,
        int dst_row,
        int dst_column
    );

    void engineThreadMoved (
        wisdom::Move move,
        wisdom::Color who,
        int game_id
    );

    void promotePiece (
        int src_row,
        int src_column,
        int dst_row,
        int dst_column,
        wisdom::ui::PieceType piece_type
    );

    void receiveChessEngineDrawStatus (
        wisdom::ProposedDrawType draw_type,
        wisdom::Color who,
        bool accepted
    );

    void applicationExiting();

    void updateEngineConfig();

protected:
    [[nodiscard]] auto
    getGame()
        -> wisdom::observer_ptr<wisdom::Game> override;

    [[nodiscard]] auto
    getGame() const
        -> wisdom::observer_ptr<const wisdom::Game> override;

    void onInCheckChanged() override;
    void onMoveStatusChanged() override;
    void onGameOverStatusChanged() override;
    void onCurrentTurnChanged() override;
    void onThirdRepetitionDrawStatusChanged() override;
    void onFiftyMovesDrawStatusChanged() override;

private:
    void init();
    void setupNewEngineThread();

    void movePieceWithPromotion (
        int srcRow,
        int srcColumn,
        int dstRow,
        int dstColumn,
        std::optional<wisdom::Piece> piece
    );

    // Returns the color of the next turn:
    [[nodiscard]] auto
    updateChessEngineForHumanMove (wisdom::Move selected_move)
        -> wisdom::Color;

    // Build the notifier that is used to interrupt the thread.
    [[nodiscard]] auto
    buildNotifier() const
        -> wisdom::MoveTimer::PeriodicFunction;

    // Update the current turn to the new color.
    void updateCurrentTurn (wisdom::Color new_color);

    // Emit appropriate player moved signal, or delay it for a draw proposal.
    void handleMove (
        wisdom::Player player_type,
        wisdom::Move move,
        wisdom::Color who
    );

    // Set up and trigger the state update.
    void notifyInternalGameStateUpdated();

    // Handle draw status changes and emit signals to engine.
    void handleDrawStatusChange (
        wisdom::ProposedDrawType draw_type,
        DrawStatus status
    );

    // Get the configuration for the game.
    [[nodiscard]] auto
    gameConfig() const
        -> ChessGame::Config;

    // Update the internal game state after user changes config or starts a new game:
    void updateInternalGameState();

    // Manage the game status:
    friend class QmlGameStatusUpdate;

private:
    void incrementGameId();

    // The game is duplicated across the main thread and the chess engine thread.
    // So, the main thread has a copy of the game and so does the engine.
    // When updates to the engine occur, signals are sent between the threads to synchronize.
    std::unique_ptr<ChessGame> my_chess_game;

    // The chess game id. We could sometimes receive moves from a previous game that were
    // queued because the signals are asynchronous. This lets us discard those signals.
    std::atomic<int> my_game_id { 1 };

    // We identify each configuration by an Id so that when we change configs,
    // The chess engine thread can be interrupted to load the new config sooner.
    std::atomic<int> my_config_id = 1;

    // The chess engine runs in this thread:
    QThread* my_chess_engine_thread = nullptr;

    UISettings my_ui_settings {};
    GameSettings my_game_settings {};
};
