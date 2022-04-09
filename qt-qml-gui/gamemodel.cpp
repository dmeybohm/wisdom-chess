#include "gamemodel.h"

#include "game.hpp"
#include "generate.hpp"
#include "fen_parser.hpp"
#include "check.hpp"

#include <QDebug>

using namespace wisdom;
using namespace std;

namespace
{
    auto buildMoveFromCoordinates(gsl::not_null<ChessGame*> chessGame, int srcRow, int srcColumn,
                                  int dstRow, int dstColumn, optional<Piece> promoted) -> pair<optional<Move>, wisdom::Color>
    {
        auto game = chessGame->access();
        Coord src = make_coord(srcRow, srcColumn);
        Coord dst = make_coord(dstRow, dstColumn);

        auto who = game->get_current_turn();
        qDebug() << "Mapping coordinates for " << srcRow << ":" << srcColumn << " -> "
                 << dstRow << ":" << dstColumn;
        return { game->map_coordinates_to_move(src, dst, promoted), who };
    }

    auto pieceFromString(QString piece) -> wisdom::Piece
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

    auto validateIsLegalMove(gsl::not_null<ChessGame*> chessGame, Move selectedMove) -> bool
    {
        auto game = chessGame->access();
        auto selectedMoveStr = to_string(selectedMove);
        qDebug() << "Selected move: " << QString(selectedMoveStr.c_str());

        auto who = game->get_current_turn();
        auto legalMoves = generate_legal_moves(game->get_board(), who);
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

    void setupNotify(gsl::not_null<ChessGame*> chessGame)
    {
        auto lockedGame = chessGame->access();
        lockedGame->set_periodic_function([](gsl::not_null<MoveTimer*> moveTimer) {
            // This runs in the ChessEngine thread, and so has the game mutex.
            auto* currentThread = QThread::currentThread();

            if (currentThread->isInterruptionRequested()) {
                qDebug() << "Setting timeout to break the loop.";
                moveTimer->set_triggered(true);
            }
        });
    }
}

GameModel::GameModel(QObject *parent)
    : QObject(parent),
      myChessGame { chessGameFromGame(make_unique<Game>(Player::Human, Player::Human)) },
//      myChessGame { chessGameFromGame(make_unique<Game>(Player::Human, Player::ChessEngine)) },
//      myChessGame { chessGameFromGame(
//                        make_unique<Game>(gameFromFen("8/PPPPPPPP/2N2N2/8/8/8/1k4K1/8 w - - 0 1"))
//                    )},
      myChessEngineThread { nullptr }
{
    // Initialize the piece list from the game->board.
    init();
    setupNotify(myChessGame.get());
    setupNewEngineThread();
}

void GameModel::init()
{
    auto lockedGame = myChessGame->access();
    setCurrentTurn(mapColor(lockedGame->get_current_turn()));
}

void GameModel::setupNewEngineThread()
{
    delete myChessEngineThread;

    auto chessEngine = new ChessEngine { myChessGame };
    myChessEngineThread = new QThread();

    // Connect event handlers for the computer and human making moves:
    connect(this, &GameModel::humanMoved, chessEngine, &ChessEngine::opponentMoved);
    connect(chessEngine, &ChessEngine::engineMoved, this, &GameModel::engineThreadMoved);

    // Initialize the engine when the engine thread starts:
    connect(myChessEngineThread, &QThread::started, chessEngine, &ChessEngine::init);

    // If the engine finds no moves available, check whether the game is over.
    connect(chessEngine, &ChessEngine::noMovesAvailable, this, &GameModel::updateGameStatus);

    // Connect the engine's move back to itself in case it's playing itself:
    // (it will return early if it's not)
    connect(this, &GameModel::engineMoved, chessEngine, &ChessEngine::opponentMoved);

    // exit event loop from engine thread when we start exiting:
    connect(this, &GameModel::terminationStarted, myChessEngineThread, &QThread::quit);

    // Cleanup chess engine when chess engine thread exits:
    connect(myChessEngineThread, &QThread::finished, chessEngine, &QObject::deleteLater);

    // Move the ownership of the engine to the engine thread so slots run on that thread:
    chessEngine->moveToThread(myChessEngineThread);
}

void GameModel::start()
{
    // todo cleanup old thread and re-connect here:
    emit gameStarted(myChessGame.get());

    myChessEngineThread->start();
}

void GameModel::movePiece(int srcRow, int srcColumn,
                          int dstRow, int dstColumn)
{
    movePieceWithPromotion(srcRow, srcColumn, dstRow, dstColumn, {});
}

void GameModel::engineThreadMoved(wisdom::Move move, wisdom::Color who)
{
    updateGameStatus();
    updateCurrentTurn(wisdom::color_invert(who));

    // re-emit single-threaded signal to listeners:
    emit engineMoved(move, who);
}

void GameModel::promotePiece(int srcRow, int srcColumn, int dstRow, int dstColumn, QString pieceString)
{
    optional<Piece> pieceType = pieceFromString(pieceString);
    movePieceWithPromotion(srcRow, srcColumn, dstRow, dstColumn, pieceType);
}

void GameModel::movePieceWithPromotion(int srcRow, int srcColumn,
                                       int dstRow, int dstColumn, std::optional<wisdom::Piece> pieceType)
{
    auto [optionalMove, who] = buildMoveFromCoordinates(myChessGame.get(), srcRow,
            srcColumn, dstRow, dstColumn, pieceType);
    if (!optionalMove.has_value()) {
        return;
    }
    auto move = *optionalMove;
    if (!validateIsLegalMove(myChessGame.get(), move)) {
        return;
    }
    auto newColor = updateChessEngineForHumanMove(move);
    updateGameStatus();
    updateCurrentTurn(newColor);
    emit humanMoved(move, who);
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

    myChessEngineThread->requestInterruption();
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
    setCurrentTurn(mapColor(newColor));
}

auto GameModel::currentTurn() -> ColorEnumValue
{
    return myCurrentTurn;
}

void GameModel::setCurrentTurn(ColorEnumValue newColor)
{
    if (newColor != myCurrentTurn) {
        qDebug() << "Updating color to " << toString(newColor);
        myCurrentTurn = newColor;
        qDebug() << "Emitting currentTurnChanged()";
        emit currentTurnChanged();
    } else {
        qDebug() << "Keeping color as " << toString(newColor);
    }
}

void GameModel::setGameStatus(const QString& newStatus)
{
    qDebug() << "Old Status: " << myGameStatus;
    qDebug() << "New status: " << newStatus;
    if (newStatus != myGameStatus) {
        myGameStatus = newStatus;
        emit gameStatusChanged();
    }
}

auto GameModel::gameStatus() -> QString
{
    return myGameStatus;
}

void GameModel::updateGameStatus()
{
    auto lockedGame = myChessGame->access();

    auto who = lockedGame->get_current_turn();
    auto board = lockedGame->get_board();
    if (is_checkmated(board, who)) {
        auto whoString = wisdom::to_string(color_invert(who)) + " wins the game.";
        setGameStatus(QString(whoString.c_str()));
        return;
    }

    if (is_stalemated_slow(board, who)) {
        setGameStatus("Draw. Stalemate.");
        return;
    }

    //        if (myGame.get_history().is_third_repetition(myGame.get_board())) {
    //            input_state = offer_draw();
    //            continue;
    //        }

    if (wisdom::History::is_fifty_move_repetition(board)) {
        setGameStatus("Draw. Fifty moves without a capture or pawn move.");
        return;
    }
}
