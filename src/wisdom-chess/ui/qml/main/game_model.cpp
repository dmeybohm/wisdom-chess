#include <QDebug>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/game.hpp"

#include "wisdom-chess/ui/qml/main/game_model.hpp"
#include "wisdom-chess/ui/qml/main/chess_engine.hpp"
#include "wisdom-chess/ui/qml/main/ui_settings.hpp"

using namespace wisdom;
namespace ui = wisdom::ui;
using gsl::not_null;
using std::atomic;
using std::make_shared;
using std::optional;
using std::pair;
using std::shared_ptr;
using wisdom::nonnull_observer_ptr;

GameModel::GameModel (QObject* parent)
    : QObject (parent)
    , my_chess_engine_thread { nullptr }
{
    my_chess_game = ChessGame::fromPlayers (
        Player::Human,
        Player::ChessEngine,
        ChessGame::Config::fromGameSettings (my_game_settings)
    );
    init();
}

GameModel::~GameModel()
{
    delete my_chess_engine_thread;
}

auto
GameModel::getGame()
    -> observer_ptr<Game>
{
    return my_chess_game->state();
}

auto
GameModel::getGame() const
    -> observer_ptr<const Game>
{
    return my_chess_game->state();
}

void GameModel::init()
{
    auto gameState = my_chess_game->state();
    setCurrentTurn (gameState->getCurrentTurn());

    setupNewEngineThread();
}

void GameModel::setupNewEngineThread()
{
    delete my_chess_engine_thread;

    // Initialize a new Game for the chess engine.
    // Any changes in the game config will be updated over a signal.
    auto computer_chess_game = my_chess_game->clone();
    computer_chess_game->setPeriodicFunction (buildNotifier());

    auto chess_engine = new ChessEngine { std::move (computer_chess_game), gameId() };

    my_chess_engine_thread = new QThread();

    // Connect event handlers for the computer and human making moves:
    connect (this, &GameModel::humanMoved,
             chess_engine, &ChessEngine::opponentMoved);
    connect (chess_engine, &ChessEngine::engineMoved,
             this, &GameModel::engineThreadMoved);

    // Update draw status between engine and game model:
    connect (chess_engine, &ChessEngine::updateDrawStatus,
             this, &GameModel::receiveChessEngineDrawStatus);
    connect (this, &GameModel::updateDrawStatus,
             chess_engine, &ChessEngine::receiveDrawStatus);

    // Initialize the engine when the engine thread starts:
    connect (my_chess_engine_thread, &QThread::started,
             chess_engine, &ChessEngine::init);

    // If the engine finds no moves available, check whether the game is over.
    connect (chess_engine, &ChessEngine::noMovesAvailable,
             this, [this]() { updateDisplayedGameState(); });

    // Connect the engine's move back to itself in case it's playing itself:
    // (it will return early if it's not)
    connect (this, &GameModel::engineMoved,
             chess_engine, &ChessEngine::receiveEngineMoved);

    // exit event loop from engine thread when we start exiting:
    connect (this, &GameModel::terminationStarted,
             chess_engine, &ChessEngine::quit);

    // Update the engine's config when user changes it:
    connect (this, &GameModel::engineConfigChanged,
             chess_engine, &ChessEngine::updateConfig);

    // Cleanup chess engine when chess engine thread exits:
    connect (my_chess_engine_thread, &QThread::finished,
             chess_engine, &QObject::deleteLater);

    // When creating a new game, send a copy of the new ChessGame to the Engine:
    connect (this, &GameModel::gameUpdated,
             chess_engine, &ChessEngine::reloadGame);

    // Resume searching when the game is unpaused:
    connect (this, &GameModel::resumeSearching,
             chess_engine, &ChessEngine::init);

    // Move the ownership of the engine to the engine thread so slots run on that thread:
    chess_engine->moveToThread (my_chess_engine_thread);
}

