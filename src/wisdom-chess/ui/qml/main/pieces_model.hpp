#pragma once

#include <QAbstractListModel>
#include <QVariant>

#include "wisdom-chess/ui/qml/main/chess_game.hpp"

struct PieceInfo
{
    PieceInfo() 
        : row { 0 }
        , column { 0 }
        , pieceImage { "" }
        , piece { wisdom::Piece_And_Color_None }
    {
    }

    PieceInfo (
        int row, 
        int column, 
        wisdom::ColoredPiece piece, 
        QString pieceImage
    ) 
        : row { row }
        , column { column }
        , piece { piece }
        , pieceImage { std::move (pieceImage) }
    {
    }

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
    explicit PiecesModel (QObject* parent = nullptr);

    static const int Rook_Animation_Delay = 225;

    enum Roles
    {
        RowRole = Qt::UserRole,
        ColumnRole,
        PieceImageRole,
    };

    [[nodiscard]] int 
    rowCount (const QModelIndex& index) const override;

    [[nodiscard]] QVariant data (
        const QModelIndex& index, 
        int role = Qt::DisplayRole
    ) const override; 

    [[nodiscard]] QHash<int, QByteArray>
    roleNames() const override;

public slots:
    void playerMoved (
        wisdom::Move selected_move, 
        wisdom::Color who
    );
    void newGame (
        gsl::not_null<const ChessGame*> game
    );

private:
    QHash<int8_t, QString> my_piece_to_image_path;
    QVector<PieceInfo> my_pieces;
};

