#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <QAbstractListModel>
#include <QHash>

#include "gamethread.h"
#include "gamethreadnotifier.h"

struct PieceModel
{
    PieceModel() {}
    PieceModel(int row, int column, wisdom::ColoredPiece piece, const QString& pieceImage) :
        row { row }, column { column }, piece { piece }, pieceImage { pieceImage }
    {}

    int row;
    int column;
    QString pieceImage;
    wisdom::ColoredPiece piece;
};

class GameModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit GameModel(QObject *parent = nullptr);

    enum Roles
    {
        RowRole = Qt::UserRole,
        ColumnRole,
        PieceImageRole,
    };

    int rowCount(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

signals:
    void humanMoved(wisdom::Move move);

public slots:
    void movePiece(int srcRow, int srcColumn,
                   int dstRow, int dstColumn);
    void applicationExiting();

private:
    wisdom::Game myGame;

    GameThread myGameThread;
    GameThreadNotifier myGameThreadNotifier;
    QHash<int8_t, QString> myPieceToImagePath;
    QVector<PieceModel> myPieces;

    void handleMoveUpdate(wisdom::Move selectedMove, wisdom::Color who);
};

#endif // GAMEMODEL_H