void GameModel::start()
{
    emit gameStarted (my_chess_game.get());

    updateEngineConfig();
    my_chess_engine_thread->start();
}

auto GameModel::browserOriginUrl() -> QString
{
#ifdef EMSCRIPTEN
    char* origin = reinterpret_cast<char*>(EM_ASM_PTR({
        var str = window.location.origin;
        var len = lengthBytesUTF8(str) + 1;
        var buf = _malloc(len);
        stringToUTF8(str, buf, len);
        return buf;
    }));
    QString result = QString::fromUtf8 (origin);
    free (origin);
    return result;
#else
    return QString {};
#endif
}

void GameModel::restart()
{
    setGameOverStatus ("");

    qDebug() << "Creating new chess game";

    my_chess_game = std::move (
        ChessGame::fromPlayers (
            my_chess_game->state()->getPlayer (Color::White),
            my_chess_game->state()->getPlayer (Color::Black),
            gameConfig()
        )
    );

    // Abort searches and discard any queued signals from them if we receive
    // them later.
    incrementGameId();

    std::shared_ptr<ChessGame> computer_chess_game = std::move (my_chess_game->clone());
    computer_chess_game->setPeriodicFunction (buildNotifier());

    // send copy of the new game state to the chess engine thread:
    emit gameUpdated (computer_chess_game, gameId());

    // Notify other objects in this thread about the new game:
    emit gameStarted (my_chess_game.get());

    // Update the config to update the notifier to use the new game Id:
    updateEngineConfig();

    setCurrentTurn (my_chess_game->state()->getCurrentTurn());
    resetStateForNewGame();
    updateDisplayedGameState();
}

void
GameModel::movePiece (
    int src_row,
    int src_column,
    int dst_row,
    int dst_column
) {
    movePieceWithPromotion (src_row, src_column, dst_row, dst_column, {});
}

void
GameModel::engineThreadMoved (
    wisdom::Move move,
    wisdom::Color who,
    int game_id
) {
    // validate this signal was not sent by an old thread:
    if (game_id != gameId())
    {
        qDebug() << "engineThreadMoved(): Ignored signal from invalid engine.";
        return;
    }

    auto game = my_chess_game->state();
    game->move (move);

    updateDisplayedGameState();
    updateCurrentTurn (wisdom::colorInvert (who));

    // re-emit single-threaded signal to listeners:
    handleMove (Player::ChessEngine, move, who);
}

void
GameModel::promotePiece (
    int src_row,
    int src_column,
    int dst_row,
    int dst_column,
    ui::PieceType piece_type
) {
    movePieceWithPromotion (
        src_row,
        src_column,
        dst_row,
        dst_column,
        mapPiece (piece_type)
    );
}

void
GameModel::movePieceWithPromotion (
    int srcRow,
    int srcColumn,
    int dstRow,
    int dstColumn,
    optional<wisdom::Piece> pieceType
) {
    auto [optional_move, who]
        = my_chess_game->moveFromCoordinates (srcRow, srcColumn, dstRow, dstColumn, pieceType);
    if (!optional_move.has_value())
    {
        return;
    }
    auto move = *optional_move;
    if (!my_chess_game->isLegalMove (move))
    {
        setMoveStatus ("Illegal move");
        return;
    }

    auto new_color = updateChessEngineForHumanMove (move);
    updateDisplayedGameState();
    updateCurrentTurn (new_color);
    handleMove (wisdom::Player::Human, move, who);
}

auto
GameModel::needsPawnPromotion (
    int src_row,
    int src_column,
    int dst_row,
    int dst_column
)
    -> bool
{
    return GameViewModelBase::needsPawnPromotion (src_row, src_column, dst_row, dst_column);
}

void GameModel::pause()
{
    my_paused.store (true);
}

void GameModel::unpause()
{
    my_paused.store (false);
    emit resumeSearching();
}

