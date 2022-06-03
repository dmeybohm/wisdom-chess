#include "piecesmodel.hpp"
#include "coord.hpp"
#include "move.hpp"

#include <QTimer>

using namespace wisdom;
using namespace std;

namespace {

    constexpr auto whitePiece(Piece piece) -> int8_t
    {
        return to_int8(make_piece(Color::White, piece));
    }

    constexpr auto blackPiece(Piece piece) -> int8_t
    {
        return to_int8(make_piece(Color::Black, piece));
    }

    auto initPieceMap() -> QHash<int8_t, QString>
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
}

PiecesModel::PiecesModel(QObject *parent)
    : QAbstractListModel(parent)
    , myPieceToImagePath { initPieceMap() }
    , myPieces {}
{
}

void PiecesModel::newGame(gsl::not_null<const ChessGame*> game)
{
    auto gameState = game->state();
    auto board = gameState->get_board();

    if (myPieces.count() > 0) {
        beginRemoveRows(QModelIndex{},  0,
                        gsl::narrow<int>(myPieces.count() - 1));
        myPieces.clear();
        endRemoveRows();
    }

    for (int row = 0; row < 8; row++) {
        for (int column = 0; column < 8; column++) {
            auto piece = board.piece_at(row, column);
            if (piece != Piece_And_Color_None) {
                PieceInfo newPiece {
                    row, column, piece, myPieceToImagePath[to_int8(piece)]
                };
                auto lastRow = myPieces.count();
                auto pieceStr = to_string(piece);
                beginInsertRows(QModelIndex{}, gsl::narrow<int>(lastRow), gsl::narrow<int>(lastRow));
                myPieces.append(newPiece);
                endInsertRows();
            }
        }
    }
}

int PiecesModel::rowCount(const QModelIndex& index) const
{
    return gsl::narrow<int>(myPieces.count());
}

QVariant PiecesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant {};
    }

    qDebug() << "Index: " << index;

    int dataRow = index.row();

    auto pieceInfo = myPieces.at(dataRow);
    if (role == RowRole) {
        return pieceInfo.row;
    } else if (role == ColumnRole) {
        return pieceInfo.column;
    } else if (role == PieceImageRole) {
        return pieceInfo.pieceImage;
    } else {
        return QVariant{};
    }
}

QHash<int, QByteArray> PiecesModel::roleNames() const
{
    static QHash<int, QByteArray> mapping {
        {RowRole, "row"},
        {ColumnRole, "column"},
        {PieceImageRole, "pieceImage"},
    };

    return mapping;
}

void PiecesModel::playerMoved(Move selectedMove, wisdom::Color who)
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

            if (is_promoting_move(selectedMove)) {
                auto promotedPiece = move_get_promoted_piece(selectedMove);
                auto newImagePath = myPieceToImagePath[to_int8(promotedPiece)];
                pieceModel.pieceImage = newImagePath;
                rolesChanged.append( PieceImageRole );
            }

            qDebug() << "index " << i << "changed";
            emit dataChanged(changedIndex, changedIndex, rolesChanged);
        }
        if (is_special_castling_move(selectedMove)) {
            auto sourceRookRow = who == wisdom::Color::White ? 7 : 0;
            auto sourceRookColumn = is_castling_move_on_king_side(selectedMove)
                    ? King_Rook_Column : Queen_Rook_Column;
            auto dstRookColumn = is_castling_move_on_king_side(selectedMove)
                    ? Kingside_Castled_Rook_Column : Queenside_Castled_Rook_Column;

            if (pieceModel.row == sourceRookRow && pieceModel.column == sourceRookColumn) {
                pieceModel.column = dstRookColumn;
                QModelIndex changedIndex = index(i, 0);

                QTimer::singleShot(Rook_Animation_Delay, this, [this, changedIndex](){
                    qDebug() << "index " << changedIndex << "changed";
                    QVector<int> rolesChanged { ColumnRole };
                    emit dataChanged(changedIndex, changedIndex, rolesChanged);
                });
            }
        }
        if (is_special_en_passant_move(selectedMove)) {
            int direction = pawn_direction(who) * -1;
            int enPassantPawnRow = dstRow + direction;
            int enPassantPawnCol = dstColumn;

            if (pieceModel.row == enPassantPawnRow && pieceModel.column == enPassantPawnCol) {
                beginRemoveRows(QModelIndex{}, i, i);
                myPieces.removeAt(i);
                qDebug() << "removing index: " << i;
                count--;
                endRemoveRows();
            }
        }
    }
}
