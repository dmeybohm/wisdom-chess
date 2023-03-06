#include "piece.hpp"
#include "board.hpp"
#include "evaluate.hpp"
#include "check.hpp"
#include "search.hpp"
#include "logger.hpp"

#include <sstream>
#include <utility>

namespace wisdom
{
    using system_clock_t = chrono::time_point<chrono::system_clock>;

    class IterativeSearchImpl
    {
    private:
        Board my_original_board;
        observer_ptr<History> my_history;
        not_null<observer_ptr<const Logger>> my_output;
        MoveGenerator my_generator {};
        MoveTimer my_timer;
        int my_total_depth;
        int my_search_depth {};

        optional<Move> my_best_move = nullopt;
        int my_best_depth = -1;
        int my_best_score = -1;
        bool my_timed_out = false;

        int my_nodes_visited = 0;
        int my_alpha_beta_cutoffs = 0;
        int my_total_nodes_visited = 0;
        int my_total_alpha_beta_cutoffs = 0;
        Color my_searching_color = Color::None;

    public:
        IterativeSearchImpl (const Board& board, History& history, const Logger& output,
                             MoveTimer timer, int total_depth) :
                my_original_board { Board { board } },
                my_history { &history },
                my_output { &output },
                my_timer { std::move (timer) },
                my_total_depth { total_depth }
        {
        }

        void iteratively_deepen (Color side);

        void iterate (Color side, int depth);

        void search (const Board& parent_board, Color side, int depth, int alpha, int beta);

        [[nodiscard]] auto synthesize_result () const -> SearchResult;

        [[nodiscard]] auto move_timer () const -> not_null<const MoveTimer*>
        {
            return &my_timer;
        }
    };

    IterativeSearch::~IterativeSearch() = default;
    IterativeSearch::IterativeSearch (const Board& board, History& history, const Logger& output,
                                      MoveTimer timer, int total_depth)
            : impl { make_unique<IterativeSearchImpl> (
                Board { board }, history, output, std::move (timer), total_depth) }
    {
    }

    SearchResult
    IterativeSearch::iteratively_deepen (Color side)
    {
         impl->iteratively_deepen (side);
         return impl->synthesize_result ();
    }

    auto IterativeSearch::is_cancelled () -> bool
    {
        return impl->move_timer()->is_cancelled();
    }

    static constexpr auto drawing_score (Color searching_color, Color current_color)
    {
        //
        // For the player looking for a move (the chess engine), a draw is considered
        // less preferable because it is more boring.
        //
        // When considering its opponent has a draw, consider it neutral.
        //
        return current_color == searching_color ? Min_Draw_Score : 0;
    }

    void IterativeSearchImpl::search (const Board& parent_board, // NOLINT(misc-no-recursion)
                                      Color side, int depth, int alpha, int beta)
    {
        std::optional<Move> best_move {};
        int best_score = -Initial_Alpha;

        auto moves = my_generator.generate_all_potential_moves (parent_board, side);
        for (auto move : moves)
        {
            if (my_timer.is_triggered ())
            {
                my_timed_out = true;
                return;
            }

            Board child_board = parent_board.with_move (side, move);

            if (!is_legal_position_after_move (child_board, side, move))
                continue;

            my_nodes_visited++;

            my_history->add_position_and_move (&child_board, move);

            if (depth <= 0)
            {
                if (is_drawing_move (child_board, side, move, *my_history))
                {
                    my_best_score = drawing_score (my_searching_color, side);
                }
                else
                {
                    my_best_score = evaluate (child_board, side,
                            my_search_depth - depth, my_generator);
                }
            }
            else
            {
                // Don't recurse into a big search if this move is a draw.
                if (my_search_depth == depth && is_drawing_move (child_board, side, move, *my_history))
                {
                    my_best_score = drawing_score (my_searching_color, side);
                }
                else
                {
                    search (child_board, color_invert (side), depth-1, -beta, -alpha);
                    my_best_score *= -1;
                }
            }

            int score = my_best_score;

            if (score > best_score)
            {
                best_score = score;
                best_move = move;
            }

            if (best_score > alpha)
                alpha = best_score;

            my_history->remove_last_position ();

            if (my_timed_out)
                return;

            if (alpha >= beta)
            {
                my_alpha_beta_cutoffs++;
                break;
            }
        }

        auto result = SearchResult { best_move, best_score,
                                     my_search_depth - depth, false };

        my_best_move = result.move;
        if (!my_best_move.has_value ())
        {
            // if there are no legal moves, then the current player is in a stalemate or checkmate position.
            result.score = evaluate_without_legal_moves (parent_board, side, result.depth);
        }
        my_best_score = result.score;
        my_best_depth = result.depth;
    }

    static void calc_time (const Logger& output, int nodes, system_clock_t start, system_clock_t end)
    {
        auto seconds_duration = chrono::duration<double> (end - start);
        auto seconds = seconds_duration.count();

        std::stringstream progress_str;
        progress_str << "search took " << seconds << "s, " << nodes / seconds << " nodes/sec";
        output.info (progress_str.str ());
    }

    void IterativeSearchImpl::iteratively_deepen (Color side)
    {
        SearchResult best_result = SearchResult::from_initial ();
        my_searching_color = side;

        try
        {
            // For now, only look every other depth
            for (int depth = 0; depth <= my_total_depth; depth == 0 ? depth++ : depth += 2)
            {
                std::ostringstream ostr;
                ostr << "Searching depth " << depth;
                my_output->info (ostr.str ());

                iterate (side, depth);
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
            my_original_board.dump ();

            std::cerr << "History leading up to move: " << "\n";
            std::cerr << my_history->get_move_history ().to_string () << "\n";
            std::terminate ();
        }
    }

    [[nodiscard]] auto IterativeSearchImpl::synthesize_result () const -> SearchResult
    {
        return SearchResult { my_best_move,
              my_best_score, my_best_depth, my_timed_out };
    }

    void IterativeSearchImpl::iterate (Color side, int depth)
    {
        std::stringstream outstr;
        outstr << "finding moves for " << to_string (side);
        my_output->debug (outstr.str ());

        my_nodes_visited = 0;
        my_alpha_beta_cutoffs = 0;

        auto start = std::chrono::system_clock::now ();

        my_search_depth = depth;
        search (my_original_board, side, depth, -Initial_Alpha, Initial_Alpha);

        auto end = std::chrono::system_clock::now ();

        auto result = synthesize_result ();

        calc_time (*my_output, my_nodes_visited, start, end);

        my_total_nodes_visited += my_nodes_visited;
        my_total_alpha_beta_cutoffs += my_alpha_beta_cutoffs;

        {
            std::stringstream progress_str;
            progress_str << "nodes visited = " << my_nodes_visited << ", alpha-beta cutoffs = " << my_alpha_beta_cutoffs;
            my_output->debug (progress_str.str ());
        }

        if (result.timed_out)
        {
            std::stringstream progress_str;
            progress_str << "Search timed out" << "\n";
            my_output->info (progress_str.str ());
            return;
        }

        if (result.move.has_value ())
        {
            Move best_move = *result.move;
            std::stringstream progress_str;
            progress_str << "move selected = " << to_string (best_move) << " [ score: "
                         << result.score << " ]\n";
            my_output->info (progress_str.str ());
        }
    }
}
