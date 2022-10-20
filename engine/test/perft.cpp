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

    void Stats::search_moves (Board& board, Color side, int depth, int max_depth,
                              MoveGenerator& generator)
    {
        if (depth >= max_depth)
            return;

        auto target_depth = max_depth - 1;

        const auto moves = generator.generate_all_potential_moves (board, side);

        for (auto move : moves)
        {
            UndoMove undo_state = board.make_move (side, move);

            if (!was_legal_move (board, side, move))
            {
                board.take_back (side, move, undo_state);
                continue;
            }

            if (depth == target_depth)
            {
                counters.nodes++;
                if (move.is_any_capturing ())
                    counters.captures++;

                if (move.is_en_passant ())
                    counters.en_passants++;
            }

            search_moves (board, color_invert (side), depth + 1, max_depth, generator);

            board.take_back (side, move, undo_state);
        }
    }

    auto wisdom::perft::convert_move (const Board& board, Color who, string move_str) -> Move
    {
        if (move_str.size () != 4 && move_str.size () != 5)
            throw wisdom::Error { "Invalid size of move" };

        // parse the move into the coordinates
        auto src = wisdom::coord_parse (move_str.substr (0, 2));
        auto dst = wisdom::coord_parse (move_str.substr (2, 2));

        auto promoted = Piece_And_Color_None;
        if (move_str.size () == 5)
        {
            auto promoted_type = piece_from_char (move_str[4]);
            promoted = make_piece (who, promoted_type);
        }

        auto src_piece = board.piece_at (src);
        auto dst_piece = board.piece_at (dst);
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
                result = Move::make_castling (src, dst);
            }
        }

        // 2. en-passant is represented without (ep) suffix
        if (wisdom::piece_type (src_piece) == Piece::Pawn)
        {
            if (Row (src) != Row (dst) && Column (src) != Column (dst)
                && dst_piece == Piece_And_Color_None)
            {
                result = Move::make_en_passant (src, dst);
            }
        }

        // 3. captures denoted without x
        if (dst_piece != Piece_And_Color_None)
            result = result.with_capture ();

        // 4. Promotions are not in parenthesis
        if (promoted != Piece_And_Color_None)
            result = result.with_promotion (promoted);

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
            board_copy.make_move (who, move);
            who = color_invert (who);
            result.push_back (move);
        }

        return result;
    }

    auto wisdom::perft::to_perft_move (const Move& move, Color who) -> string
    {
        if (move.is_castling ())
        {
            auto row = who == Color::White ? Last_Row : First_Row;
            auto src_col = King_Column;
            auto dst_col = move.is_castling_on_kingside () ? Kingside_Castled_King_Column
                                                           : Queenside_Castled_King_Column;

            Move normal = wisdom::Move::make (row, src_col, row, dst_col);
            return wisdom::to_string (normal.get_src ()) + wisdom::to_string (normal.get_dst ());
        }

        if (move.is_en_passant ())
        {
            return wisdom::to_string (move.get_src ()) + wisdom::to_string (move.get_dst ());
        }

        if (move.is_promoting ())
        {
            auto promoted = move.get_promoted_piece ();

            return wisdom::to_string (move.get_src ()) + wisdom::to_string (move.get_dst ())
                + wisdom::piece_char (promoted);
        }

        return wisdom::to_string (move.get_src ()) + wisdom::to_string (move.get_dst ());
    }

    auto wisdom::perft::perft_results (const Board& board, Color active_player, int depth,
                                       wisdom::MoveGenerator& generator)
        -> PerftResults
    {
        Board board_copy = board;
        wisdom::perft::PerftResults results;
        Stats cumulative;

        auto moves = generator.generate_all_potential_moves (board_copy, active_player);

        for (const auto& move : moves)
        {
            Stats stats;

            Color next_player = wisdom::color_invert (active_player);
            auto undo_state = board_copy.make_move (active_player, move);

            if (!wisdom::was_legal_move (board_copy, active_player, move))
            {
                board_copy.take_back (active_player, move, undo_state);
                continue;
            }

            stats.search_moves (board_copy, next_player, 1, depth, generator);
            board_copy.take_back (active_player, move, undo_state);

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
            board.make_move (color, move);
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
