#include "gamemodel.hpp"

#include "game.hpp"
#include "check.hpp"
#include "chessengine.hpp"
#include "ui_settings.hpp"

#include <QDebug>
#include <QTimer>
#include <chrono>

using namespace wisdom;
using std::optional;
using gsl::not_null;
using std::make_shared;
using std::shared_ptr;
using std::pair;
using std::atomic;

namespace
{
    auto getFirstHumanPlayerColor(Players players) -> optional<Color>
    {
        if (players[0] == Player::Human) {
            return Color::White;
        }
        if (players[1] == Player::Human) {
            return Color::Black;
        }

        return {};
    }
}

GameModel::GameModel(QObject *parent)
    : QObject(parent)
    , myCurrentTurn {}
    , myChessEngineThread { nullptr }
{
    myChessGame = ChessGame::fromPlayers(
                Player::Human,
                Player::ChessEngine,
                ChessGame::Config::fromGameSettings (myGameSettings)
    );
    init ();
}

GameModel::~GameModel()
{
    delete myChessEngineThread;
}

void GameModel::init()
{
    auto gameState = myChessGame->state();
    setCurrentTurn(wisdom::ui::mapColor(gameState->get_current_turn()));

    setupNewEngineThread();
}

void GameModel::setupNewEngineThread()
{
    delete myChessEngineThread;

    // Initialize a new Game for the chess engine.
    // Any changes in the game config will be updated over a signal.
    auto computerChessGame = myChessGame->clone();
    computerChessGame->setPeriodicFunction(buildNotifier());

    auto chessEngine = new ChessEngine { std::move(computerChessGame), myGameId };

    myChessEngineThread = new QThread();

    // Connect event handlers for the computer and human making moves:
    connect(this, &GameModel::humanMoved,
            chessEngine, &ChessEngine::opponentMoved);
    connect(chessEngine, &ChessEngine::engineMoved,
            this, &GameModel::engineThreadMoved);

    // Update draw status between engine and game model:
    connect(chessEngine, &ChessEngine::updateDrawStatus,
            this, &GameModel::receiveChessEngineDrawStatus);
    connect(this, &GameModel::updateDrawStatus,
            chessEngine, &ChessEngine::receiveDrawStatus);

    // Initialize the engine when the engine thread starts:
    connect(myChessEngineThread, &QThread::started,
            chessEngine, &ChessEngine::init);

    // If the engine finds no moves available, check whether the game is over.
    connect(chessEngine, &ChessEngine::noMovesAvailable,
            this, &GameModel::updateDisplayedGameState);

    // Connect the engine's move back to itself in case it's playing itself:
    // (it will return early if it's not)
    connect(this, &GameModel::engineMoved,
            chessEngine, &ChessEngine::receiveEngineMoved);

    // exit event loop from engine thread when we start exiting:
    connect(this, &GameModel::terminationStarted,
            chessEngine, &ChessEngine::quit);

    // Update the engine's config when user changes it:
    connect(this, &GameModel::engineConfigChanged,
            chessEngine, &ChessEngine::updateConfig);

    // Cleanup chess engine when chess engine thread exits:
    connect(myChessEngineThread, &QThread::finished,
            chessEngine, &QObject::deleteLater);

    // When creating a new game, send a copy of the new ChessGame to the Engine:
    connect(this, &GameModel::gameUpdated,
            chessEngine, &ChessEngine::reloadGame);

    // Move the ownership of the engine to the engine thread so slots run on that thread:
    chessEngine->moveToThread(myChessEngineThread);
}

void GameModel::start()
{
    emit gameStarted(myChessGame.get());

    updateEngineConfig();
    myChessEngineThread->start();
}

void GameModel::restart()
{
    // let other objects in this thread know about the new game:
    setGameOverStatus("");

    qDebug() << "Creating new chess game";

    myChessGame = std::move(ChessGame::fromPlayers(
        myChessGame->state()->get_player(Color::White),
        myChessGame->state()->get_player(Color::Black),
        gameConfig()
    ));

    // Abort searches and discard any queued signals from them if we receive
    // them later.
    myGameId++;

    std::shared_ptr<ChessGame> computerChessGame = std::move(myChessGame->clone());
    computerChessGame->setPeriodicFunction(buildNotifier());

    // send copy of the new game state to the chess engine thread:
    emit gameUpdated(computerChessGame, myGameId);

    // Notify other objects in this thread about the new game:
    emit gameStarted(myChessGame.get());

    // Update the config to update the notifier to use the new game Id:
    updateEngineConfig();

    setCurrentTurn(wisdom::ui::mapColor(myChessGame->state()->get_current_turn()));
    resetStateForNewGame();
    updateDisplayedGameState();
}