void GameModel::applicationExiting()
{
    qDebug() << "Trying to exit application...";

    // End the thread by changing the game id:
    incrementGameId();
    qDebug() << "Terminated start...";
    emit terminationStarted();

    // For desktop, we need to wait here before exiting. But on web that will hang.
#ifndef EMSCRIPTEN
    my_chess_engine_thread->wait();
#endif

    qDebug() << "Termination ended.";
}

void GameModel::updateEngineConfig()
{
    my_config_id++;

    emit engineConfigChanged (gameConfig(), buildNotifier());
}

auto
GameModel::updateChessEngineForHumanMove (
    Move selected_move
)
    -> wisdom::Color
{
    auto game_state = my_chess_game->state();

    game_state->move (selected_move);
    return game_state->getCurrentTurn();
}

auto
GameModel::gameId() const
    -> int
{
    return my_game_id.load();
}

void GameModel::incrementGameId()
{
    my_game_id++;
}

auto
GameModel::buildNotifier() const
    -> MoveTimer::PeriodicFunction
{
    auto initial_game_id = gameId();
    auto initial_config_id = my_config_id.load();
    const auto* game_id_ptr = &my_game_id;
    const auto* config_id_ptr = &my_config_id;
    const auto* paused_ptr = &my_paused;

    return (
        [game_id_ptr, initial_game_id, config_id_ptr,
         initial_config_id, paused_ptr] (not_null<MoveTimer*> move_timer)
        {
            // This runs in the ChessEngine thread.
            auto current_config_id = config_id_ptr->load();

            // Check if the game is paused (e.g. menu or dialog is open):
            if (paused_ptr->load())
            {
                move_timer->setCancelled (true);
                return;
            }

            // Check if config has changed:
            if (initial_config_id != current_config_id)
            {
                qDebug() << "Setting timeout to break the loop. (Config changed)";

                // Discard the results of the search. The GameModel will send
                // an updateConfig signal to fire off a new search with the new
                // config.
                move_timer->setCancelled (true);
            }

            // Check if game has changed. If so, the game is over.
            if (initial_game_id != game_id_ptr->load())
            {
                qDebug() << "Setting timeout to break the loop. (Game ended)";
                move_timer->setCancelled (true);
            }
        });
}

void GameModel::updateCurrentTurn (Color new_color)
{
    setCurrentTurn (new_color);
}

void
GameModel::handleMove (
    Player player_type,
    Move move,
    Color who
) {
    if (player_type == wisdom::Player::ChessEngine)
    {
        emit engineMoved (move, who, gameId());
    }
    else
    {
        emit humanMoved (move, who);
    }
}

void GameModel::updateInternalGameState()
{
    my_chess_game->setPlayers (mapPlayer (my_game_settings.whitePlayer()),
                               mapPlayer (my_game_settings.blackPlayer()));
    my_chess_game->setConfig (gameConfig());
    notifyInternalGameStateUpdated();
}

void GameModel::notifyInternalGameStateUpdated()
{
    auto game_state = my_chess_game->state();

    updateCurrentTurn (game_state->getCurrentTurn());
    if (!gameOverStatus().empty())
    {
        return;
    }

    updateEngineConfig();
}

auto
GameModel::gameConfig() const
    -> ChessGame::Config
{
    return ChessGame::Config {
        my_chess_game->state()->getPlayers(),
        MaxDepth { my_game_settings.maxDepth() },
        chrono::seconds { my_game_settings.maxSearchTime() },
    };
}

auto
GameModel::qmlCurrentTurn() const
    -> ui::Color
{
    return ui::mapColor (GameViewModelBase::currentTurn());
}

void GameModel::setQmlCurrentTurn (ui::Color new_color)
{
    setCurrentTurn (ui::mapColor (new_color));
}

void GameModel::setQmlGameOverStatus (const QString& new_status)
{
    setGameOverStatus (new_status.toStdString());
}

auto
GameModel::qmlGameOverStatus() const
    -> QString
{
    return QString::fromStdString (gameOverStatus());
}

