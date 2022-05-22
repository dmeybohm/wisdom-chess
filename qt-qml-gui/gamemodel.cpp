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
    auto buildMoveFromCoordinates(not_null<ChessGame*> chessGame, int srcRow, int srcColumn,
                                  int dstRow, int dstColumn, optional<Piece> promoted)
        -> pair<optional<Move>, wisdom::Color>
    {
        auto game = chessGame->access();
        Coord src = make_coord(srcRow, srcColumn);
        Coord dst = make_coord(dstRow, dstColumn);

        auto who = game->get_current_turn();
        qDebug() << "Mapping coordinates for " << srcRow << ":" << srcColumn << " -> "
                 << dstRow << ":" << dstColumn;
        return { game->map_coordinates_to_move(src, dst, promoted), who };
    }

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

    auto validateIsLegalMove(not_null<ChessGame*> chessGame, Move selectedMove) -> bool
    {
        auto game = chessGame->access();
        auto selectedMoveStr = to_string(selectedMove);
        qDebug() << "Selected move: " << QString(selectedMoveStr.c_str());

        auto who = game->get_current_turn();
        auto generator = game->get_move_generator();
        auto legalMoves = generator->generate_legal_moves(game->get_board(), who);
        auto legalMovesStr = to_string(legalMoves);
        qDebug() << QString(legalMovesStr.c_str());
        for (auto legalMove : legalMoves) {
            if (legalMove == selectedMove) {
                return true;
            }
        }
        return false;
    }

    auto gameFromFen(const std::string& input) -> Game
    {
        FenParser parser { input };
        auto game = parser.build ();
        return game;
    }

    auto chessGameFromGame(unique_ptr<Game> game) -> unique_ptr<ChessGame>
    {
        return make_unique<ChessGame>(std::move(game));
    }

    void setupNotify(not_null<ChessGame*> chessGame, atomic<int>* gameId)
    {
        auto lockedGame = chessGame->access();
        auto initialGameId = gameId->load();
        lockedGame->set_periodic_function([initialGameId, gameId](not_null<MoveTimer*> moveTimer) {
            // This runs in the ChessEngine thread.
            // Check if the gameId we passed in originally has changed - if so,
            // the game is over.
            if (initialGameId != gameId->load()) {
                qDebug() << "Setting timeout to break the loop.";
                moveTimer->set_triggered(true);
            }
        });
    }
}

GameModel::GameModel(QObject *parent)
    : QObject(parent),
      myChessGame { chessGameFromGame(make_unique<Game>(Player::Human, Player::ChessEngine)) },
      myChessEngineThread { nullptr }
{
    init();
}

GameModel::~GameModel()
{
    delete myChessEngineThread;
}

void GameModel::init()
{
    auto lockedGame = myChessGame->access();
    setCurrentTurn(wisdom::chess::mapColor(lockedGame->get_current_turn()));
    lockedGame->set_white_player(Player::Human);
    lockedGame->set_black_player(Player::ChessEngine);

    setupNewEngineThread();
}

void GameModel::setupNewEngineThread()
{
    delete myChessEngineThread;

    // Initialize a new Game for the chess engine.
    // Any changes in the game config will be updated over a signal.
    auto computerGame = make_unique<Game>(Player::Human, Player::ChessEngine);
    auto computerChessGame = chessGameFromGame(std::move(computerGame));
    setupNotify(computerChessGame.get(), &myGameId);
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

    myChessEngineThread->start();
}

void GameModel::restart()
{
    if (myDelayedMoveTimer != nullptr) {
        myDelayedMoveTimer->stop();
        delete myDelayedMoveTimer;
    }
    qDebug() << "Creating new chess game";

    auto game = make_unique<Game>(Player::Human, Player::ChessEngine);
    myChessGame = std::move(chessGameFromGame(std::move(game)));
    notifyInternalGameStateUpdated();

    // let other objects in this thread know about the new game:
    emit gameStarted(myChessGame.get());
}

void GameModel::movePiece(int srcRow, int srcColumn,
                          int dstRow, int dstColumn)
{
    movePieceWithPromotion(srcRow, srcColumn, dstRow, dstColumn, {});
}