void GameModel::movePiece(int srcRow, int srcColumn,
                          int dstRow, int dstColumn)
{
    movePieceWithPromotion(srcRow, srcColumn, dstRow, dstColumn, {});
}

void GameModel::engineThreadMoved(wisdom::Move move, wisdom::Color who, int gameId)
{
    // validate this signal was not sent by an old thread:
    if (gameId != myGameId) {
        qDebug() << "engineThreadMoved(): Ignored signal from invalid engine.";
        return;
    }

    auto game = myChessGame->state();
    game->move(move);

    updateDisplayedGameState();
    updateCurrentTurn(wisdom::color_invert(who));

    // re-emit single-threaded signal to listeners:
    handleMove(Player::ChessEngine, move, who);
}

void GameModel::promotePiece(int srcRow, int srcColumn,
                             int dstRow, int dstColumn,
                             wisdom::ui::PieceType pieceType)
{
    movePieceWithPromotion(srcRow, srcColumn, dstRow, dstColumn, mapPiece(pieceType));
}

void GameModel::movePieceWithPromotion(int srcRow, int srcColumn,
                                       int dstRow, int dstColumn,
                                       optional<wisdom::Piece> pieceType)
{
    auto [optionalMove, who] = myChessGame->moveFromCoordinates(srcRow, srcColumn,
                                                                dstRow, dstColumn, pieceType);
    if (!optionalMove.has_value()) {
        return;
    }
    auto move = *optionalMove;
    if (!myChessGame->isLegalMove(move)) {
        setMoveStatus("Illegal move");
        return;
    }
    auto newColor = updateChessEngineForHumanMove(move);
    updateDisplayedGameState();
    updateCurrentTurn(newColor);
    handleMove(wisdom::Player::Human, move, who);
}

bool GameModel::needsPawnPromotion(int srcRow, int srcColumn, int dstRow, int dstColumn)
{
    auto [optionalMove, who] = myChessGame->moveFromCoordinates(srcRow,
            srcColumn, dstRow, dstColumn, Piece::Queen);
    if (!optionalMove.has_value()) {
        return false;
    }
    return optionalMove->is_promoting();
}

void GameModel::applicationExiting()
{
    qDebug() << "Trying to exit application...";

    // End the thread by changing the game id:
    myGameId = 0;
    qDebug() << "Terminated start...";
    emit terminationStarted();

    // For desktop, we need to wait here before exiting. But on web that will hang.
#ifndef EMSCRIPTEN
    myChessEngineThread->wait();
#endif

    qDebug() << "Termination ended.";
}

void GameModel::updateEngineConfig()
{
    myConfigId++;

    emit engineConfigChanged(gameConfig(), buildNotifier());
}

auto GameModel::updateChessEngineForHumanMove(Move selectedMove) -> wisdom::Color
{
    auto gameState = myChessGame->state();

    gameState->move(selectedMove);
    return gameState->get_current_turn();
}

auto GameModel::buildNotifier() const -> MoveTimer::PeriodicFunction
{
    auto initialGameId = myGameId.load();
    auto initialConfigId = myConfigId.load();
    const auto* gameIdPtr = &myGameId;
    const auto* configIdPtr = &myConfigId;

    return ([gameIdPtr, initialGameId, configIdPtr, initialConfigId](not_null<MoveTimer*> moveTimer) {
         // This runs in the ChessEngine thread.
        auto currentConfigId = configIdPtr->load();

         // Check if config has changed:
         if (initialConfigId != currentConfigId) {
             qDebug() << "Setting timeout to break the loop. (Config changed)";
             moveTimer->set_triggered(true);

             // Discard the results of the search. The GameModel will send
             // an updateConfig signal to fire off a new search with the new
             // config.
             moveTimer->set_cancelled(true);
         }

         // Check if game has changed. If so, the game is over.
         if (initialGameId != gameIdPtr->load()) {
             qDebug() << "Setting timeout to break the loop. (Game ended)";
             moveTimer->set_triggered(true);
         }
    });
}

void GameModel::updateCurrentTurn(Color newColor)
{
    setCurrentTurn(wisdom::ui::mapColor(newColor));
}

void GameModel::handleMove(Player playerType, Move move, Color who)
{
    if (playerType == wisdom::Player::ChessEngine) {
        emit engineMoved(move, who, myGameId);
    } else {
        emit humanMoved(move, who);
    }
}

