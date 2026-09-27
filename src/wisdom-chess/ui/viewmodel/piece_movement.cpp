#include "wisdom-chess/ui/viewmodel/piece_movement.hpp"

namespace wisdom::ui
{
    auto
    pieceMovement (Move move)
        -> PieceMovement
    {
        auto src = move.getSrc();
        auto dst = move.getDst();

        PieceMovement result {
            .mover = { src, dst },
            .castling_rook = std::nullopt,
            .captured = std::nullopt,
            .promoted_piece = move.getPromotedPiece(),
        };

        if (move.isCastling())
        {
            auto rook_move = castlingRookMove (move);
            result.castling_rook = PieceStep { rook_move.getSrc(), rook_move.getDst() };
        }
        else if (move.isEnPassant())
        {
            result.captured = enPassantTakenPawnCoord (src, dst);
        }
        else if (move.isNormalCapturing())
        {
            result.captured = dst;
        }

        return result;
    }
}
