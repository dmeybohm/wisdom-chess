#include "piece.hpp"
#include "board.hpp"
#include "evaluate.hpp"
#include "check.hpp"
#include "search.hpp"
#include "transposition_table.hpp"
#include "analytics.hpp"
#include "logger.hpp"

#include <sstream>
#include <iostream>

namespace wisdom
{
    thread_local int nodes_visited, cutoffs;

    using system_clock_t = chrono::time_point<chrono::system_clock>;

    class IterativeSearchImpl
    {
    private:
        Board &my_board;
        History &my_history;
        TranspositionTable &my_transpositions;
        Logger &my_output;
        analysis::Analytics &my_analytics;
        MoveTimer my_timer;
        int my_total_depth;

    public:
        IterativeSearchImpl (Board &board, History &history, TranspositionTable &transpositions,
                             Logger &output, MoveTimer timer, int total_depth) :
                my_board { board },
                my_history { history },
                my_transpositions { transpositions },
                my_output { output },
                my_analytics { analysis::make_dummy_analytics () },
                my_timer { timer },
                my_total_depth { total_depth }
        {
        }

        IterativeSearchImpl (Board &board, History &history, TranspositionTable &transpositions,
                             Logger &output, MoveTimer timer, int total_depth,
                             analysis::Analytics &analytics) :
                my_board { board },
                my_history { history },
                my_transpositions { transpositions },
                my_output { output },
                my_analytics { analytics },
                my_timer { timer },
                my_total_depth { total_depth }
        {
        }

        auto iteratively_deepen (Color side) -> SearchResult;

        auto search_moves (Color side, int depth, int alpha, int beta,
                           const ScoredMoveList &moves,
                           analysis::Decision &decision) -> SearchResult;

        auto iterate (Color side, int depth, analysis::Iteration &iteration)
            -> SearchResult;

        auto search (Color side, int depth, int alpha, int beta,
                     analysis::Decision &decision)
            -> SearchResult;

        auto recurse_or_evaluate (Color side, int depth, int alpha, int beta,
                                  Move move, analysis::Decision &parent,
                                  analysis::Position &position)
            -> SearchResult;

        auto transposition_value (int depth, Move &move, analysis::Position &position,
                                  optional<RelativeTransposition> &transposition) const
            -> SearchResult;

        auto evaluated_value (Color &side, int depth,
                              Move &move, analysis::Position &position) const
            -> SearchResult;

        auto recursed_value (Color &side, int depth, int alpha, int beta,Move &move,
                             analysis::Decision &parent, analysis::Position &position)
            -> SearchResult;
    };

    IterativeSearch::~IterativeSearch() = default;
    IterativeSearch::IterativeSearch (Board &board,
                                      History &history,
                                      TranspositionTable &transpositions,
                                      Logger &output,
                                      MoveTimer timer,
                                      int total_depth) :
            impl { make_unique<IterativeSearchImpl> (
                board,
                history,
                transpositions,
                output,
                timer,
                total_depth,
                analysis::make_dummy_analytics ()
             )}
    {}

    IterativeSearch::IterativeSearch (
        Board &board, History &history, TranspositionTable &transpositions, Logger &output,
        MoveTimer timer, int total_depth, analysis::Analytics &analytics) :
            impl { make_unique<IterativeSearchImpl> (
                board, history, transpositions, output, timer, total_depth, analytics) }
    {
    }

    SearchResult IterativeSearch::iteratively_deepen (Color side)
    {
        return impl->iteratively_deepen (side);
    }

    SearchResult
    IterativeSearchImpl::transposition_value (int depth, Move &move, analysis::Position &position,
                                              optional<RelativeTransposition> &transposition) const
    {
        transposition->variation_glimpse.push_front (move);
        auto result = SearchResult { transposition->variation_glimpse, move, transposition->score,
                                     my_total_depth - depth, false };
        position.finalize (result);
        position.store_transposition_hit (*transposition);
        return result;
    }

    SearchResult IterativeSearchImpl::recurse_or_evaluate (Color side, int depth, int alpha, int beta,
                                                           Move move, analysis::Decision &parent,
                                                           analysis::Position &position)
    {
        // Check the transposition table for the move:
        auto transposition = my_transpositions.lookup_board_for_depth (my_board, side, depth);

        if (transposition.has_value ())
        {
            return transposition_value (depth, move, position, transposition);
        }
        else if (depth <= 0)
        {
            return evaluated_value (side, depth, move, position);
        }
        else
        {
            return recursed_value (side, depth, alpha, beta, move, parent, position);
        }
    }

    SearchResult
    IterativeSearchImpl::recursed_value (Color &side, int depth, int alpha, int beta, Move &move,
                                         analysis::Decision &parent, analysis::Position &position)
    {
        auto decision = parent.make_child (position);

        SearchResult other_search_result = search (color_invert (side), depth - 1,
                                                   -beta, -alpha, decision);
        if (other_search_result.timed_out)
            return other_search_result;

        other_search_result.variation_glimpse.push_front (move);
        other_search_result.score *= -1;

        position.finalize (other_search_result);
        decision.finalize (other_search_result);

        return other_search_result;
    }

    SearchResult
    IterativeSearchImpl::evaluated_value (Color &side, int depth, Move &move,
                                      analysis::Position &position) const
    {
        int score = evaluate_and_check_draw (my_board, side, my_total_depth - depth,
                                             move, my_history);
        auto result = SearchResult::from_evaluated_move (move, score, my_total_depth, depth);

        position.finalize (result);
        return result;
    }

