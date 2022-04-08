#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <QObject>
#include <QThread>

#include "chessgame.h"
#include "chessengine.h"
#include "colorenum.h"
#include "piecesmodel.h"

class GameModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(ColorEnum::Value currentTurn READ currentTurn WRITE setCurrentTurn NOTIFY currentTurnChanged)
    Q_PROPERTY(QString gameStatus READ gameStatus WRITE setGameStatus NOTIFY gameStatusChanged)

public:
    explicit GameModel(QObject *parent = nullptr);

    Q_INVOKABLE void start();
    Q_INVOKABLE bool needsPawnPromotion(int srcRow, int srcColumn, int dstRow, int dstColumn);

    ColorEnum::Value currentTurn();
    void setCurrentTurn(ColorEnumValue newColor);

    void setGameStatus(const QString& newStatus);
    auto gameStatus() -> QString;

signals:
    void gameStarted(gsl::not_null<ChessGame*> game);
    void humanMoved(wisdom::Move move, wisdom::Color who);
    void engineMoved(wisdom::Move move, wisdom::Color who);
    void terminationStarted();
    void currentTurnChanged();
    void gameStatusChanged();

public slots:
    void movePiece(int srcRow, int srcColumn,
                   int dstRow, int dstColumn);
    void engineThreadMoved(wisdom::Move move, wisdom::Color who);
    void promotePiece(int srcRow, int srcColumn, int dstRow, int dstColumn, QString pieceType);
    void applicationExiting();
    void updateGameStatus();

private:
    // The game is shared across the main thread and the chess engine thread.
    // Before calling any of the methods in the libwisdom-core library, the mutex
    // must be held because it is not thread safe.
    //
    // To do so, call the `->access()`
    // method on the `ChessGame` object, and then use the -> operator to call
    // method on the wisdom::Game* pointer:
    std::shared_ptr<ChessGame> myChessGame;

    // The chess engine runs in this thread, and grabs the game mutext as needed:
    QThread* myChessEngineThread;
    ColorEnumValue myCurrentTurn;
    QString myGameStatus {};

    void init();
    void setupNewEngineThread();
    void movePieceWithPromotion(int srcRow, int srcColumn,
                                int dstRow, int dstColumn, std::optional<wisdom::Piece> piece);

    // Returns the color of the next turn:
    auto updateChessEngineForHumanMove(wisdom::Move selectedMove) -> wisdom::Color;

    // Update the current turn to the new color.
    void updateCurrentTurn(wisdom::Color newColor);
};

#endif // GAMEMODEL_H
