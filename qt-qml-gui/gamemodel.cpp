#include "gamemodel.h"

#include "game.hpp"
#include "generate.hpp"

#include <QDebug>

using namespace wisdom;
using namespace std;

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

GameModel::GameModel(QObject *parent)
    : QAbstractListModel(parent),
      myGame { Color::White, Color::Black },
      myPieceToImagePath { initPieceMap(this) },
      myPieces {}
{
    // Initialize the piece list from the game->board.
    auto board = myGame.get_board();
    auto str = board.to_string();
    qDebug() << QString(str.c_str());
    for (int row = 0; row < 8; row++) {
        for (int column = 0; column < 8; column++) {
            auto piece = board.piece_at(row, column);
            if (piece != Piece_And_Color_None) {
                PieceModel newPiece {
                    row, column, piece, myPieceToImagePath[to_int8(piece)]
                };
                auto pieceStr = to_string(piece);
                qDebug() << "New piece: " << QString(pieceStr.c_str());
                myPieces.append(newPiece);
            }
        }
    }
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
    qDebug() << "movePiece(" << srcRow << ":" << srcColumn <<
                "," << dstRow << ":" << dstColumn;

    string srcCoord = string(1, col_to_char(srcColumn)) +
            string(1, row_to_char(srcRow));
    string dstCoord = string(1, col_to_char(dstColumn)) +
            string(1, row_to_char(dstRow));
    string move_as_string = srcCoord + dstCoord;
    string move_as_capturing = srcCoord + "x" + dstCoord;

    qDebug() << "move: " << QString(move_as_string.c_str());
    qDebug() << "move(capturing)" << QString(move_as_capturing.c_str());

    Move selectedMove = move_parse(move_as_string, myGame.get_current_turn());
    Move capturingMove = move_parse(move_as_capturing, myGame.get_current_turn());
    auto selectedMoveStr = to_string(selectedMove);
    qDebug() << QString(selectedMoveStr.c_str());

    auto legalMoves = generate_legal_moves (myGame.get_board(), myGame.get_current_turn());
    auto legalMovesStr = to_string(legalMoves);
    qDebug() << QString(legalMovesStr.c_str());

    auto hasMove = false;
    for (auto legalMove : legalMoves) {
        if (legalMove == selectedMove) {
            hasMove = true;
            break;
        }
        if (legalMove == capturingMove) {
            qDebug() << "using capturing move";
            selectedMove = capturingMove;
            hasMove = true;
            break;
        }
    }

    if (!hasMove) {
        return;
    }

    // make the move:
    myGame.move (selectedMove);

    // todo handle castling / en passant / promotion

    //
    // Find the affected pieces, if any, and update them.
    //
    auto count = myPieces.count();
    auto removedSomething = false;
    for (int i = 0; i < count; i++) {
        auto& pieceModel = myPieces[i];
        if (pieceModel.row == dstRow && pieceModel.column == dstColumn) {
            beginRemoveRows(QModelIndex{}, i, i);
            myPieces.removeAt(i);
            qDebug() << "removing index: " << i;
            removedSomething = true;
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
    }

}
