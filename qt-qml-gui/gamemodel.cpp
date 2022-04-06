#include "gamemodel.h"

#include "game.hpp"
#include "generate.hpp"
#include "fen_parser.hpp"

#include <QDebug>

using namespace wisdom;
using namespace std;

namespace
{
    constexpr auto whitePiece(Piece piece) -> int8_t
    {
        return to_int8(make_piece(Color::White, piece));
    }

    constexpr auto blackPiece(Piece piece) -> int8_t
    {
        return to_int8(make_piece(Color::Black, piece));
    }

    auto initPieceMap(QObject *parent) -> QHash<int8_t, QString>
    {
        auto result = QHash<int8_t, QString> {
            { whitePiece(Piece::Pawn), "images/Chess_plt45.svg" },
            { whitePiece(Piece::Rook), "images/Chess_rlt45.svg" },
            { whitePiece(Piece::Knight), "images/Chess_nlt45.svg" },
            { whitePiece(Piece::Bishop), "images/Chess_blt45.svg" },
            { whitePiece(Piece::Queen), "images/Chess_qlt45.svg" },
            { whitePiece(Piece::King), "images/Chess_klt45.svg" },
            { blackPiece(Piece::Pawn), "images/Chess_pdt45.svg" },
            { blackPiece(Piece::Rook), "images/Chess_rdt45.svg" },
            { blackPiece(Piece::Knight), "images/Chess_ndt45.svg" },
            { blackPiece(Piece::Bishop), "images/Chess_bdt45.svg" },
            { blackPiece(Piece::Queen), "images/Chess_qdt45.svg" },
            { blackPiece(Piece::King), "images/Chess_kdt45.svg" },
        };

        return result;
    }

    auto buildMoveFromCoordinates(mutex* gameMutex, Game* game, int srcRow, int srcColumn,
                                  int dstRow, int dstColumn, optional<Piece> promoted) -> pair<optional<Move>, wisdom::Color>
    {
        lock_guard guard { *gameMutex };
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

    auto validateIsLegalMove(mutex* gameMutex, Game* game, Move selectedMove) -> bool
    {
        lock_guard guard { *gameMutex };
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
}

GameModel::GameModel(QObject *parent)
    : QAbstractListModel(parent),
      myPieceToImagePath { initPieceMap(this) },
      myPieces {},
      myChessEngine { new ChessEngine { myGame, &myGameMutex } },
      myChessEngineThread {},
      myGame { make_shared<Game>(gameFromFen("8/PPPPPPPP/2N2N2/8/8/8/1k4K1/8 w - - 0 1")) }
{
    // Initialize the piece list from the game->board.
    auto board = myGame->get_board();
    myGame->set_periodic_notified(&myChessEngineNotifier);

    for (int row = 0; row < 8; row++) {
        for (int column = 0; column < 8; column++) {
            auto piece = board.piece_at(row, column);
            if (piece != Piece_And_Color_None) {
                PieceModel newPiece {
                    row, column, piece, myPieceToImagePath[to_int8(piece)]
                };
                auto pieceStr = to_string(piece);
                myPieces.append(newPiece);
            }
        }
    }

    // Connect event handlers for the computer and human making moves:
    connect(this, &GameModel::humanMoved, myChessEngine, &ChessEngine::opponentMoved);
    connect(myChessEngine, &ChessEngine::engineMoved, this, &GameModel::updateModelStateForMove);

    // Initialize the engine when the engine thread starts:
    connect(&myChessEngineThread, &QThread::started, myChessEngine, &ChessEngine::init);

    // exit event loop from engine thread when we start exiting:
    connect(this, &GameModel::terminationStarted, &myChessEngineThread, &QThread::quit);

    // Cleanup chess engine when chess engine thread exits:
    connect(&myChessEngineThread, &QThread::finished, myChessEngine, &QObject::deleteLater);

    // Move the ownership of the engine to the engine thread so slots run on that thread:
    myChessEngine->moveToThread(&myChessEngineThread);
    myChessEngineThread.start();
}

int GameModel::rowCount(const QModelIndex& index) const
{
    return myPieces.count();
}

QVariant GameModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant {};
    }

    qDebug() << "Index: " << index;

    int dataRow = index.row();

    PieceModel piece = myPieces.at(dataRow);
    if (role == RowRole) {
        return piece.row;
    } else if (role == ColumnRole) {
        return piece.column;
    } else if (role == PieceImageRole) {
        return piece.pieceImage;
    } else {
        return QVariant{};
    }
}

QHash<int, QByteArray> GameModel::roleNames() const
{
    static QHash<int, QByteArray> mapping {
        {RowRole, "row"},
        {ColumnRole, "column"},
        {PieceImageRole, "pieceImage"},
    };

    return mapping;
}

