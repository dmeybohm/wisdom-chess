#include <iostream>
#include <sstream>
#include <memory>

#include "piece.hpp"
#include "board.hpp"
#include "generate.hpp"
#include "evaluate.hpp"
#include "check.hpp"
#include "move_tree.hpp"
#include "search.hpp"
#include "move_timer.hpp"
#include "move_history.hpp"
#include "multithread_search.hpp"
#include "logger.hpp"
#include "history.hpp"

namespace wisdom
{
    constexpr int Max_Depth = 16;
    constexpr int Max_Search_Seconds = 10;

    thread_local int nodes_visited, cutoffs;

    using system_clock_t = std::chrono::time_point<std::chrono::system_clock>;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;
    using std::chrono::seconds;
    using wisdom::Logger;

    static SearchResult recurse_or_evaluate (Board &board, Color side, Logger &output, History &history,
                                             MoveTimer &timer, int depth, int start_depth, int alpha, int beta,
                                             Move move)
    {
        if (depth <= 0)
        {
            int score = evaluate_and_check_draw (board, side, start_depth - depth,
                                                 move, history);
            return SearchResult { move, false, score, start_depth - depth, {} };
        }
        else
        {
            // Check the transposition table for the move:
            auto optional_transposition = board.check_transposition_table (side, depth);
            if (optional_transposition.has_value ())
            {
                return SearchResult { move, false, optional_transposition->score * -1,
                                      start_depth - depth, {} };
            }

            SearchResult other_search_result = search (board, color_invert (side),
                                                       output, history, timer,
                                                       depth - 1, start_depth, -beta, -alpha);
            if (other_search_result.timed_out)
                return other_search_result;

            other_search_result.score *= -1;
            return other_search_result;
        }
    }

    static SearchResult search_moves (Board &board, Color side, Logger &output, History &history,
                                      MoveTimer &timer, int depth, int start_depth, int alpha, int beta,
                                      const ScoredMoveList &moves)
    {
        int best_score = -Initial_Alpha;
        std::optional<Move> best_move {};
        VariationGlimpse best_variation;

        for (auto [move, move_score] : moves)
        {
            if (timer.is_triggered ())
                return SearchResult::from_timeout ();

            UndoMove undo_state = do_move (board, side, move);

            if (!was_legal_move (board, side, move))
            {
                undo_move (board, side, move, undo_state);
                continue;
            }

            nodes_visited++;

            history.add_position_and_move (board, move);

            SearchResult other_search_result = recurse_or_evaluate (board, side, output, history, timer,
                                                                    depth, start_depth, alpha, beta, move);


            if (!other_search_result.timed_out)
                board.add_evaluation_to_transposition_table (other_search_result.score, side, depth);

            history.remove_position_and_last_move (board);
            undo_move (board, side, move, undo_state);

            if (other_search_result.timed_out)
                return other_search_result;

            int score = other_search_result.score;

            if (score > best_score)
            {
                best_score = score;
                best_move = move;

                other_search_result.variation_glimpse.push_front (move);
                best_variation = other_search_result.variation_glimpse;
            }

            if (best_score > alpha)
                alpha = best_score;

            if (alpha >= beta)
            {
                cutoffs++;
                break;
            }
        }

        return SearchResult { best_move, false, best_score, start_depth - depth, best_variation };
    }

    SearchResult
    search (Board &board, Color side, Logger &output, History &history, MoveTimer &timer,
            int depth, int start_depth, int alpha, int beta)
    {
        MoveGenerator generator = board.move_generator ();

        ScoredMoveList moves = generator.generate (board, side);

        SearchResult result = search_moves (board, side, output, history, timer,
                                            depth, start_depth, alpha, beta, moves);

        if (result.timed_out)
            return result;
        
        // if there are no legal moves, then the current player is in a stalemate or checkmate position.
        if (!result.move.has_value ())
        {
            SearchResult no_moves_available_result { result };
            auto [my_king_row, my_king_col] = king_position (board, side);
            no_moves_available_result.score = is_king_threatened (board, side, my_king_row, my_king_col) ?
                    -1 * checkmate_score_in_moves (start_depth - depth) : 0;
            board.add_evaluation_to_transposition_table (no_moves_available_result.score, side, depth);
            return no_moves_available_result;
        }

        return result;
    }

    static void calc_time (Logger &output, int nodes, system_clock_t start, system_clock_t end)
    {
        auto duration = end - start;
        milliseconds ms = std::chrono::duration_cast<milliseconds> (duration);
        double seconds = ms.count () / 1000.0;

        std::stringstream progress_str;
        progress_str << "search took " << seconds << ", " << nodes / seconds << " nodes/sec";
        output.println (progress_str.str ());
    }

    SearchResult IterativeSearch::iteratively_deepen (Color side)
    {
        SearchResult best_result = SearchResult::from_initial ();

        // For now, only look every other depth
        for (int depth = 0; depth <= my_total_depth; depth <= 2 ? depth++ : depth += 2)
        {
            std::ostringstream ostr;
            ostr << "Searching depth " << depth;
            my_output.println(ostr.str());

            SearchResult next_result = iterate (my_board, side, my_output, my_history, my_timer, depth);
            if (next_result.timed_out)
                break;

            best_result = next_result;
            if (is_checkmating_opponent_score (next_result.score))
                break;

        }

        return best_result;
    }

    SearchResult iterate (Board &board, Color side, Logger &output,
                          History &history, MoveTimer &timer, int depth)
    {
        std::stringstream outstr;
        outstr << "finding moves for " << to_string (side);
        output.println (outstr.str ());

        nodes_visited = 0;
        cutoffs = 0;

        auto start = std::chrono::system_clock::now ();

        SearchResult result = search (board, side, output, history, timer,
                                      depth, depth, -Initial_Alpha, Initial_Alpha);

        auto end = std::chrono::system_clock::now ();
        calc_time (output, nodes_visited, start, end);

        if (result.timed_out)
        {
            std::stringstream progress_str;
            progress_str << "Search timed out" << "\n";
            output.println (progress_str.str ());
            return result;
        }

        if (result.move.has_value())
        {
            Move best_move = result.move.value();
            std::stringstream progress_str;
            progress_str << "move selected = " << to_string (best_move) << " [ score: "
                         << result.score << " ]\n";
            progress_str << "nodes visited = " << nodes_visited << ", cutoffs = " << cutoffs;
            output.println (progress_str.str ());
        }

        // principal variation could be null if search was interrupted
        if (result.variation_glimpse.size () > 0)
        {
            std::stringstream variation_str;
            variation_str << "principal variation: " << result.variation_glimpse.to_string ();
            output.println (variation_str.str ());
        }

        return result;
    }

    std::optional<Move> find_best_move_multithreaded (Board &board, Color side, Logger &output, History &history)
    {
        MoveTimer overdue_timer { Max_Search_Seconds };

        MultithreadSearch search { board, side, output, history, overdue_timer };
        SearchResult result = search.search ();
        return result.move;
    }

    std::optional<Move> find_best_move (Board &board, Color side, Logger &output, History &history)
    {
        MoveTimer overdue_timer { Max_Search_Seconds };
        IterativeSearch iterative_search { board, history, output, overdue_timer, Max_Depth };
        SearchResult result = iterative_search.iteratively_deepen (side);
        return result.move;
    }

    // Get the score for a checkmate discovered X moves away.
    // Checkmates closer to the current position are more valuable than those
    // further away.
    int checkmate_score_in_moves (int moves)
    {
        return Infinity + Infinity / (1 + moves);
    }

    bool is_checkmating_opponent_score (int score)
    {
        return score > Infinity;
    }

}
