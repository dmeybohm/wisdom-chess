#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <QAbstractListModel>
#include <QHash>
#include <QThread>

#include "chessengine.h"
#include "chessenginenotifier.h"
#include "colorenum.h"

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

    Q_PROPERTY(ColorEnum::Value currentTurn READ currentTurn NOTIFY currentTurnChanged)

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

    Q_INVOKABLE bool needsPawnPromotion(int srcRow, int srcColumn, int dstRow, int dstColumn);

    void movePieceWithPromotion(int srcRow, int srcColumn,
                                int dstRow, int dstColumn, std::optional<wisdom::Piece> piece);
signals:
    void humanMoved(wisdom::Move move);
    void terminationStarted();
    void currentTurnChanged();
    ColorEnum::Value currentTurn();

public slots:
    void movePiece(int srcRow, int srcColumn,
                   int dstRow, int dstColumn);
    void promotePiece(int srcRow, int srcColumn, int dstRow, int dstColumn, QString pieceType);
    void applicationExiting();

private:
    // The game is shared across the main thread and the chess engine thread.
    // Before calling any of the methods in the libwisdom-core library, the mutex
    // must be held because it is not thread safe.
    std::shared_ptr<wisdom::Game> myGame;
    std::mutex myGameMutex;

    // The chess engine runs in this thread, and grabs the game mutext as needed:
    QThread myChessEngineThread;
    ChessEngine* myChessEngine;
    ChessEngineNotifier myChessEngineNotifier {};
    ColorEnumValue myCurrentTurn;

    QHash<int8_t, QString> myPieceToImagePath;
    QVector<PieceModel> myPieces;

    void updateModelStateForMove(wisdom::Move selectedMove, wisdom::Color who);
    void updateChessEngineForHumanMove(wisdom::Move selectedMove);
    void updateCurrentTurn();
    void setCurrentTurn(ColorEnumValue newColor);
};

#endif // GAMEMODEL_H
