#include "gamemodel.hpp"

#include "game.hpp"
#include "generate.hpp"
#include "fen_parser.hpp"
#include "check.hpp"

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
    auto pieceFromString(const QString& piece) -> wisdom::Piece
    {
        if (piece == "queen") {
            return wisdom::Piece::Queen;
        } else if (piece == "king") {
            return wisdom::Piece::King;
        } else if (piece == "pawn") {
            return wisdom::Piece::Pawn;
        } else if (piece == "knight") {
            return wisdom::Piece::Knight;
        } else if (piece == "bishop") {
            return wisdom::Piece::Bishop;
        } else if (piece == "rook") {
            return wisdom::Piece::Rook;
        } else {
            assert(0); abort();
        }
    }
}

GameModel::GameModel(QObject *parent)
    : QObject(parent)
    , myMaxDepth { 4 }
    , myMaxSearchTime { 5 }
    , myChessEngineThread { nullptr }
{
    myChessGame = ChessGame::fromPlayers(Player::Human, Player::ChessEngine, gameConfig());
    init();
}

GameModel::~GameModel()
{
    delete myChessEngineThread;
}

void GameModel::init()
{
    auto gameState = myChessGame->state();
    setCurrentTurn(wisdom::chess::mapColor(gameState->get_current_turn()));
    myChessGame->setPlayers(Player::Human, Player::ChessEngine);

    setupNewEngineThread();
}

void GameModel::setupNewEngineThread()
{
    delete myChessEngineThread;

    // Initialize a new Game for the chess engine.
    // Any changes in the game config will be updated over a signal.
    auto computerChessGame = myChessGame->clone();
    computerChessGame->setupNotify(&myGameId);
    auto chessEngine = new ChessEngine { std::move(computerChessGame), myGameId };

    myChessEngineThread = new QThread();

    // Connect event handlers for the computer and human making moves:
    connect(this, &GameModel::humanMoved,
            chessEngine, &ChessEngine::opponentMoved);
    connect(chessEngine, &ChessEngine::engineMoved,
            this, &GameModel::engineThreadMoved);

    // Connect event handlers for draw proposition to the engine:
    connect(this, &GameModel::proposeDrawToEngine,
            chessEngine, &ChessEngine::drawProposed);
    connect(chessEngine, &ChessEngine::drawProposalResponse,
            this, &GameModel::drawProposalResponse);

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
            myChessEngineThread, &QThread::quit);

    // Update the engine's config when user changes its:
    connect(this, &GameModel::maxDepthChanged,
            this, &GameModel::updateEngineConfig);
    connect(this, &GameModel::maxSearchTimeChanged,
            this, &GameModel::updateEngineConfig);
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
    myDrawEverProposed = false;
    setGameOverStatus("");

    if (myDelayedMoveTimer != nullptr) {
        myDelayedMoveTimer->stop();
        delete myDelayedMoveTimer;
    }
    qDebug() << "Creating new chess game";

    myChessGame = std::move(ChessGame::fromPlayers(
        myChessGame->state()->get_player(Color::White),
        myChessGame->state()->get_player(Color::Black),
        gameConfig()
    ));
    notifyInternalGameStateUpdated();

    emit gameStarted(myChessGame.get());
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
    checkForDrawAndEmitPlayerMoved(Player::ChessEngine, move, who);
}

void GameModel::promotePiece(int srcRow, int srcColumn,
                             int dstRow, int dstColumn,
                             QString pieceString)
{
    optional<Piece> pieceType = pieceFromString(pieceString);
    movePieceWithPromotion(srcRow, srcColumn, dstRow, dstColumn, pieceType);
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
    checkForDrawAndEmitPlayerMoved(wisdom::Player::Human, move, who);
}

bool GameModel::needsPawnPromotion(int srcRow, int srcColumn, int dstRow, int dstColumn)
{
    auto [optionalMove, who] = myChessGame->moveFromCoordinates(srcRow,
            srcColumn, dstRow, dstColumn, Piece::Queen);
    if (!optionalMove.has_value()) {
        return false;
    }
    return is_promoting_move(*optionalMove);
}

void GameModel::applicationExiting()
{
    qDebug() << "Trying to exit application...";

    // End the thread by changing the game id:
    myGameId = 0;
    emit terminationStarted();

    myChessEngineThread->wait();
}

void GameModel::updateEngineConfig()
{
    emit engineConfigChanged(ChessGame::Config { MaxDepth { myMaxDepth }, std::chrono::seconds { myMaxSearchTime }});
}

auto GameModel::updateChessEngineForHumanMove(Move selectedMove) -> wisdom::Color
{
    auto gameState = myChessGame->state();

    gameState->move(selectedMove);
    return gameState->get_current_turn();
}

void GameModel::updateCurrentTurn(Color newColor)
{
    setCurrentTurn(wisdom::chess::mapColor(newColor));
}

void GameModel::checkForDrawAndEmitPlayerMoved(Player playerType, Move move, Color who)
{
    bool needProposal;
    wisdom::Player oppositePlayer;
    {
        assert(!myLastDelayedMoveSignal.has_value());
        auto gameState = myChessGame->state();
        auto board = gameState->get_board();

        oppositePlayer = gameState->get_player(color_invert(who));
        myLastDelayedMoveSignal = [this, playerType, move, who](){
            delete myDelayedMoveTimer;
            myDelayedMoveTimer = nullptr;
            if (playerType == wisdom::Player::ChessEngine) {
                emit engineMoved(move, who, myGameId);
            } else {
                emit humanMoved(move, who);
            }
        };

        needProposal = gameState->get_history().is_third_repetition(board) && !myDrawEverProposed;
    }

    if (needProposal) {
        proposeDraw(oppositePlayer);
    } else {
        auto emitSignal = *myLastDelayedMoveSignal;
        myLastDelayedMoveSignal.reset();
        QTimer::singleShot(50, this, emitSignal);
    }
}