void GameModel::setQmlMoveStatus (const QString& new_status)
{
    setMoveStatus (new_status.toStdString());
}

auto
GameModel::qmlMoveStatus() const
    -> QString
{
    return QString::fromStdString (moveStatus());
}

void GameModel::setQmlInCheck (bool new_in_check)
{
    setInCheck (new_in_check);
}

auto
GameModel::qmlInCheck() const
    -> bool
{
    return inCheck();
}

void GameModel::onInCheckChanged()
{
    emit inCheckChanged();
}

void GameModel::onMoveStatusChanged()
{
    emit moveStatusChanged();
}

void GameModel::onGameOverStatusChanged()
{
    emit gameOverStatusChanged();
}

void GameModel::onCurrentTurnChanged()
{
    emit currentTurnChanged();
}

void GameModel::onThirdRepetitionDrawStatusChanged()
{
    emit thirdRepetitionDrawStatusChanged();
    auto status = thirdRepetitionDrawStatus();
    if (status == DrawStatus::Accepted || status == DrawStatus::Declined)
    {
        handleDrawStatusChange (ProposedDrawType::ThreeFoldRepetition, status);
    }
}

void GameModel::onFiftyMovesDrawStatusChanged()
{
    emit fiftyMovesDrawStatusChanged();
    auto status = fiftyMovesDrawStatus();
    if (status == DrawStatus::Accepted || status == DrawStatus::Declined)
    {
        handleDrawStatusChange (ProposedDrawType::FiftyMovesWithoutProgress, status);
    }
}

auto
GameModel::uiSettings() const
    -> const UISettings&
{
    return my_ui_settings;
}

void GameModel::setUISettings (const UISettings& settings)
{
    if (my_ui_settings != settings)
    {
        my_ui_settings = settings;
        emit uiSettingsChanged();
    }
}

auto
GameModel::gameSettings() const
    -> const GameSettings&
{
    return my_game_settings;
}

void GameModel::setGameSettings (const GameSettings& new_game_settings)
{
    if (my_game_settings != new_game_settings)
    {
        my_game_settings = new_game_settings;
        emit gameSettingsChanged();
        updateInternalGameState();
    }
}

auto
GameModel::cloneUISettings()
    -> UISettings
{
    return my_ui_settings;
}

auto
GameModel::cloneGameSettings()
    -> GameSettings
{
    return my_game_settings;
}

void GameModel::setQmlThirdRepetitionDrawStatus (DrawStatus draw_status)
{
    setThirdRepetitionDrawStatus (draw_status);
}

void GameModel::setQmlFiftyMovesDrawStatus (DrawStatus draw_status)
{
    setFiftyMovesDrawStatus (draw_status);
}

void
GameModel::handleDrawStatusChange (
    wisdom::ProposedDrawType draw_type,
    DrawStatus status
) {
    auto game_state = my_chess_game->state();
    auto optional_color = ui::getFirstHumanPlayerColor (game_state->getPlayers());

    assert (optional_color.has_value());
    auto who = *optional_color;
    auto opponent_color = colorInvert (who);

    bool accepted = (status == DrawStatus::Accepted);
    game_state->setProposedDrawStatus (draw_type, who, accepted);
    emit updateDrawStatus (draw_type, who, accepted);
    if (game_state->getPlayer (opponent_color) == Player::Human)
    {
        game_state->setProposedDrawStatus (draw_type, opponent_color, accepted);
        emit updateDrawStatus (draw_type, opponent_color, accepted);
    }

    updateDisplayedGameState();
}

void
GameModel::receiveChessEngineDrawStatus (
    wisdom::ProposedDrawType draw_type,
    wisdom::Color who,
    bool accepted
) {
    auto gameState = my_chess_game->state();
    gameState->setProposedDrawStatus (draw_type, who, accepted);
    updateDisplayedGameState();
}