    SearchResult IterativeSearchImpl::search_moves (Color side, int depth, int alpha, int beta,
                                                    const ScoredMoveList &moves,
                                                    analysis::Decision &decision)
    {
        int best_score = -Initial_Alpha;
        std::optional<Move> best_move {};
        VariationGlimpse best_variation;

        for (auto [move, move_score] : moves)
        {
            auto position = decision.make_position (move);

            if (my_timer.is_triggered ())
                return SearchResult::from_timeout ();

            UndoMove undo_state = my_board.make_move (side, move);

            if (!was_legal_move (my_board, side, move))
            {
                my_board.take_back (side, move, undo_state);
                continue;
            }

            nodes_visited++;

            my_history.add_position_and_move (my_board, move);

            SearchResult other_search_result = recurse_or_evaluate (side, depth, alpha, beta, move,
                                                                    decision, position);

            int score = other_search_result.score;

            if (score > best_score)
            {
                best_score = score;
                best_move = move;

                best_variation = other_search_result.variation_glimpse;
                decision.preliminary_choice (position);
            }

            if (best_score > alpha)
                alpha = best_score;

            if (!other_search_result.timed_out)
            {
                my_transpositions.add (my_board,
                    other_search_result.score, side, depth,
                    other_search_result.variation_glimpse
                );
            }

            my_history.remove_position_and_last_move (my_board);
            my_board.take_back (side, move, undo_state);

            if (other_search_result.timed_out)
                return other_search_result;

            if (alpha >= beta)
            {
                cutoffs++;
                break;
            }
        }

        auto result = SearchResult { best_variation, best_move, best_score,
                                     my_total_depth - depth, false };
        decision.finalize (result);
        return result;
    }

    SearchResult IterativeSearchImpl::search (Color side, int depth,
                                              int alpha, int beta, analysis::Decision &decision)
    {
        MoveGenerator generator = MoveGenerator { my_transpositions };

        ScoredMoveList moves = generator.generate (my_board, side);

        SearchResult result = search_moves (side, depth, alpha, beta, moves, decision);

        if (result.timed_out)
            return result;
        
        // if there are no legal moves, then the current my_computer_player is in a stalemate or checkmate position.
        if (!result.move.has_value ())
        {
            SearchResult no_moves_available_result { result };
            auto [my_king_row, my_king_col] = my_board.get_king_position (side);

            no_moves_available_result.score = is_king_threatened (my_board, side, my_king_row, my_king_col) ?
                    -1 * checkmate_score_in_moves (my_total_depth - depth) : 0;
            my_transpositions.add (my_board, no_moves_available_result.score, side, depth,
                                   result.variation_glimpse);

            return no_moves_available_result;
        }

        return result;
    }

    static void calc_time (Logger &output, int nodes, system_clock_t start, system_clock_t end)
    {
        auto seconds_duration = chrono::duration<double> (end - start);
        auto seconds = seconds_duration.count();

        std::stringstream progress_str;
        progress_str << "search took " << seconds << "s, " << nodes / seconds << " nodes/sec";
        output.println (progress_str.str ());
    }

    SearchResult IterativeSearchImpl::iteratively_deepen (Color side)
    {
        SearchResult best_result = SearchResult::from_initial ();

        auto iterative_search_analytics = my_analytics.make_iterative_search (my_board, side);

        try
        {
            // For now, only look every other depth
            for (int depth = 0; depth <= my_total_depth; depth == 0 ? depth++ : depth += 2)
            {
                std::ostringstream ostr;
                ostr << "Searching depth " << depth;
                my_output.println (ostr.str ());

                auto iteration_analysis = iterative_search_analytics.make_iteration (depth);
                SearchResult next_result = iterate (side, depth, iteration_analysis);
                if (next_result.timed_out)
                    break;

                if (depth <= 2 || depth % 2 == 1)
                    best_result = next_result;

                if (is_checkmating_opponent_score (next_result.score))
                {
                    best_result = next_result;
                    break;
                }
            }

            return best_result;
        }
        catch (const Error &e)
        {
            std::cerr << "Uncaught error: " << e.message () << "\n";
            std::cerr << e.extra_info () << "\n";
            my_board.dump ();
            std::cerr << "History leading up to move: " << "\n";
            std::cerr << my_history.get_move_history ().to_string () << "\n";
            std::terminate ();
        }
    }

    SearchResult IterativeSearchImpl::iterate (Color side, int depth, analysis::Iteration &iteration)
    {
        std::stringstream outstr;
        outstr << "finding moves for " << to_string (side);
        my_output.println (outstr.str ());

        nodes_visited = 0;
        cutoffs = 0;

        auto analyzed_search = iteration.make_search ();
        auto analyzed_decision = analyzed_search.make_decision ();

        auto start = std::chrono::system_clock::now ();

        SearchResult result = search (side, depth, -Initial_Alpha, Initial_Alpha, analyzed_decision);

        auto end = std::chrono::system_clock::now ();

        analyzed_decision.finalize (result);

        calc_time (my_output, nodes_visited, start, end);

        if (result.timed_out)
        {
            std::stringstream progress_str;
            progress_str << "Search timed out" << "\n";
            my_output.println (progress_str.str ());
            return result;
        }

        if (result.move.has_value())
        {
            Move best_move = *result.move;
            std::stringstream progress_str;
            progress_str << "move selected = " << to_string (best_move) << " [ score: "
                         << result.score << " ]\n";
            progress_str << "nodes visited = " << nodes_visited << ", cutoffs = " << cutoffs;
            my_output.println (progress_str.str ());
        }

        // principal variation could be null if search was interrupted
        if (result.variation_glimpse.size () > 0)
        {
            std::stringstream variation_str;
            variation_str << "principal variation: " << result.variation_glimpse.to_string ();
            my_output.println (variation_str.str ());
        }

        return result;
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

