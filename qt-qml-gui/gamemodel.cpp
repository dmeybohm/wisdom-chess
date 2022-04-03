#include "gamemodel.h"

#include "game.hpp"
#include "generate.hpp"

#include <QDebug>

using namespace wisdom;
using namespace std;

namespace
{
    constexpr auto white_piece(Piece piece) -> int8_t
    {
        return to_int8(make_piece(Color::White, piece));
    }

    constexpr auto black_piece(Piece piece) -> int8_t
    {
        return to_int8(make_piece(Color::Black, piece));
    }

    auto initPieceMap(QObject *parent) -> QHash<int8_t, QString>
    {
        auto result = QHash<int8_t, QString> {
            { white_piece(Piece::Pawn), "images/Chess_plt45.svg" },
            { white_piece(Piece::Rook), "images/Chess_rlt45.svg" },
            { white_piece(Piece::Knight), "images/Chess_nlt45.svg" },
            { white_piece(Piece::Bishop), "images/Chess_blt45.svg" },
            { white_piece(Piece::Queen), "images/Chess_qlt45.svg" },
            { white_piece(Piece::King), "images/Chess_klt45.svg" },
            { black_piece(Piece::Pawn), "images/Chess_pdt45.svg" },
            { black_piece(Piece::Rook), "images/Chess_rdt45.svg" },
            { black_piece(Piece::Knight), "images/Chess_ndt45.svg" },
            { black_piece(Piece::Bishop), "images/Chess_bdt45.svg" },
            { black_piece(Piece::Queen), "images/Chess_qdt45.svg" },
            { black_piece(Piece::King), "images/Chess_kdt45.svg" },
        };

        return result;
    }

    auto buildMoveFromCoordinates(mutex* gameMutex, Game* game, int srcRow, int srcColumn,
                                  int dstRow, int dstColumn) -> pair<optional<Move>, Color>
    {
        lock_guard guard { *gameMutex };
        Coord src = make_coord(srcRow, srcColumn);
        Coord dst = make_coord(dstRow, dstColumn);

        auto who = game->get_current_turn();
        qDebug() << "Mapping coordinates for " << srcRow << ":" << srcColumn << " -> "
                 << dstRow << ":" << dstColumn;
        return { game->map_coordinates_to_move(src, dst, {}), who };
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

    void updateChessEngineForHumanMove(mutex* gameMutex, Game* game, Move selectedMove)
    {
        lock_guard guard { *gameMutex };

        game->move(selectedMove);
    }
}

GameModel::GameModel(QObject *parent)
    : QAbstractListModel(parent),
      myPieceToImagePath { initPieceMap(this) },
      myPieces {},
      myChessEngine { myGame, &myGameMutex },
      myChessEngineThread {},
      myGame { make_shared<Game> (Color::White, Color::Black) }
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

    connect(&myChessEngineThread, &QThread::started, &myChessEngine, &ChessEngine::init);

    // exit event loop from engine thread when we start exiting:
    connect(this, &GameModel::terminationStarted, &myChessEngineThread, &QThread::quit);

    connect(this, &GameModel::humanMoved, &myChessEngine, &ChessEngine::opponentMoved);
    connect(&myChessEngine, &ChessEngine::engineMoved, this, &GameModel::updateModelStateForMove);

    // Cleanup chess engine when chess engine thread exits:
    connect(&myChessEngineThread, &QThread::finished, &QObject::deleteLater);
    myChessEngine.moveToThread(&myChessEngineThread);
    myChessEngineThread.start();
}

int GameModel::rowCount(const QModelIndex &index) const
{
    return myPieces.count();
}

QVariant GameModel::data(const QModelIndex &index, int role) const
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
    auto [optionalMove, who] = buildMoveFromCoordinates(&myGameMutex, myGame.get(), srcRow,
            srcColumn, dstRow, dstColumn);
    if (!optionalMove.has_value()) {
        return;
    }
    auto move = *optionalMove;
    if (!validateIsLegalMove(&myGameMutex, myGame.get(), move)) {
        return;
    }
    updateChessEngineForHumanMove(&myGameMutex, myGame.get(), move);
    emit humanMoved(move);
    updateModelStateForMove(move, who);
}

void GameModel::applicationExiting()
{
    qDebug() << "Trying to exit application...";

    myChessEngineThread.requestInterruption();
    emit terminationStarted();

    myChessEngineThread.wait();
}

void GameModel::updateModelStateForMove(Move selectedMove, Color who)
{
    Coord src = move_src(selectedMove);
    Coord dst = move_dst(selectedMove);

    int srcRow = Row(src);
    int srcColumn = Column(src);
    int dstRow = Row(dst);
    int dstColumn = Column(dst);

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

            qDebug() << "index " << i << "changed";
            emit dataChanged(changedIndex, changedIndex, rolesChanged);
        }
        if (is_castling_move(selectedMove)) {
            auto sourceRookRow = who == Color::White ? 7 : 0;
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