void GameModel::updateInternalGameState()
{
    myChessGame->setPlayers(
        mapPlayer(myGameSettings.whitePlayer()), mapPlayer(myGameSettings.blackPlayer())
    );
    myChessGame->setConfig(gameConfig());
    notifyInternalGameStateUpdated();
}

void GameModel::notifyInternalGameStateUpdated()
{
    auto gameState = myChessGame->state();

    updateCurrentTurn(gameState->get_current_turn());
    if (myGameOverStatus != "") {
        return;
    }

    updateEngineConfig();
}

auto GameModel::gameConfig() const -> ChessGame::Config
{
    return ChessGame::Config {
        myChessGame->state()->get_players(),
        MaxDepth { myGameSettings.maxDepth() },
        chrono::seconds { myGameSettings.maxSearchTime() },
    };
}

auto GameModel::currentTurn() const -> wisdom::ui::Color
{
    return myCurrentTurn;
}

void GameModel::setCurrentTurn(wisdom::ui::Color newColor)
{
    if (newColor != myCurrentTurn) {
        myCurrentTurn = newColor;
        emit currentTurnChanged();
    }
}

void GameModel::setGameOverStatus(const QString& newStatus)
{
    if (newStatus != myGameOverStatus) {
        myGameOverStatus = newStatus;
        emit gameOverStatusChanged();
    }
}

auto GameModel::gameOverStatus() const -> QString
{
    return myGameOverStatus;
}

void GameModel::setMoveStatus(const QString &newStatus)
{
    if (newStatus != myMoveStatus) {
        myMoveStatus = newStatus;
        emit moveStatusChanged();
    }
}

auto GameModel::moveStatus() const -> QString
{
    return myMoveStatus;
}

void GameModel::setInCheck(bool newInCheck)
{
    if (newInCheck != myInCheck) {
        myInCheck = newInCheck;
        emit inCheckChanged();
    }
}

auto GameModel::inCheck() const -> bool
{
    return myInCheck;
}

void GameModel::updateDisplayedGameState()
{
    auto gameState = myChessGame->state();

    auto who = gameState->get_current_turn();
    auto& board = gameState->get_board();

    setMoveStatus("");
    setGameOverStatus("");
    setInCheck(false);

    auto nextStatus = gameState->status();

    switch (nextStatus)
    {
    case GameStatus::Playing:
        break;

    case GameStatus::Checkmate: {
        auto whoString = "<b>Checkmate</b> - " + wisdom::to_string(color_invert(who)) + " wins the game.";
        setGameOverStatus(QString(whoString.c_str()));
        return;
    }

    case GameStatus::Stalemate: {
        auto stalemateStr = "<b>Stalemate</b> - No legal moves for <b>" + wisdom::to_string(who) + "</b>";
        setGameOverStatus(stalemateStr.c_str());
        return;
    }

    case GameStatus::FivefoldRepetitionDraw:
        setGameOverStatus("<b>Draw</b> - Fivefold repetition rule.");
        return;

    case GameStatus::ThreefoldRepetitionReached:
        if (getFirstHumanPlayerColor(gameState->get_players()).has_value()) {
            setThirdRepetitionDrawProposed(true);
        }
        break;

    case GameStatus::FiftyMovesWithoutProgressReached:
        if (getFirstHumanPlayerColor(gameState->get_players()).has_value()) {
            setFiftyMovesWithoutProgressDrawProposed(true);
        }
        break;

    case GameStatus::ThreefoldRepetitionAccepted:
        setGameOverStatus("<b>Draw</b> - Threefold repetition rule.");
        return;

    case GameStatus::FiftyMovesWithoutProgressAccepted:
        setGameOverStatus("<b>Draw</b> - Fifty moves without progress.");
        return;

    case GameStatus::SeventyFiveMovesWithoutProgressDraw:
        setGameOverStatus("<b>Draw</b> - Seventy-five moves without progress.");
        return;

    case GameStatus::InsufficientMaterialDraw:
        setGameOverStatus("<b>Draw</b> - Insufficient material to checkmate.");
        return;
    }

    if (wisdom::is_king_threatened(board, who, board.get_king_position(who))) {
        setInCheck(true);
    }
}

void GameModel::resetStateForNewGame()
{
    setThirdRepetitionDrawAnswered(false);
    setThirdRepetitionDrawProposed(false);
    setFiftyMovesWithoutProgressDrawAnswered(false);
    setFiftyMovesWithoutProgressDrawProposed(false);
}

