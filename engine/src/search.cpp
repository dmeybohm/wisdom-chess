#include "piece.hpp"
#include "board.hpp"
#include "evaluate.hpp"
#include "check.hpp"
#include "search.hpp"
#include "analytics.hpp"
#include "logger.hpp"

#include <sstream>
#include <utility>

namespace wisdom
{
    using system_clock_t = chrono::time_point<chrono::system_clock>;

    class IterativeSearchImpl
    {
    private:
        gsl::not_null<Board*> my_board;
        gsl::not_null<History*> my_history;
        gsl::not_null<const Logger*> my_output;
        gsl::not_null<analysis::Analytics*> my_analytics;
        MoveTimer my_timer;
        int my_total_depth;
        MoveGenerator my_generator {};

        optional<Move> my_best_move = nullopt;
        int my_best_depth = -1;
        int my_best_score = -1;
        bool my_timed_out = false;

        int my_nodes_visited = 0;
        int my_alpha_beta_cutoffs = 0;
        int my_total_nodes_visited = 0;
        int my_total_alpha_beta_cutoffs = 0;

    public:
        IterativeSearchImpl (Board& board, History& history, const Logger& output,
                             MoveTimer timer, int total_depth, analysis::Analytics& analytics) :
                my_board { &board },
                my_history { &history },
                my_output { &output },
                my_analytics { &analytics },
                my_timer { std::move( timer )},
                my_total_depth { total_depth }
        {
        }

        IterativeSearchImpl (Board& board, History& history, const Logger& output,
                             MoveTimer timer, int total_depth) :
                IterativeSearchImpl (board, history, output, std::move (timer), total_depth,
                                     analysis::make_dummy_analytics ())
        {
        }

        void iteratively_deepen (Color side);

        void search_moves (Color side, int depth, int alpha, int beta,
                           analysis::Decision& decision);

        void iterate (Color side, int depth, analysis::Iteration& iteration);

        void search (Color side, int depth, int alpha, int beta,
                     analysis::Decision& decision);

        [[nodiscard]] auto synthesize_result () const -> SearchResult;
    };

    IterativeSearch::~IterativeSearch() = default;
    IterativeSearch::IterativeSearch (Board& board, History& history, const Logger& output,
                                      MoveTimer timer, int total_depth,
                                      analysis::Analytics& analytics) :
            impl { make_unique<IterativeSearchImpl> (
                board, history, output, timer, total_depth, analytics) }
    {
    }

    IterativeSearch::IterativeSearch (Board& board, History& history, const Logger& output,
                                      MoveTimer timer, int total_depth) :
            IterativeSearch (board, history, output, timer, total_depth,
                             analysis::make_dummy_analytics ())
    {
    }

    SearchResult
    IterativeSearch::iteratively_deepen (Color side)
    {
         impl->iteratively_deepen (side);
         return impl->synthesize_result ();
    }


    void IterativeSearchImpl::search_moves (Color side, int depth, int alpha, int beta, // NOLINT(misc-no-recursion)
                                            analysis::Decision& decision)
    {
        std::optional<Move> best_move {};
        auto moves = my_generator.generate_all_potential_moves (*my_board, side);
        constexpr int No_Move_Seen_Score = -3 * Initial_Alpha;
        int best_score = No_Move_Seen_Score;

        for (auto move : moves)
        {
            if (my_timer.is_triggered ())
            {
                my_timed_out = true;
                return;
            }

            UndoMove undo_state = my_board->make_move (side, move);

            if (!was_legal_move (*my_board, side, move))
            {
                my_board->take_back (side, move, undo_state);
                continue;
            }

            auto position = decision.make_position (move);

            my_nodes_visited++;

            my_history->add_position_and_move (*my_board, move, undo_state);

            if (depth <= 0)
            {
                my_best_score = evaluate_and_check_draw (*my_board, side, my_total_depth - depth,
                                                         move, *my_history, my_generator);
            }
            else
            {
                auto new_decision = decision.make_child (position);

                search_moves (color_invert (side), depth - 1, -beta, -alpha, new_decision);

                my_best_score *= -1;
            }

            int score = my_best_score;

            position.finalize (score);

            if (score > best_score)
            {
                best_score = score;
                best_move = move;

                decision.select_position (position);
            }

            if (best_score > alpha)
                alpha = best_score;

            my_history->remove_position_and_last_move (*my_board);
            my_board->take_back (side, move, undo_state);

            if (my_timed_out)
                return;

            if (alpha >= beta)
            {
                my_alpha_beta_cutoffs++;
                break;
            }
        }

        auto result = SearchResult { {}, best_move, best_score,
                                     my_total_depth - depth, false };
        decision.finalize (result);

        my_best_move = result.move;
        if (result.score == No_Move_Seen_Score)
        {
            auto king_coord = my_board->get_king_position (side);
            result.score = is_king_threatened_not_inlined(*my_board, side, king_coord)
                ? -1 * checkmate_score_in_moves (result.depth)
                : 0;
        }
        my_best_score = result.score;
        my_best_depth = result.depth;
    }

