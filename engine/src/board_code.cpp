#include "board_code.hpp"
#include "board.hpp"
#include "coord.hpp"
#include "board_builder.hpp"

#include <ostream>

namespace wisdom
{
    BoardCode::BoardCode()
    {
        auto iterator = Board::allCoords();
        my_code = std::accumulate (
            iterator.begin(),
            iterator.end(),
            my_code,
            [](const auto value, const auto coord)
            {
                return value ^ boardCodeHash (coord, Piece_And_Color_None); 
            }
        );
    }

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

        auto en_passant_targets = board.getEnPassantTargets();
        if (en_passant_targets[Color_Index_White] != nullopt)
        {
            setEnPassantTarget (Color::White, *en_passant_targets[Color_Index_White]);
        }
        else if (en_passant_targets[Color_Index_Black] != nullopt)
        {
            setEnPassantTarget (Color::Black, *en_passant_targets[Color_Index_Black]);
        }
        else
        {
            clearEnPassantTargets ();
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

        auto en_passant_targets = builder.getEnPassantTargets();
        if (en_passant_targets[Color_Index_White] != nullopt)
        {
            result.setEnPassantTarget (Color::White, *en_passant_targets[Color_Index_White]);
        }
        else if (en_passant_targets[Color_Index_Black] != nullopt)
        {
            result.setEnPassantTarget (Color::Black, *en_passant_targets[Color_Index_Black]);
        }
        else
        {
            result.clearEnPassantTargets();
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

        removePiece (src, src_piece);
        addPiece (dst, src_piece);

        if (move.isPromoting())
        {
            assert (src_piece_type == Piece::Pawn);
            addPiece (dst, move.getPromotedPiece());
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
}
