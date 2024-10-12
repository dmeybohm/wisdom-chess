#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/str.hpp"

#include "wisdom-chess-perft.hpp"

namespace wisdom
{
    using wisdom::perft::MoveCounter;
    using wisdom::perft::Stats;
    using wisdom::perft::toMoveList;

    void Stats::searchMoves ( // NOLINT(misc-no-recursion)
        const wisdom::Board& board,
        wisdom::Color side,
        int depth,
        int max_depth
    ) {
        if (depth >= max_depth)
            return;

        auto target_depth = max_depth - 1;

        const auto moves = generateAllPotentialMoves (board, side);

        for (auto move : moves)
        {
            Board new_board = board.withMove (side, move);

            if (!isLegalPositionAfterMove (new_board, side, move))
                continue;

            if (depth == target_depth)
            {
                counters.nodes++;
                if (move.isAnyCapturing())
                    counters.captures++;

                if (move.isEnPassant())
                    counters.en_passants++;
            }

            searchMoves (new_board, colorInvert (side), depth + 1, max_depth);
        }
    }

    auto wisdom::perft::convertMove (const Board& board, Color who, string move_str) -> Move
    {
        if (move_str.size() != 4 && move_str.size() != 5)
            throw wisdom::Error { "Invalid size of move" };

        // parse the move into the coordinates
        auto src = wisdom::coordParse (move_str.substr (0, 2));
        auto dst = wisdom::coordParse (move_str.substr (2, 2));

        auto promoted = Piece_And_Color_None;
        if (move_str.size() == 5)
        {
            auto promoted_type = pieceFromChar (move_str[4]);
            promoted = ColoredPiece::make (who, promoted_type);
        }

        auto src_piece = board.pieceAt (src);
        auto dst_piece = board.pieceAt (dst);
        assert (pieceColor (src_piece) == who);

        Move result = wisdom::Move::make (src, dst);

        // 1. castling is represented by two space king moves
        if (wisdom::pieceType (src_piece) == Piece::King)
        {
            int castling_row
                = pieceColor (src_piece) == Color::White ? wisdom::Last_Row : wisdom::First_Row;
            if (castling_row == src.row() && castling_row == dst.row()
                && std::abs (src.column() - dst.column()))
            {
                result = Move::makeCastling (src, dst);
            }
        }

        // 2. en-passant is represented without (ep) suffix
        if (wisdom::pieceType (src_piece) == Piece::Pawn)
        {
            if (src.row() != dst.row() && src.column() != dst.column()
                && dst_piece == Piece_And_Color_None)
            {
                result = Move::makeEnPassant (src, dst);
            }
        }

        // 3. captures denoted without x
        if (dst_piece != Piece_And_Color_None)
            result = result.withCapture();

        // 4. Promotions are not in parentheses
        if (promoted != Piece_And_Color_None)
            result = result.withPromotion (promoted);

        return result;
    }

    auto wisdom::perft::toMoveList (const wisdom::Board& board, Color who, const string& move_list)
        -> MoveList
    {
        MoveList result;

        // Make a copy of the board for modifications:
        auto board_copy = board;

        auto moves_str_list = wisdom::split (move_list, " ");

        for (const auto& move_str : moves_str_list)
        {
            auto move = convertMove (board_copy, who, move_str);
            board_copy = board_copy.withMove (who, move);
            who = colorInvert (who);
            result.append (move);
        }

        return result;
    }

    auto wisdom::perft::toPerftMove (const Move& move, Color who) -> string
    {
        if (move.isCastling())
        {
            auto row = who == Color::White ? Last_Row : First_Row;
            auto src_col = King_Column;
            auto dst_col = move.isCastlingOnKingside() ? Kingside_Castled_King_Column
                                                       : Queenside_Castled_King_Column;

            Move normal = wisdom::Move::make (row, src_col, row, dst_col);
            return wisdom::asString (normal.getSrc()) + wisdom::asString (normal.getDst());
        }

        if (move.isEnPassant())
        {
            return wisdom::asString (move.getSrc()) + wisdom::asString (move.getDst());
        }

        if (move.isPromoting())
        {
            auto promoted = move.getPromotedPiece();

            return wisdom::asString (move.getSrc()) + wisdom::asString (move.getDst())
                + wisdom::pieceToChar (promoted);
        }

        return wisdom::asString (move.getSrc()) + wisdom::asString (move.getDst());
    }

    auto
    wisdom::perft::perftResults (const Board& board, Color active_player, int depth)
        -> PerftResults
    {
        wisdom::perft::PerftResults results;
        Stats cumulative;

        auto moves = generateAllPotentialMoves (board, active_player);

        for (const auto& move : moves)
        {
            Stats stats;

            Color next_player = wisdom::colorInvert (active_player);
            auto new_board = board.withMove (active_player, move);

            if (!wisdom::isLegalPositionAfterMove (new_board, active_player, move))
                continue;

            stats.searchMoves (new_board, next_player, 1, depth);

            auto perft_move = wisdom::perft::toPerftMove (move, active_player);
            results.move_results.push_back ({ stats.counters.nodes, perft_move });

            cumulative += stats;
        }

        results.total_nodes = cumulative.counters.nodes;
        return results;
    }

    auto wisdom::perft::applyList (Board& board, Color color, const MoveList& list) -> Color
    {
        for (auto& move : list)
        {
            board = board.withMove (color, move);
            color = wisdom::colorInvert (color);
        }

        return color;
    }

    auto wisdom::perft::asString (const PerftResults& perft_results) -> string
    {
        string output;

        for (const auto& move_result : perft_results.move_results)
        {
            int64_t nodes = move_result.nodes > 0 ? move_result.nodes : 1;
            output += move_result.move + " " + std::to_string (nodes) + "\n";
        }

        output += "\n" + std::to_string (perft_results.total_nodes) + "\n";
        return output;
    }
}