void GameModel::updateInternalGameState()
{
    auto whitePlayer = myWhiteIsComputer ? wisdom::Player::ChessEngine : wisdom::Player::Human;
    auto blackPlayer = myBlackIsComputer ? wisdom::Player::ChessEngine : wisdom::Player::Human;
    myChessGame->setPlayers(whitePlayer, blackPlayer);
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

    std::shared_ptr<ChessGame> computerChessGame = std::move(myChessGame->clone());

    myGameId++;
    computerChessGame->setupNotify(&myGameId);

    // Update the engine config if needed:
    emit updateEngineConfig();

    // send copy of the new game state to the chess engine thread:
    emit gameUpdated(computerChessGame, myGameId);

    updateDisplayedGameState();
}

ChessGame::Config GameModel::gameConfig() const
{
    return ChessGame::Config {
        MaxDepth { myMaxDepth },
        chrono::seconds { myMaxSearchTime }
    };
}

auto GameModel::currentTurn() -> wisdom::chess::ChessColor
{
    return myCurrentTurn;
}

void GameModel::setCurrentTurn(wisdom::chess::ChessColor newColor)
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

auto GameModel::gameOverStatus() -> QString
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

auto GameModel::moveStatus() -> QString
{
    return myMoveStatus;
}

void GameModel::setInCheck(const bool newInCheck)
{
    if (newInCheck != myInCheck) {
        myInCheck = newInCheck;
        emit inCheckChanged();
    }
}

auto GameModel::inCheck() -> bool
{
    return myInCheck;
}

void GameModel::updateDisplayedGameState()
{
    auto gameState = myChessGame->state();

    auto who = gameState->get_current_turn();
    auto& board = gameState->get_board();
    auto generator = gameState->get_move_generator();
    setMoveStatus("");
    setGameOverStatus("");
    setInCheck(false);
    if (is_checkmated(board, who, *generator)) {
        auto whoString = "<b>Checkmate</b> - " + wisdom::to_string(color_invert(who)) + " wins the game.";
        setGameOverStatus(QString(whoString.c_str()));
        return;
    }

    if (is_stalemated(board, who, *generator)) {
        auto stalemateStr = "<b>Stalemate</b> - No legal moves for " + wisdom::to_string(who) + ". Draw";
        setGameOverStatus(stalemateStr.c_str());
        return;
    }


    if (gameState->get_history().is_fifth_repetition(gameState->get_board())) {
        setGameOverStatus("<b>Draw</b> - Fifth move repetition.");
        return;
    }

    if (wisdom::History::is_fifty_move_repetition(board)) {
        setGameOverStatus("<b>Draw</b> - Fifty moves without a capture or pawn move.");
        return;
    }

    if (wisdom::is_king_threatened(board, who, board.get_king_position(who))) {
        setInCheck(true);
    }
}

auto GameModel::drawProposedToHuman() -> bool
{
    return myDrawProposedToHuman;
}

auto GameModel::whiteIsComputer() -> bool
{
    return myWhiteIsComputer;
}

void GameModel::setWhiteIsComputer(bool newWhiteIsComputer)
{
    if (myWhiteIsComputer != newWhiteIsComputer) {
        myWhiteIsComputer = newWhiteIsComputer;
        updateInternalGameState();
        emit whiteIsComputerChanged();
    }
}

auto GameModel::blackIsComputer() -> bool
{
    return myBlackIsComputer;
}

void GameModel::setMaxDepth(int maxDepth)
{
    if (myMaxDepth != maxDepth) {
        myMaxDepth = maxDepth;
        myChessGame->setConfig(gameConfig());
        emit maxDepthChanged();
    }
}

int GameModel::maxDepth()
{
    return myMaxDepth;
}

void GameModel::setMaxSearchTime(int maxSearchTime)
{
    if (maxSearchTime != myMaxSearchTime) {
        myMaxSearchTime = maxSearchTime;
        myChessGame->setConfig(gameConfig());
        emit maxSearchTimeChanged();
    }
}

int GameModel::maxSearchTime()
{
    return myMaxSearchTime;
}

void GameModel::setBlackIsComputer(bool newBlackIsComputer)
{
    if (myBlackIsComputer != newBlackIsComputer) {
        myBlackIsComputer = newBlackIsComputer;
        updateInternalGameState();
        emit blackIsComputerChanged();
    }
}

void GameModel::setDrawProposedToHuman(bool drawProposed)
{
    qDebug() << "setDrawProposedToHuman";
    if (myDrawProposedToHuman != drawProposed) {
        myDrawProposedToHuman = drawProposed;
        emit drawProposedToHumanChanged();
    }
}

void GameModel::proposeDraw(wisdom::Player player)
{
    if (player == Player::Human) {
        setDrawProposedToHuman(true);
    } else {
        emit proposeDrawToEngine();
    }
    myDrawEverProposed = true;
}

void GameModel::drawProposalResponse(bool accepted)
{
    setDrawProposedToHuman(false);
    assert(myLastDelayedMoveSignal.has_value());
    auto emitSignal = *myLastDelayedMoveSignal;
    myLastDelayedMoveSignal.reset();
    emitSignal();

    // If the proposal was accepted, update the status.
    if (accepted) {
        setGameOverStatus("<b>Draw</b> - Third repetition rule.");
        return;
    }
}