void GameModel::movePiece(int srcRow, int srcColumn,
                          int dstRow, int dstColumn)
{
    movePieceWithPromotion(srcRow, srcColumn, dstRow, dstColumn, {});
}

void GameModel::promotePiece(int srcRow, int srcColumn, int dstRow, int dstColumn, QString pieceString)
{
    optional<Piece> pieceType = pieceFromString(pieceString);
    movePieceWithPromotion(srcRow, srcColumn, dstRow, dstColumn, pieceType);
}

void GameModel::movePieceWithPromotion(int srcRow, int srcColumn,
                                       int dstRow, int dstColumn, std::optional<wisdom::Piece> pieceType)
{
    auto [optionalMove, who] = buildMoveFromCoordinates(&myGameMutex, myGame.get(), srcRow,
            srcColumn, dstRow, dstColumn, pieceType);
    if (!optionalMove.has_value()) {
        return;
    }
    auto move = *optionalMove;
    if (!validateIsLegalMove(&myGameMutex, myGame.get(), move)) {
        return;
    }
    updateChessEngineForHumanMove(move);
    emit humanMoved(move);
    updateModelStateForMove(move, who);
}

bool GameModel::needsPawnPromotion(int srcRow, int srcColumn, int dstRow, int dstColumn)
{
    auto [optionalMove, who] = buildMoveFromCoordinates(&myGameMutex, myGame.get(), srcRow,
            srcColumn, dstRow, dstColumn, Piece::Queen);
    if (!optionalMove.has_value()) {
        return false;
    }
    return is_promoting_move(*optionalMove);
}

void GameModel::applicationExiting()
{
    qDebug() << "Trying to exit application...";

    myChessEngineThread.requestInterruption();
    emit terminationStarted();

    myChessEngineThread.wait();
}

void GameModel::updateModelStateForMove(Move selectedMove, wisdom::Color who)
{
    Coord src = move_src(selectedMove);
    Coord dst = move_dst(selectedMove);

    int srcRow = Row(src);
    int srcColumn = Column(src);
    int dstRow = Row(dst);
    int dstColumn = Column(dst);

    updateCurrentTurn();

    auto count = myPieces.count();
    for (int i = 0; i < count; i++) {
        auto& pieceModel = myPieces[i];
        if (pieceModel.row == dstRow && pieceModel.column == dstColumn) {
            beginRemoveRows(QModelIndex{}, i, i);
            myPieces.removeAt(i);
            qDebug() << "removing index: " << i;
            count--;
            endRemoveRows();
        }
        if ((pieceModel.row == srcRow && pieceModel.column == srcColumn)) {
            pieceModel.row = dstRow;
            pieceModel.column = dstColumn;

            QVector<int> rolesChanged { RowRole, ColumnRole };
            QModelIndex changedIndex = index(i, 0);

            if (is_promoting_move(selectedMove)) {
                auto promotedPiece = move_get_promoted_piece(selectedMove);
                auto newImagePath = myPieceToImagePath[to_int8(promotedPiece)];
                pieceModel.pieceImage = newImagePath;
                rolesChanged.append( PieceImageRole );
            }

            qDebug() << "index " << i << "changed";
            emit dataChanged(changedIndex, changedIndex, rolesChanged);
        }
        if (is_castling_move(selectedMove)) {
            auto sourceRookRow = who == wisdom::Color::White ? 7 : 0;
            auto sourceRookColumn = is_castling_move_on_king_side(selectedMove)
                    ? King_Rook_Column : Queen_Rook_Column;
            auto dstRookColumn = is_castling_move_on_king_side(selectedMove)
                    ? Kingside_Castled_Rook_Column : Queenside_Castled_Rook_Column;

            if (pieceModel.row == sourceRookRow && pieceModel.column == sourceRookColumn) {
                pieceModel.column = dstRookColumn;
                QVector<int> rolesChanged { ColumnRole };
                QModelIndex changedIndex = index(i, 0);

                qDebug() << "index " << i << "changed";
                emit dataChanged(changedIndex, changedIndex, rolesChanged);
            }
        }
    }
}

void GameModel::updateChessEngineForHumanMove(Move selectedMove)
{
    lock_guard guard { myGameMutex };

    myGame->move(selectedMove);
}

void GameModel::updateCurrentTurn()
{
    auto newColor = [this]() {
    lock_guard guard { myGameMutex };
        return myGame->get_current_turn();
    }();
    setCurrentTurn(mapColor(newColor));
}

void GameModel::setCurrentTurn(ColorEnumValue newColor)
{
    if (newColor != myCurrentTurn) {
        myCurrentTurn = newColor;
        emit currentTurnChanged();
    }
}
