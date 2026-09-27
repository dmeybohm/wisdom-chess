#include "wisdom-chess/engine/coord.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/board.hpp"

#include "wisdom-chess/ui/qml/main/pieces_model.hpp"
#include "wisdom-chess/ui/viewmodel/piece_movement.hpp"

using namespace wisdom;
using namespace std;

namespace wisdom::ui::qml
{
    // The engine's types, not the wisdom::ui mirrors QML sees.
    using wisdom::Color;
    using wisdom::Player;

    namespace
    {
        constexpr auto
        whitePiece (Piece piece)
            -> int8_t
        {
            return toInt8 (ColoredPiece::make (Color::White, piece));
        }

        constexpr auto
        blackPiece (Piece piece)
            -> int8_t
        {
            return toInt8 (ColoredPiece::make (Color::Black, piece));
        }

        auto
        initPieceMap()
            -> QHash<int8_t, QString>
        {
            auto result = QHash<int8_t, QString> {
                { whitePiece (Piece::Pawn), "../images/Chess_plt45.svg" },
                { whitePiece (Piece::Rook), "../images/Chess_rlt45.svg" },
                { whitePiece (Piece::Knight), "../images/Chess_nlt45.svg" },
                { whitePiece (Piece::Bishop), "../images/Chess_blt45.svg" },
                { whitePiece (Piece::Queen), "../images/Chess_qlt45.svg" },
                { whitePiece (Piece::King), "../images/Chess_klt45.svg" },
                { blackPiece (Piece::Pawn), "../images/Chess_pdt45.svg" },
                { blackPiece (Piece::Rook), "../images/Chess_rdt45.svg" },
                { blackPiece (Piece::Knight), "../images/Chess_ndt45.svg" },
                { blackPiece (Piece::Bishop), "../images/Chess_bdt45.svg" },
                { blackPiece (Piece::Queen), "../images/Chess_qdt45.svg" },
                { blackPiece (Piece::King), "../images/Chess_kdt45.svg" },
            };

            return result;
        }
    }

    PiecesModel::PiecesModel (QObject* parent)
            : QAbstractListModel (parent)
            , my_piece_to_image_path { initPieceMap() }
            , my_pieces {}
    {
    }

    void PiecesModel::newGame (wisdom::nonnull<const ChessGame> game)
    {
        auto game_state = game->state();
        auto board = game_state->getBoard();

        if (my_pieces.count() > 0)
        {
            beginRemoveRows (QModelIndex {}, 0, wisdom::narrow<int> (my_pieces.count() - 1));
            my_pieces.clear();
            endRemoveRows();
        }

        for (int row = 0; row < wisdom::Num_Rows; row++)
        {
            for (int column = 0; column < wisdom::Num_Columns; column++)
            {
                auto piece = board.pieceAt (row, column);
                if (piece != Piece_And_Color_None)
                {
                    PieceInfo newPiece { row, column, piece, my_piece_to_image_path[toInt8 (piece)] };
                    auto lastRow = my_pieces.count();
                    beginInsertRows (QModelIndex {}, wisdom::narrow<int> (lastRow),
                                     wisdom::narrow<int> (lastRow));
                    my_pieces.append (newPiece);
                    endInsertRows();
                }
            }
        }
    }

    int PiecesModel::rowCount (const QModelIndex& index) const
    {
        if (index.isValid())
        {
            // At some index - no child rows.
            return 0;
        }
        else
        {
            // At the root: equal to the number of top-level rows.
            return wisdom::narrow<int> (my_pieces.count());
        }
    }

    auto
    PiecesModel::data (
        const QModelIndex& index,
        int role
    ) const
        -> QVariant
    {
        if (!index.isValid())
        {
            return QVariant {};
        }

        int data_row = index.row();
        auto piece_info = my_pieces.at (data_row);
        switch (role)
        {
            case RowRole:
                return piece_info.row;
            case ColumnRole:
                return piece_info.column;
            case PieceImageRole:
                return piece_info.pieceImage;
            case IsCastlingRookRole:
                return piece_info.is_castling_rook;
            case CastlingSourceColumnRole:
                return piece_info.castling_source_column;
            default:
                return QVariant {};
        }
    }

    auto
    PiecesModel::roleNames() const
        -> QHash<int, QByteArray>
    {
        static QHash<int, QByteArray> mapping {
            { RowRole, "row" },
            { ColumnRole, "column" },
            { PieceImageRole, "pieceImage" },
            { IsCastlingRookRole, "isCastlingRook" },
            { CastlingSourceColumnRole, "castlingSourceColumn" },
        };

        return mapping;
    }

    auto
    PiecesModel::indexOf (Coord coord) const
        -> int
    {
        for (int i = 0; i < my_pieces.count(); i++)
        {
            const auto& piece_model = my_pieces[i];
            if (piece_model.row == coord.row<int>() && piece_model.column == coord.column<int>())
                return i;
        }
        return -1;
    }

    void
    PiecesModel::playerMoved (
        Move selected_move,
        wisdom::Color who
    ) {
        auto movement = ui::pieceMovement (selected_move);

        for (int i = 0; i < my_pieces.count(); i++)
        {
            auto& piece_model = my_pieces[i];

            if (piece_model.is_castling_rook || piece_model.castling_source_column != -1)
            {
                piece_model.is_castling_rook = false;
                piece_model.castling_source_column = -1;

                QModelIndex cleared_index = index (i, 0);
                emit dataChanged (
                    cleared_index,
                    cleared_index,
                    QVector<int> { IsCastlingRookRole, CastlingSourceColumnRole }
                );
            }
        }

        if (movement.captured.has_value())
        {
            int captured = indexOf (*movement.captured);
            if (captured >= 0)
            {
                beginRemoveRows (QModelIndex {}, captured, captured);
                my_pieces.removeAt (captured);
                endRemoveRows();
            }
        }

        int mover = indexOf (movement.mover.src);
        if (mover >= 0)
        {
            auto& piece_model = my_pieces[mover];
            piece_model.row = movement.mover.dst.row<int>();
            piece_model.column = movement.mover.dst.column<int>();

            QVector<int> roles_changed { RowRole, ColumnRole };

            if (movement.promoted_piece != Piece::None)
            {
                auto promoted_piece = ColoredPiece::make (who, movement.promoted_piece);
                piece_model.pieceImage = my_piece_to_image_path[toInt8 (promoted_piece)];
                roles_changed.append (PieceImageRole);
            }

            QModelIndex changed_index = index (mover, 0);
            emit dataChanged (changed_index, changed_index, roles_changed);
        }

        if (movement.castling_rook.has_value())
        {
            int rook = indexOf (movement.castling_rook->src);
            if (rook >= 0)
            {
                auto& piece_model = my_pieces[rook];
                QModelIndex changed_index = index (rook, 0);

                piece_model.castling_source_column = movement.castling_rook->src.column<int>();
                emit dataChanged (changed_index, changed_index, QVector<int> { CastlingSourceColumnRole });

                piece_model.column = movement.castling_rook->dst.column<int>();
                piece_model.is_castling_rook = true;
                emit dataChanged (changed_index, changed_index, QVector<int> { ColumnRole, IsCastlingRookRole });
            }
        }
    }
}
