#include <QTimer>

#include "wisdom-chess/engine/coord.hpp"
#include "wisdom-chess/engine/move.hpp"

#include "wisdom-chess/ui/qml/main/pieces_model.hpp"

using namespace wisdom;
using namespace std;

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

void PiecesModel::newGame (gsl::not_null<const ChessGame*> game)
{
    auto game_state = game->state();
    auto board = game_state->getBoard();

    if (my_pieces.count() > 0)
    {
        beginRemoveRows (QModelIndex {}, 0, gsl::narrow<int> (my_pieces.count() - 1));
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
                auto pieceStr = asString (piece);
                beginInsertRows (QModelIndex {}, gsl::narrow<int> (lastRow),
                                 gsl::narrow<int> (lastRow));
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
        return gsl::narrow<int> (my_pieces.count());
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
    };

    return mapping;
}

void
PiecesModel::playerMoved (
    Move selected_move, 
    wisdom::Color who
) {
    Coord src = selected_move.getSrc();
    Coord dst = selected_move.getDst();

    int src_row = coordRow<int> (src);
    int src_column = coordColumn<int> (src);
    int dst_row = coordRow<int> (dst);
    int dst_column = coordColumn<int> (dst);

    auto count = my_pieces.count();
    for (int i = 0; i < count; i++)
    {
        auto& piece_model = my_pieces[i];
        if (piece_model.row == dst_row && piece_model.column == dst_column)
        {
            beginRemoveRows (QModelIndex {}, i, i);
            my_pieces.removeAt (i);
            count--;
            endRemoveRows();
        }
        if ((piece_model.row == src_row && piece_model.column == src_column))
        {
            piece_model.row = dst_row;
            piece_model.column = dst_column;

            QVector<int> roles_changed { RowRole, ColumnRole };
            QModelIndex changed_index = index (i, 0);

            if (selected_move.isPromoting())
            {
                auto promotedPiece = selected_move.getPromotedPiece();
                auto newImagePath = my_piece_to_image_path[toInt8 (promotedPiece)];
                piece_model.pieceImage = newImagePath;
                roles_changed.append (PieceImageRole);
            }

            emit dataChanged (changed_index, changed_index, roles_changed);
        }
        if (selected_move.isCastling())
        {
            auto source_rook_row = who == wisdom::Color::White ? 7 : 0;
            auto source_rook_column
                = selected_move.isCastlingOnKingside() ? King_Rook_Column : Queen_Rook_Column;
            auto dst_rook_column = selected_move.isCastlingOnKingside()
                ? Kingside_Castled_Rook_Column
                : Queenside_Castled_Rook_Column;

            if (piece_model.row == source_rook_row && piece_model.column == source_rook_column)
            {
                piece_model.column = dst_rook_column;
                QPersistentModelIndex changed_index = index (i, 0);

                QTimer::singleShot (
                    Rook_Animation_Delay, 
                    this, 
                    [this, changed_index]()
                    {
                        if (!changed_index.isValid())
                            return;
                        QVector<int> roles_changed { ColumnRole };
                        emit dataChanged (changed_index, changed_index, roles_changed);
                    }
                );
            }
        }
        if (selected_move.isEnPassant())
        {
            int direction = pawnDirection (who) * -1;
            int en_passant_pawn_row = dst_row + direction;
            int en_passant_pawn_col = dst_column;

            if (piece_model.row == en_passant_pawn_row && piece_model.column == en_passant_pawn_col)
            {
                beginRemoveRows (QModelIndex {}, i, i);
                my_pieces.removeAt (i);
                count--;
                endRemoveRows();
            }
        }
    }
}