    void IterativeSearchImpl::search (Color side, int depth,
                                      int alpha, int beta, analysis::Decision& decision)
    {
        search_moves (side, depth, alpha, beta, decision);

        if (my_timed_out)
            return;
        
        // if there are no legal moves, then the current computer_player is in a stalemate or checkmate position.
        if (!my_best_move.has_value ())
        {
            auto king_coord = my_board->get_king_position (side);

            my_best_score = is_king_threatened (*my_board, side, king_coord) ?
                    -1 * checkmate_score_in_moves (my_total_depth - depth) : 0;
        }
    }

    static void calc_time (const Logger& output, int nodes, system_clock_t start, system_clock_t end)
    {
        auto seconds_duration = chrono::duration<double> (end - start);
        auto seconds = seconds_duration.count();

        std::stringstream progress_str;
        progress_str << "search took " << seconds << "s, " << nodes / seconds << " nodes/sec";
        output.println (progress_str.str ());
    }

    void IterativeSearchImpl::iteratively_deepen (Color side)
    {
        SearchResult best_result = SearchResult::from_initial ();

        auto iterative_search_analytics = my_analytics->make_iterative_search (*my_board, side);

        try
        {
            // For now, only look every other depth
            for (int depth = 0; depth <= my_total_depth; depth == 0 ? depth++ : depth += 2)
            {
                std::ostringstream ostr;
                ostr << "Searching depth " << depth;
                my_output->println (ostr.str ());

                auto iteration_analysis = iterative_search_analytics.make_iteration (depth);
                iterate (side, depth, iteration_analysis);
                if (my_timed_out)
                    break;

                auto next_result = synthesize_result ();
                if (next_result.move.has_value ())
                {
                    best_result = next_result;
                    if (is_checkmating_opponent_score (next_result.score))
                        break;
                }
            }

            my_best_score = best_result.score;
            my_best_move = best_result.move;
            my_best_depth = best_result.depth;
            return;
        }
        catch (const Error &e)
        {
            std::cerr << "Uncaught error: " << e.message () << "\n";
            std::cerr << e.extra_info () << "\n";
            my_board->dump ();

            std::cerr << "History leading up to move: " << "\n";
            std::cerr << my_history->get_move_history ().to_string () << "\n";
            std::terminate ();
        }
    }

    [[nodiscard]] auto IterativeSearchImpl::synthesize_result () const -> SearchResult
    {
        return SearchResult { {}, my_best_move,
                              my_best_score, my_best_depth, my_timed_out };
    }

    void IterativeSearchImpl::iterate (Color side, int depth, analysis::Iteration& iteration)
    {
        std::stringstream outstr;
        outstr << "finding moves for " << to_string (side);
        my_output->println (outstr.str ());

        auto analyzed_search = iteration.make_search ();
        auto analyzed_decision = analyzed_search.make_decision ();

        my_nodes_visited = 0;
        my_alpha_beta_cutoffs = 0;

        auto start = std::chrono::system_clock::now ();

        search (side, depth, -Initial_Alpha, Initial_Alpha, analyzed_decision);

        auto end = std::chrono::system_clock::now ();

        auto result = synthesize_result ();

        analyzed_decision.finalize (result);

        calc_time (*my_output, my_nodes_visited, start, end);

        my_total_nodes_visited += my_nodes_visited;
        my_total_alpha_beta_cutoffs += my_alpha_beta_cutoffs;

        {
            std::stringstream progress_str;
            progress_str << "nodes visited = " << my_nodes_visited << ", alpha-beta cutoffs = " << my_alpha_beta_cutoffs;
            my_output->println (progress_str.str ());
        }

        if (result.timed_out)
        {
            std::stringstream progress_str;
            progress_str << "Search timed out" << "\n";
            my_output->println (progress_str.str ());
            return;
        }

        if (result.move.has_value ())
        {
            Move best_move = *result.move;
            std::stringstream progress_str;
            progress_str << "move selected = " << to_string (best_move) << " [ score: "
                         << result.score << " ]\n";
            my_output->println (progress_str.str ());
        }

        // principal variation could be null if search was interrupted
        if (result.variation_glimpse.size () > 0)
        {
            std::stringstream variation_str;
            variation_str << "principal variation: " << result.variation_glimpse.to_string ();
            my_output->println (variation_str.str ());
        }
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

