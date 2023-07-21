#include "perft.hpp"
#include "board.hpp"
#include "check.hpp"
#include "move.hpp"
#include "piece.hpp"
#include "str.hpp"

namespace wisdom
{
    using wisdom::perft::MoveCounter;
    using wisdom::perft::Stats;
    using wisdom::perft::to_move_list;

    void Stats::search_moves (const Board& board, Color side, int depth, int max_depth, // NOLINT(misc-no-recursion)
                              MoveGenerator& generator)
    {
        if (depth >= max_depth)
            return;

        auto target_depth = max_depth - 1;

        const auto moves = generator.generateAllPotentialMoves (board, side);

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

            search_moves (new_board, color_invert (side),
                          depth + 1, max_depth, generator);
        }
    }

    auto wisdom::perft::convert_move (const Board& board, Color who, string move_str) -> Move
    {
        if (move_str.size () != 4 && move_str.size () != 5)
            throw wisdom::Error { "Invalid size of move" };

        // parse the move into the coordinates
        auto src = wisdom::coordParse (move_str.substr (0, 2));
        auto dst = wisdom::coordParse (move_str.substr (2, 2));

        auto promoted = Piece_And_Color_None;
        if (move_str.size () == 5)
        {
            auto promoted_type = piece_from_char (move_str[4]);
            promoted = ColoredPiece::make (who, promoted_type);
        }

        auto src_piece = board.pieceAt (src);
        auto dst_piece = board.pieceAt (dst);
        assert (piece_color (src_piece) == who);

        Move result = wisdom::Move::make (src, dst);

        // 1. castling is represented by two space king moves
        if (wisdom::piece_type (src_piece) == Piece::King)
        {
            int castling_row
                = piece_color (src_piece) == Color::White ? wisdom::Last_Row : wisdom::First_Row;
            if (castling_row == Row (src) && castling_row == Row (dst)
                && std::abs (Column (src) - Column (dst)))
            {
                result = Move::makeCastling (src, dst);
            }
        }

        // 2. en-passant is represented without (ep) suffix
        if (wisdom::piece_type (src_piece) == Piece::Pawn)
        {
            if (Row (src) != Row (dst) && Column (src) != Column (dst)
                && dst_piece == Piece_And_Color_None)
            {
                result = Move::makeEnPassant (src, dst);
            }
        }

        // 3. captures denoted without x
        if (dst_piece != Piece_And_Color_None)
            result = result.withCapture();

        // 4. Promotions are not in parenthesis
        if (promoted != Piece_And_Color_None)
            result = result.withPromotion (promoted);

        return result;
    }

    auto wisdom::perft::to_move_list (const Board& board, Color who, const string& move_list)
        -> MoveList
    {
        MoveList result = MoveList::uncached ();

        // Make a copy of the board for modifications:
        auto board_copy = board;

        auto moves_str_list = wisdom::split (move_list, " ");

        for (const auto& move_str : moves_str_list)
        {
            auto move = convert_move (board_copy, who, move_str);
            board_copy = board_copy.withMove (who, move);
            who = color_invert (who);
            result.push_back (move);
        }

        return result;
    }

    auto wisdom::perft::to_perft_move (const Move& move, Color who) -> string
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
                + wisdom::piece_char (promoted);
        }

        return wisdom::asString (move.getSrc()) + wisdom::asString (move.getDst());
    }

    auto wisdom::perft::perft_results (const Board& board, Color active_player, int depth,
                                       wisdom::MoveGenerator& generator)
        -> PerftResults
    {
        wisdom::perft::PerftResults results;
        Stats cumulative;

        auto moves = generator.generateAllPotentialMoves (board, active_player);

        for (const auto& move : moves)
        {
            Stats stats;

            Color next_player = wisdom::color_invert (active_player);
            auto new_board = board.withMove (active_player, move);

            if (!wisdom::isLegalPositionAfterMove (new_board, active_player, move))
                continue;

            stats.search_moves (new_board, next_player, 1, depth, generator);

            auto perft_move = wisdom::perft::to_perft_move (move, active_player);
            results.move_results.push_back ({ stats.counters.nodes, perft_move });

            cumulative += stats;
        }

        results.total_nodes = cumulative.counters.nodes;
        return results;
    }

    auto wisdom::perft::apply_list (Board& board, Color color, const MoveList& list) -> Color
    {
        for (auto& move : list)
        {
            board = board.withMove (color, move);
            color = wisdom::color_invert (color);
        }

        return color;
    }

    auto wisdom::perft::to_string (const PerftResults& perft_results) -> string
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