void GameModel::engineThreadMoved(wisdom::Move move, wisdom::Color who,
                                  int gameId)
{
    // validate this signal was not sent by an old thread:
    if (gameId != myGameId) {
        qDebug() << "engineThreadMoved(): Ignored signal from invalid engine.";
        return;
    }

    auto game = myChessGame->access();
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
    auto [optionalMove, who] = buildMoveFromCoordinates(myChessGame.get(), srcRow,
            srcColumn, dstRow, dstColumn, pieceType);
    if (!optionalMove.has_value()) {
        return;
    }
    auto move = *optionalMove;
    if (!validateIsLegalMove(myChessGame.get(), move)) {
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
    auto [optionalMove, who] = buildMoveFromCoordinates(myChessGame.get(), srcRow,
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

auto GameModel::updateChessEngineForHumanMove(Move selectedMove) -> wisdom::Color
{
    auto lockedGame = myChessGame->access();

    lockedGame->move(selectedMove);
    return lockedGame->get_current_turn();
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
        auto lockedGame = myChessGame->access();
        auto board = lockedGame->get_board();

        oppositePlayer = lockedGame->get_player(color_invert(who));
        myLastDelayedMoveSignal = [this, playerType, move, who](){
            delete myDelayedMoveTimer;
            myDelayedMoveTimer = nullptr;
            if (playerType == wisdom::Player::ChessEngine) {
                emit engineMoved(move, who, myGameId);
            } else {
                emit humanMoved(move, who);
            }
        };

        needProposal = lockedGame->get_history().is_third_repetition(board) && !myDrawEverProposed;
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
    myChessGame->access()->set_white_player(whitePlayer);
    myChessGame->access()->set_black_player(blackPlayer);
    notifyInternalGameStateUpdated();
}

void GameModel::notifyInternalGameStateUpdated()
{
    auto currentGame = myChessGame->access();
    auto whitePlayer = currentGame->get_player(wisdom::Color::White);
    auto blackPlayer = currentGame->get_player(wisdom::Color::Black);

    updateCurrentTurn(currentGame->get_current_turn());

    // Copy current game state to FEN and send on to the chess engine thread:
    auto fen = currentGame->get_board().to_fen_string(currentGame->get_current_turn());
    auto newGame = make_unique<Game>(gameFromFen(fen));
    newGame->set_white_player(whitePlayer);
    newGame->set_black_player(blackPlayer);

    std::shared_ptr<ChessGame> computerChessGame = std::move(chessGameFromGame(std::move(newGame)));

    myGameId++;
    setupNotify(computerChessGame.get(), &myGameId);

    // send copy of the new game state to the chsss engine thread:
    emit gameUpdated(computerChessGame, myGameId);

    updateDisplayedGameState();
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
    auto lockedGame = myChessGame->access();

    auto who = lockedGame->get_current_turn();
    auto board = lockedGame->get_board();
    auto generator = lockedGame->get_move_generator();
    setMoveStatus("");
    setGameOverStatus("");
    setInCheck(false);
    if (is_checkmated(board, who, *generator)) {
        auto whoString = "Checkmate - " + wisdom::to_string(color_invert(who)) + " wins the game.";
        setGameOverStatus(QString(whoString.c_str()));
        return;
    }

    if (is_stalemated(board, who, *generator)) {
        auto stalemateStr = "Stalemate - No legal moves for " + wisdom::to_string(who) + ". Draw";
        setGameOverStatus(stalemateStr.c_str());
        return;
    }


    if (wisdom::History::is_fifty_move_repetition(board)) {
        setGameOverStatus("Draw - Fifty moves without a capture or pawn move.");
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

    // If the proposal was accepted, update the status.
    if (accepted) {
        setGameOverStatus("Draw proposed and accepted after third repetition rule.");
        return;
    }

    // If the proposal was rejected, emit the appropriate signal stored in the lambda.
    assert(myLastDelayedMoveSignal.has_value());
    auto emitSignal = *myLastDelayedMoveSignal;
    myLastDelayedMoveSignal.reset();
    emitSignal();
}
