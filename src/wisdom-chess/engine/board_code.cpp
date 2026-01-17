#include <bitset>
#include <ostream>

#include "wisdom-chess/engine/board_code.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/coord.hpp"
#include "wisdom-chess/engine/board_builder.hpp"

namespace wisdom
{
    BoardCode::BoardCode() = default;

    BoardCode::BoardCode (const Board& board)
        : BoardCode::BoardCode {}
    {
        for (auto coord : Board::allCoords())
        {
            ColoredPiece piece = board.pieceAt (coord);
            addPiece (coord, piece);
        }

        auto current_turn = board.getCurrentTurn();
        setCurrentTurn (current_turn);

        auto en_passant_target = board.getEnPassantTarget();
        if (en_passant_target != nullopt)
        {
            setEnPassantTarget (
                en_passant_target->vulnerable_color,
                en_passant_target->coord
            );
        }
        else
        {
            clearEnPassantTarget();
        }

        setCastleState (Color::White, board.getCastlingEligibility (Color::White));
        setCastleState (Color::Black, board.getCastlingEligibility (Color::Black));
    }

    auto BoardCode::fromBoard (const Board& board) -> BoardCode
    {
        return BoardCode { board };
    }

    auto BoardCode::fromBoardBuilder (const BoardBuilder& builder) -> BoardCode
    {
        auto result = BoardCode {};

        for (auto coord : CoordIterator {})
        {
            ColoredPiece piece = builder.pieceAt (coord);
            result.addPiece (coord, piece);
        }

        auto current_turn = builder.getCurrentTurn();
        result.setCurrentTurn (current_turn);

        auto en_passant_target = builder.getEnPassantTarget();
        if (en_passant_target.has_value())
        {
            result.setEnPassantTarget (
                en_passant_target->vulnerable_color,
                en_passant_target->coord
            );
        }
        else
        {
            result.clearEnPassantTarget();
        }

        result.setCastleState (Color::White, builder.getCastleState (Color::White));
        result.setCastleState (Color::Black, builder.getCastleState (Color::Black));
        return result;
    }

    auto BoardCode::fromDefaultPosition() -> BoardCode
    {
        BoardBuilder builder = BoardBuilder::fromDefaultPosition();
        return BoardCode::fromBoardBuilder (builder);
    }

    auto BoardCode::fromEmptyBoard() -> BoardCode
    {
        return BoardCode {};
    }

    void BoardCode::applyMove (const Board& board, Move move) noexcept
    {
        Coord src = move.getSrc();
        Coord dst = move.getDst();

        ColoredPiece src_piece = board.pieceAt (src);

        Piece src_piece_type = pieceType (src_piece);
        Color src_piece_color = pieceColor (src_piece);
        Color opponent_color = colorInvert (src_piece_color);

        if (move.isCastling())
        {
            int src_col, dst_col;
            int row;

            if (move.isCastlingOnKingside())
            {
                dst_col = Kingside_Castled_Rook_Column;
                src_col = Last_Column;
            }
            else
            {
                dst_col = Queenside_Castled_Rook_Column;
                src_col = 0;
            }
            row = src_piece_color == Color::White ? Last_Row : First_Row;

            Coord rook_src = makeCoord (row, src_col);
            ColoredPiece rook = ColoredPiece::make (src_piece_color, Piece::Rook);
            removePiece (rook_src, rook);
            addPiece (makeCoord (row, dst_col), rook);
        }
        else if (move.isEnPassant())
        {
            // subtract horizontal pawn and add no piece there:
            Coord taken_pawn_coord = enPassantTakenPawnCoord (src, dst);
            removePiece (taken_pawn_coord, ColoredPiece::make (opponent_color, Piece::Pawn));
        }
        else if (move.isNormalCapturing())
        {
            ColoredPiece dst_piece = board.pieceAt (dst);
            removePiece (dst, dst_piece);
        }

        removePiece (src, src_piece);

        if (move.isPromoting())
        {
            assert (src_piece_type == Piece::Pawn);
            addPiece (dst, move.getPromotedPiece());
        }
        else
        {
            addPiece (dst, src_piece);
        }
    }

    auto BoardCode::numberOfSetBits() const -> std::size_t
    {
        std::bitset<64> bits { my_code };
        return bits.count();
    }

    auto operator<< (std::ostream& os, const BoardCode& code) -> std::ostream&
    {
        os << code.asString();
        return os;
    }

    auto BoardCode::asString() const noexcept
        -> string
    {
        std::bitset<64> bits { my_code };
        return bits.to_string();
    }
}