auto GameModel::uiSettings() const -> const UISettings&
{
    return myUISettings;
}

void GameModel::setUISettings(const UISettings& settings)
{
    if (myUISettings != settings) {
        myUISettings = settings;
        emit uiSettingsChanged();
    }
}

auto GameModel::gameSettings() const -> const GameSettings&
{
    return myGameSettings;
}

void GameModel::setGameSettings(const GameSettings &newGameSettings)
{
    if (myGameSettings != newGameSettings) {
        myGameSettings = newGameSettings;
        emit gameSettingsChanged();
        updateInternalGameState();
    }
}

auto GameModel::cloneUISettings() -> UISettings
{
    return myUISettings;
}

auto GameModel::cloneGameSettings() -> GameSettings
{
    return myGameSettings;
}

auto GameModel::thirdRepetitionDrawProposed() const -> bool
{
    return myThirdRepetitionDrawProposed;
}

void GameModel::setThirdRepetitionDrawProposed(bool drawProposed)
{
    qDebug() << "setThirdRepetitionDrawProposed";
    if (myThirdRepetitionDrawProposed != drawProposed) {
        myThirdRepetitionDrawProposed = drawProposed;
        emit thirdRepetitionDrawProposedChanged();
    }
}

auto GameModel::fiftyMovesWithoutProgressDrawProposed() const -> bool
{
    return myThirdRepetitionDrawProposed;
}

void GameModel::setFiftyMovesWithoutProgressDrawProposed(bool drawProposed)
{
    if (myFiftyMovesWithProgressDrawProposed != drawProposed) {
        myFiftyMovesWithProgressDrawProposed = drawProposed;
        emit fiftyMovesWithoutProgressDrawProposedChanged();
    }
}

auto GameModel::thirdRepetitionDrawAnswered() const -> bool
{
    return myThirdRepetitionDrawAnswered;
}

void GameModel::setThirdRepetitionDrawAnswered(bool answered)
{
    if (myThirdRepetitionDrawAnswered != answered) {
        myThirdRepetitionDrawAnswered = answered;
        emit thirdRepetitionDrawAnsweredChanged();
    }
}

auto GameModel::fiftyMovesWithoutProgressDrawAnswered() const -> bool
{
    return myThirdRepetitionDrawAnswered;
}

void GameModel::setFiftyMovesWithoutProgressDrawAnswered(bool answered)
{
    if (myFiftyMovesWithProgressDrawAnswered != answered) {
        myFiftyMovesWithProgressDrawAnswered = answered;
        emit fiftyMovesWithoutProgressDrawAnsweredChanged();
    }
}

void GameModel::proposeDraw(wisdom::Player player, wisdom::ProposedDrawType drawType)
{
    if (player == Player::Human) {
        switch (drawType) {
        case wisdom::ProposedDrawType::FiftyMovesWithoutProgress:
            setFiftyMovesWithoutProgressDrawProposed(true);
        case wisdom::ProposedDrawType::ThreeFoldRepetition:
            setThirdRepetitionDrawProposed(true);
        }
    }
}

void GameModel::humanWantsThreefoldRepetitionDraw(bool accepted)
{
    setProposedDrawTypeAcceptance(ProposedDrawType::ThreeFoldRepetition, accepted);
    setThirdRepetitionDrawAnswered(true);
}

void GameModel::humanWantsFiftyMovesWithoutProgressDraw(bool accepted)
{
    setProposedDrawTypeAcceptance(ProposedDrawType::FiftyMovesWithoutProgress, accepted);
    setFiftyMovesWithoutProgressDrawAnswered(true);
}

void GameModel::setProposedDrawTypeAcceptance(wisdom::ProposedDrawType drawType,
                                              bool accepted)
{
    auto gameState = myChessGame->state();
    auto optionalColor = getFirstHumanPlayerColor(gameState->get_players());

    assert(optionalColor.has_value());
    auto who = *optionalColor;
    auto opponentColor = color_invert(who);

    gameState->set_proposed_draw_status(drawType, who, accepted);
    if (gameState->get_player(color_invert(who)) == Player::Human) {
        gameState->set_proposed_draw_status(drawType, opponentColor, accepted);
    }
    emit updateDrawStatus(drawType, who, accepted);

    updateDisplayedGameState();
}

void GameModel::receiveChessEngineDrawStatus(wisdom::ProposedDrawType drawType,
                                             wisdom::Color who, bool accepted)
{
    auto gameState = myChessGame->state();
    gameState->set_proposed_draw_status(drawType, who, accepted);
    updateDisplayedGameState();
}

