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
    Q_PROPERTY(bool drawProposedToHuman READ drawProposedToHuman WRITE setDrawProposedToHuman NOTIFY drawProposedToHumanChanged)

public:
    explicit GameModel(QObject *parent = nullptr);
    virtual ~GameModel() override;

    Q_INVOKABLE void start();
    Q_INVOKABLE bool needsPawnPromotion(int srcRow, int srcColumn, int dstRow, int dstColumn);

    ColorEnum::Value currentTurn();
    void setCurrentTurn(ColorEnumValue newColor);

    void setGameStatus(const QString& newStatus);
    auto gameStatus() -> QString;

    void setDrawProposedToHuman(bool drawProposedToHuman);
    auto drawProposedToHuman() -> bool;

signals:
    void gameStarted(gsl::not_null<ChessGame*> game);

    void humanMoved(wisdom::Move move, wisdom::Color who);
    void engineMoved(wisdom::Move move, wisdom::Color who);

    void currentTurnChanged();
    void gameStatusChanged();

    // Use a property to communicate to QML and the human player:
    void drawProposedToHumanChanged();

    // Use a raw signal to communicate to the thread engine on the other thread.
    // We don't want to send on toggling the boolean.
    void proposeDrawToEngine();

    void terminationStarted();

public slots:
    void movePiece(int srcRow, int srcColumn,
                   int dstRow, int dstColumn);
    void engineThreadMoved(wisdom::Move move, wisdom::Color who);
    void promotePiece(int srcRow, int srcColumn, int dstRow, int dstColumn, QString pieceType);
    void applicationExiting();
    void updateGameStatus();
    void drawProposalResponse(bool accepted);

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
    bool myDrawProposedToHuman = false;

    // Track whether draw was proposed to only propose a draw once per game:
    bool myDrawEverProposed = false;

    // last move before the draw proposal
    std::optional<std::function<void()>> myLastDelayedMoveSignal {};

    void init();
    void setupNewEngineThread();
    void movePieceWithPromotion(int srcRow, int srcColumn,
                                int dstRow, int dstColumn, std::optional<wisdom::Piece> piece);


    // Propose a draw to either the human or computer.
    void proposeDraw(wisdom::Player player);

    // Returns the color of the next turn:
    auto updateChessEngineForHumanMove(wisdom::Move selectedMove) -> wisdom::Color;

    // Update the current turn to the new color.
    void updateCurrentTurn(wisdom::Color newColor);

    // Emit appropriate player moved signal, or delay it for a draw proposal.
    void checkForDrawAndEmitPlayerMoved(wisdom::Player playerType, wisdom::Move move, wisdom::Color who);
};

#endif // GAMEMODEL_HG
