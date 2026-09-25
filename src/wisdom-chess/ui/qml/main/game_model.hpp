#pragma once

#include <QElapsedTimer>
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
class ChessEngine;

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

    // In the mirror enum QML knows; the view-model's enum has no meta-object.
    Q_PROPERTY (wisdom::ui::QmlDrawByRepetitionStatus thirdRepetitionDrawStatus
        READ qmlThirdRepetitionDrawStatus
        WRITE setQmlThirdRepetitionDrawStatus
        NOTIFY thirdRepetitionDrawStatusChanged)

    Q_PROPERTY (wisdom::ui::QmlDrawByRepetitionStatus fiftyMovesDrawStatus
        READ qmlFiftyMovesDrawStatus
        WRITE setQmlFiftyMovesDrawStatus
        NOTIFY fiftyMovesDrawStatusChanged)

    // How long a piece takes to move, in milliseconds. The board animates
    // for this long, and a move is held back until the one before it has
    // finished animating.
    Q_PROPERTY (int animationDelay
        READ animationDelay
        WRITE setAnimationDelay
        NOTIFY animationDelayChanged)

    // How long the rook of a castling move waits before it follows the king.
    Q_PROPERTY (int castlingRookPause
        READ castlingRookPause
        CONSTANT)

public:
    // The board animates a move for this long, in milliseconds.
    static constexpr int Default_Animation_Delay = 200;

    // The rook of a castling move waits this long before it follows the
    // king, and then animates for the animation delay.
    static constexpr int Castling_Rook_Pause = 225;

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

    Q_INVOKABLE void pause();
    Q_INVOKABLE void unpause();

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

    [[nodiscard]] auto
    qmlThirdRepetitionDrawStatus() const
        -> wisdom::ui::QmlDrawByRepetitionStatus;

    void setQmlThirdRepetitionDrawStatus (wisdom::ui::QmlDrawByRepetitionStatus draw_status);

    [[nodiscard]] auto
    qmlFiftyMovesDrawStatus() const
        -> wisdom::ui::QmlDrawByRepetitionStatus;

    void setQmlFiftyMovesDrawStatus (wisdom::ui::QmlDrawByRepetitionStatus draw_status);

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

    [[nodiscard]] auto
    animationDelay() const
        -> int;
    void setAnimationDelay (int new_delay);

    [[nodiscard]] auto
    castlingRookPause() const
        -> int;

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
    void animationDelayChanged();

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

    // Resume searching after being unpaused.
    void resumeSearching();

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

    // Whether an engine move is waiting for the board to finish animating.
    [[nodiscard]] auto
    isHoldingAMove() const
        -> bool;

    void onInCheckChanged() override;
    void onMoveStatusChanged() override;
    void onGameOverStatusChanged() override;
    void onCurrentTurnChanged() override;
    void onThirdRepetitionDrawStatusChanged() override;
    void onFiftyMovesDrawStatusChanged() override;

private:
    void init();
    void setupNewEngineThread();

    // Stop the engine thread and, where possible, wait for it to finish.
    void stopEngineThread();

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

    // Apply an engine move to the displayed game and announce it.
    void showEngineMove (
        wisdom::Move move,
        wisdom::Color who
    );

    // How much of the last move's animation is left, in milliseconds.
    [[nodiscard]] auto
    remainingAnimation() const
        -> int;

    // Show the move that was held back, if it still belongs to this game.
    void showHeldMove();

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

    // The engine QObject moved onto my_chess_engine_thread. It is normally
    // deleted via QThread::finished -> deleteLater(), which only fires once
    // the thread has actually run and exited its event loop. Tracked here
    // so it can be deleted directly if the thread never started.
    ChessEngine* my_chess_engine = nullptr;

    // Whether start() has ever been called for the current engine thread.
    bool my_engine_thread_started = false;

    // Whether the game is paused (e.g. menu or dialog is open).
    // Read by the engine thread's periodic function to cancel searches.
    std::atomic<bool> my_paused { false };

    // An engine move that arrived while the move before it was still
    // animating. Only one can be outstanding: the engine searches again
    // once the move it found has been shown.
    struct HeldMove
    {
        wisdom::Move move;
        wisdom::Color who;
        int game_id;
    };

    // Since the last move was shown, and how long it animates for.
    QElapsedTimer my_animating_since;
    int my_animation_duration = 0;

    std::optional<HeldMove> my_held_move {};
    QTimer my_hold_timer;

    int my_animation_delay = Default_Animation_Delay;

    UISettings my_ui_settings {};
    GameSettings my_game_settings {};
};
