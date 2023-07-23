#include "game_model.hpp"

#include "check.hpp"
#include "chess_engine.hpp"
#include "game.hpp"
#include "ui_settings.hpp"

#include <QDebug>
#include <chrono>

using namespace wisdom;
namespace ui = wisdom::ui;
using gsl::not_null;
using std::atomic;
using std::make_shared;
using std::optional;
using std::pair;
using std::shared_ptr;

namespace
{
    auto getFirstHumanPlayerColor (Players players) -> optional<Color>
    {
        if (players[0] == Player::Human)
        {
            return Color::White;
        }
        if (players[1] == Player::Human)
        {
            return Color::Black;
        }

        return {};
    }
}

GameModel::GameModel (QObject* parent) :
        QObject (parent), my_current_turn {}, my_chess_engine_thread { nullptr }
{
    my_chess_game = ChessGame::fromPlayers (Player::Human, Player::ChessEngine,
                                            ChessGame::Config::fromGameSettings (my_game_settings));
    init();
}

GameModel::~GameModel()
{
    delete my_chess_engine_thread;
}

void GameModel::init()
{
    auto gameState = my_chess_game->state();
    setCurrentTurn (ui::mapColor (gameState->getCurrentTurn()));

    setupNewEngineThread();
}

void GameModel::setupNewEngineThread()
{
    delete my_chess_engine_thread;

    // Initialize a new Game for the chess engine.
    // Any changes in the game config will be updated over a signal.
    auto computer_chess_game = my_chess_game->clone();
    computer_chess_game->setPeriodicFunction (buildNotifier());

    auto chess_engine = new ChessEngine { std::move (computer_chess_game), my_game_id };

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
             this, &GameModel::updateDisplayedGameState);

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

    // Move the ownership of the engine to the engine thread so slots run on that thread:
    chess_engine->moveToThread (my_chess_engine_thread);
}

void GameModel::start()
{
    emit gameStarted (my_chess_game.get());

    updateEngineConfig();
    my_chess_engine_thread->start();
}

void GameModel::restart()
{
    // let other objects in this thread know about the new game:
    setGameOverStatus ("");

    qDebug() << "Creating new chess game";

    my_chess_game = std::move (
        ChessGame::fromPlayers (my_chess_game->state()->getPlayer (Color::White),
                                my_chess_game->state()->getPlayer (Color::Black), gameConfig()));

    // Abort searches and discard any queued signals from them if we receive
    // them later.
    my_game_id++;

    std::shared_ptr<ChessGame> computer_chess_game = std::move (my_chess_game->clone());
    computer_chess_game->setPeriodicFunction (buildNotifier());

    // send copy of the new game state to the chess engine thread:
    emit gameUpdated (computer_chess_game, my_game_id);

    // Notify other objects in this thread about the new game:
    emit gameStarted (my_chess_game.get());

    // Update the config to update the notifier to use the new game Id:
    updateEngineConfig();

    setCurrentTurn (ui::mapColor (my_chess_game->state()->getCurrentTurn()));
    resetStateForNewGame();
    updateDisplayedGameState();
}

void GameModel::movePiece (int src_row, int src_column, int dst_row, int dst_column)
{
    movePieceWithPromotion (src_row, src_column, dst_row, dst_column, {});
}

