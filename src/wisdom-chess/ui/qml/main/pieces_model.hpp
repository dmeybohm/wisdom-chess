#pragma once

#include <QAbstractListModel>
#include <QVariant>

#include "wisdom-chess/ui/qml/main/chess_game.hpp"

namespace wisdom::ui::qml
{
    struct PieceInfo
    {
        PieceInfo()
            : row { 0 }
            , column { 0 }
            , piece_image {}
            , piece { wisdom::Piece_And_Color_None }
            , is_castling_rook { false }
            , castling_source_column { -1 }
        {
        }

        PieceInfo (
            int row,
            int column,
            wisdom::ColoredPiece piece,
            QString piece_image
        )
            : row { row }
            , column { column }
            , piece_image { std::move (piece_image) }
            , piece { piece }
            , is_castling_rook { false }
            , castling_source_column { -1 }
        {
        }

        int row;
        int column;
        QString piece_image;
        wisdom::ColoredPiece piece;
        bool is_castling_rook;
        int castling_source_column;
    };

    class ChessGame;

    class PiecesModel final : public QAbstractListModel
    {
        Q_OBJECT

    public:
        explicit PiecesModel (QObject* parent = nullptr);

        enum Roles
        {
            RowRole = Qt::UserRole,
            ColumnRole,
            PieceImageRole,
            IsCastlingRookRole,
            CastlingSourceColumnRole,
        };

        [[nodiscard]] int
        rowCount (const QModelIndex& index) const noexcept override;

        [[nodiscard]] QVariant
        data (
            const QModelIndex& index,
            int role = Qt::DisplayRole
        ) const noexcept override;

        [[nodiscard]] QHash<int, QByteArray>
        roleNames() const noexcept override;

    public slots:
        void playerMoved (
            wisdom::Move selected_move,
            wisdom::Color who
        ) noexcept;
        void newGame (
            wisdom::nonnull<const ChessGame> game
        ) noexcept;

    private:
        // The list row of the piece on the square, or -1.
        [[nodiscard]] auto
        indexOf (wisdom::Coord coord) const
            -> int;

        QHash<int, QString> my_piece_to_image_path;
        QVector<PieceInfo> my_pieces;
    };
}
