#ifndef PIECESMODEL_H
#define PIECESMODEL_H

#include <QAbstractListModel>

#include "chessgame.hpp"

struct PieceInfo
{
    PieceInfo() {}
    PieceInfo(int row, int column, wisdom::ColoredPiece piece, const QString& pieceImage) :
        row { row }, column { column }, piece { piece }, pieceImage { pieceImage }
    {}

    int row;
    int column;
    QString pieceImage;
    wisdom::ColoredPiece piece;
};

class ChessGame;

class PiecesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit PiecesModel(QObject *parent = nullptr);

    static const int Rook_Animation_Delay = 225;

    enum Roles
    {
        RowRole = Qt::UserRole,
        ColumnRole,
        PieceImageRole,
    };

    int rowCount(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void playerMoved(wisdom::Move selectedMove, wisdom::Color who);
    void newGame(gsl::not_null<ChessGame*> game);

private:
    // The game is shared across the main thread and the chess engine thread.
    // Before calling any of the methods in the libwisdom-core library, the mutex
    // must be held because it is not thread safe.
    std::shared_ptr<ChessGame> myChessGame;

    QHash<int8_t, QString> myPieceToImagePath;
    QVector<PieceInfo> myPieces;
};

#endif // PIECESMODEL_H