void GameModel::engineThreadMoved (wisdom::Move move, wisdom::Color who, int game_id)
{
    // validate this signal was not sent by an old thread:
    if (game_id != my_game_id)
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

void GameModel::promotePiece (int src_row, int src_column, int dst_row, int dst_column,
                              ui::PieceType piece_type)
{
    movePieceWithPromotion (src_row, src_column, dst_row, dst_column, mapPiece (piece_type));
}

void GameModel::movePieceWithPromotion (int srcRow, int srcColumn, int dstRow, int dstColumn,
                                        optional<wisdom::Piece> pieceType)
{
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

bool GameModel::needsPawnPromotion (int src_row, int src_column, int dst_row, int dst_column)
{
    auto [optional_move, who]
        = my_chess_game->moveFromCoordinates (src_row, src_column, dst_row,
                                                                    dst_column, Piece::Queen);
    if (!optional_move.has_value())
    {
        return false;
    }
    return optional_move->isPromoting();
}

void GameModel::applicationExiting()
{
    qDebug() << "Trying to exit application...";

    // End the thread by changing the game id:
    my_game_id = 0;
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

auto GameModel::updateChessEngineForHumanMove (Move selected_move) -> wisdom::Color
{
    auto game_state = my_chess_game->state();

    game_state->move (selected_move);
    return game_state->getCurrentTurn();
}

auto GameModel::buildNotifier() const -> MoveTimer::PeriodicFunction
{
    auto initial_game_id = my_game_id.load();
    auto initial_config_id = my_config_id.load();
    const auto* game_id_ptr = &my_game_id;
    const auto* config_id_ptr = &my_config_id;

    return (
        [game_id_ptr, initial_game_id, config_id_ptr,
         initial_config_id] (not_null<MoveTimer*> move_timer)
        {
            // This runs in the ChessEngine thread.
            auto current_config_id = config_id_ptr->load();

            // Check if config has changed:
            if (initial_config_id != current_config_id)
            {
                qDebug() << "Setting timeout to break the loop. (Config changed)";
                move_timer->setTriggered (true);

                // Discard the results of the search. The GameModel will send
                // an updateConfig signal to fire off a new search with the new
                // config.
                move_timer->setCancelled (true);
            }

            // Check if game has changed. If so, the game is over.
            if (initial_game_id != game_id_ptr->load())
            {
                qDebug() << "Setting timeout to break the loop. (Game ended)";
                move_timer->setTriggered (true);
            }
        });
}

void GameModel::updateCurrentTurn (Color new_color)
{
    setCurrentTurn (ui::mapColor (new_color));
}

void GameModel::handleMove (Player player_type, Move move, Color who)
{
    if (player_type == wisdom::Player::ChessEngine)
    {
        emit engineMoved (move, who, my_game_id);
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
    if (my_game_over_status != "")
    {
        return;
    }

    updateEngineConfig();
}

auto GameModel::gameConfig() const -> ChessGame::Config
{
    return ChessGame::Config {
        my_chess_game->state()->getPlayers(),
        MaxDepth { my_game_settings.maxDepth() },
        chrono::seconds { my_game_settings.maxSearchTime() },
    };
}

auto GameModel::currentTurn() const -> ui::Color
{
    return my_current_turn;
}

void GameModel::setCurrentTurn (ui::Color new_color)
{
    if (new_color != my_current_turn)
    {
        my_current_turn = new_color;
        emit currentTurnChanged();
    }
}

void GameModel::setGameOverStatus (const QString& new_status)
{
    if (new_status != my_game_over_status)
    {
        my_game_over_status = new_status;
        emit gameOverStatusChanged();
    }
}

auto GameModel::gameOverStatus() const -> QString
{
    return my_game_over_status;
}

void GameModel::setMoveStatus (const QString& new_status)
{
    if (new_status != my_move_status)
    {
        my_move_status = new_status;
        emit moveStatusChanged();
    }
}

auto GameModel::moveStatus() const -> QString
{
    return my_move_status;
}

void GameModel::setInCheck (bool new_in_check)
{
    if (new_in_check != my_in_check)
    {
        my_in_check = new_in_check;
        emit inCheckChanged();
    }
}

auto GameModel::inCheck() const -> bool
{
    return my_in_check;
}

class QmlGameStatusUpdate : public GameStatusUpdate
{
private:
    wisdom::observer_ptr<GameModel> my_parent;

public:
    explicit QmlGameStatusUpdate (observer_ptr<GameModel> parent) : my_parent { parent }
    {
    }

    [[nodiscard]] auto getGameState() -> not_null<wisdom::Game*>
    {
        return my_parent->my_chess_game->state();
    }

    void checkmate() override
    {
        auto who = getGameState()->getBoard().getCurrentTurn();
        auto opponent = colorInvert (who);
        auto who_string = "<b>Checkmate</b> - " + wisdom::asString (opponent) + " wins the game.";
        my_parent->setGameOverStatus (QString (who_string.c_str()));
    }

    void stalemate() override
    {
        auto who = getGameState()->getBoard().getCurrentTurn();
        auto stalemate_str
            = "<b>Stalemate</b> - No legal moves for <b>" + wisdom::asString (who) + "</b>";
        my_parent->setGameOverStatus (stalemate_str.c_str());
    }

    void insufficientMaterial() override
    {
        my_parent->setGameOverStatus ("<b>Draw</b> - Insufficient material to checkmate.");
    }

    void thirdRepetitionDrawReached() override
    {
        auto game_state = getGameState();
        if (getFirstHumanPlayerColor (game_state->getPlayers()).has_value())
            my_parent->setThirdRepetitionDrawStatus (GameModel::DrawStatus::Proposed);
    }

    void thirdRepetitionDrawAccepted() override
    {
        my_parent->setGameOverStatus ("<b>Draw</b> - Threefold repetition rule.");
    }

    void fifthRepetitionDraw() override
    {
        my_parent->setGameOverStatus ("<b>Draw</b> - Fivefold repetition rule.");
    }

    void fiftyMovesWithoutProgressReached() override
    {
        auto game_state = getGameState();
        if (getFirstHumanPlayerColor (game_state->getPlayers()).has_value())
            my_parent->setFiftyMovesDrawStatus (GameModel::DrawStatus::Proposed);
    }

    void fiftyMovesWithoutProgressAccepted() override
    {
        my_parent->setGameOverStatus ("<b>Draw</b> - Fifty moves without progress.");
    }

    void seventyFiveMovesWithNoProgress() override
    {
        my_parent->setGameOverStatus ("<b>Draw</b> - Seventy-five moves without progress.");
    }
};

void GameModel::updateDisplayedGameState()
{
    auto game_state = my_chess_game->state();
    auto& board = game_state->getBoard();
    auto who = game_state->getCurrentTurn();

    setMoveStatus ("");
    setGameOverStatus ("");
    setInCheck (false);

    QmlGameStatusUpdate status_manager { this };
    status_manager.update (game_state->status());

    if (wisdom::isKingThreatened (board, who, board.getKingPosition (who)))
        setInCheck (true);
}

void GameModel::resetStateForNewGame()
{
    setThirdRepetitionDrawStatus (DrawStatus::NotReached);
    setFiftyMovesDrawStatus (DrawStatus::NotReached);
}

auto GameModel::uiSettings() const -> const UISettings&
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

auto GameModel::gameSettings() const -> const GameSettings&
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

auto GameModel::cloneUISettings() -> UISettings
{
    return my_ui_settings;
}

auto GameModel::cloneGameSettings() -> GameSettings
{
    return my_game_settings;
}

auto GameModel::thirdRepetitionDrawStatus() const -> DrawStatus
{
    return my_third_repetition_draw_status;
}

void GameModel::setThirdRepetitionDrawStatus (DrawStatus draw_status)
{
    if (my_third_repetition_draw_status != draw_status)
    {
        my_third_repetition_draw_status = draw_status;
        emit thirdRepetitionDrawStatusChanged();
        if (draw_status == DrawStatus::Accepted || draw_status == DrawStatus::Declined)
        {
            setProposedDrawStatus (ProposedDrawType::ThreeFoldRepetition, draw_status);
        }
    }
}

auto GameModel::fiftyMovesDrawStatus() const -> DrawStatus
{
    return my_fifty_moves_draw_status;
}

void GameModel::setFiftyMovesDrawStatus (DrawStatus draw_status)
{
    if (my_fifty_moves_draw_status != draw_status)
    {
        my_fifty_moves_draw_status = draw_status;
        emit fiftyMovesDrawStatusChanged();
        if (draw_status == DrawStatus::Accepted || draw_status == DrawStatus::Declined)
        {
            setProposedDrawStatus (ProposedDrawType::FiftyMovesWithoutProgress, draw_status);
        }
    }
}

void GameModel::setProposedDrawStatus (wisdom::ProposedDrawType draw_type, DrawStatus status)
{
    auto game_state = my_chess_game->state();
    auto optional_color = getFirstHumanPlayerColor (game_state->getPlayers());

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

void GameModel::receiveChessEngineDrawStatus (wisdom::ProposedDrawType draw_type, wisdom::Color who,
                                              bool accepted)
{
    auto gameState = my_chess_game->state();
    gameState->setProposedDrawStatus (draw_type, who, accepted);
    updateDisplayedGameState();
}