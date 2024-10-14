#include <sstream>
#include <utility>
#include <iostream>

#include "wisdom-chess/engine/piece.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/search.hpp"
#include "wisdom-chess/engine/logger.hpp"

namespace wisdom
{
    using SystemClockTime = chrono::time_point<chrono::system_clock>;

    class IterativeSearchImpl
    {
    public:
        IterativeSearchImpl (
            const Board& board,
            const History& history,
            shared_ptr<Logger> output,
            MoveTimer timer,
            int total_depth
        )
            : my_original_board { Board { board } }
            , my_history { History { history } }
            , my_output { std::move (output) }
            , my_timer { std::move (timer) }
            , my_total_depth { total_depth }
        {
        }

        [[nodiscard]] auto
        iterativelyDeepen (Color side)
            -> SearchResult;

        auto
        iterate (Color side, int depth)
            -> SearchResult;

        // Search for the best move, and return the best score.
        auto
        search (const Board& parent_board, Color side, int depth,
                int alpha, int beta)
            -> int;

        // Get the best result the search found.
        [[nodiscard]] auto
        getBestResult() const
            -> SearchResult;

        [[nodiscard]] auto
        moveTimer() const&
            -> const MoveTimer&
        {
            return my_timer;
        }

    private:
        Board my_original_board;
        History my_history;
        SearchResult my_current_result {};
        MoveTimer my_timer;
        shared_ptr<Logger> my_output;

        int my_total_depth;
        int my_search_depth {};
        int my_nodes_visited = 0;
        int my_alpha_beta_cutoffs = 0;
        int my_total_nodes_visited = 0;
        int my_total_alpha_beta_cutoffs = 0;
        Color my_searching_color = Color::None;
    };

    IterativeSearch::~IterativeSearch() = default;

    IterativeSearch::IterativeSearch (
        const Board& board,
        const History& history,
        shared_ptr<Logger> output,
        const MoveTimer& timer,
        int total_depth
    ) : impl {
            make_unique<IterativeSearchImpl> (
                Board { board },
                  history,
                  std::move (output),
                  timer,
                  total_depth
            )
        }
    {
    }

    auto 
    IterativeSearch::iterativelyDeepen (Color side)
        -> SearchResult
    {
        return impl->iterativelyDeepen (side);
    }

    auto 
    IterativeSearch::isCancelled() 
        -> bool
    {
        return impl->moveTimer().isCancelled();
    }

    auto 
    IterativeSearch::moveTimer() const& 
        -> const MoveTimer&
    {
        return impl->moveTimer();
    }

    static constexpr auto 
    drawingScore (Color searching_color, Color current_color)
        -> int
    {
        //
        // For the player looking for a move (the chess engine), a draw is considered
        // less preferable because it is more boring.
        //
        // When considering its opponent has a draw, consider it neutral.
        //
        return current_color == searching_color ? Min_Draw_Score : 0;
    }

    auto 
    IterativeSearchImpl::search ( // NOLINT(misc-no-recursion)
        const Board& parent_board, 
        Color side, 
        int depth, 
        int alpha, 
        int beta
    )
        -> int
    {
        std::optional<Move> best_move {};
        int best_score = -Initial_Alpha;

        auto moves = generateAllPotentialMoves (parent_board, side);
        for (auto move : moves)
        {
            int score;

            if (my_timer.isTriggered())
            {
                my_current_result.timed_out = true;
                return -Initial_Alpha;
            }

            Board child_board = parent_board.withMove (side, move);

            if (!isLegalPositionAfterMove (child_board, side, move))
                continue;

            my_nodes_visited++;

            my_history.addTentativePosition (child_board);

            if (depth <= 0)
            {
                if (isProbablyDrawingMove (child_board, side, move, my_history))
                {
                    score = drawingScore (my_searching_color, side);
                }
                else
                {
                    score = evaluate (child_board, side, my_search_depth - depth);
                }
            }
            else
            {
                // Don't recurse into a big search if this move is a draw.
                if (my_search_depth == depth
                    && isProbablyDrawingMove (child_board, side, move, my_history))
                {
                    score = drawingScore (my_searching_color, side);
                }
                else
                {
                    score = -1 * search (child_board, colorInvert (side), depth - 1, -beta, -alpha);
                }
            }

            if (score > best_score)
            {
                best_score = score;
                best_move = move;
            }

            if (best_score > alpha)
                alpha = best_score;

            my_history.removeLastTentativePosition();

            if (my_current_result.timed_out)
                return -Initial_Alpha;

            if (alpha >= beta)
            {
                my_alpha_beta_cutoffs++;
                break;
            }
        }

        my_current_result.depth = my_search_depth - depth;
        if (!best_move.has_value())
        {
            // if there are no legal moves, then the current player is in a
            // stalemate or checkmate position.
            best_score = evaluateWithoutLegalMoves (parent_board, side, my_current_result.depth);
        }
        my_current_result.move = best_move;
        my_current_result.score = best_score;
        return best_score;
    }

    static void
    logSearchTime (
        const Logger& output, 
        int nodes, 
        SystemClockTime start, 
        SystemClockTime end
    ) {
        auto seconds_duration = chrono::duration<double> (end - start);
        auto seconds = seconds_duration.count();
        auto rate = nodes / std::max (0.000000001, seconds);

        std::stringstream progress_str;
        progress_str << "search took " << seconds << "s, " << rate << " nodes/sec";
        output.info (std::move (progress_str).str());
    }

    auto 
    IterativeSearchImpl::iterativelyDeepen (Color side) 
        -> SearchResult
    {
        SearchResult best_result {};
        my_searching_color = side;

        try
        {
            my_timer.start();

            // For now, only look every other depth
            for (int depth = 0; depth <= my_total_depth; depth == 0 ? depth++ : depth += 2)
            {
                std::ostringstream ostr;
                ostr << "Searching depth " << depth;
                my_output->info (std::move (ostr).str());

                iterate (side, depth);
                if (my_current_result.timed_out)
                    break;

                auto next_result = getBestResult();
                if (next_result.move.has_value())
                {
                    best_result = next_result;
                    if (isCheckmatingOpponentScore (next_result.score))
                        break;
                }
            }

            return best_result;
        }
        catch (const Error& e)
        {
            std::cerr << "Uncaught error: " << e.message() << "\n";
            std::cerr << e.extra_info() << "\n";
            my_original_board.dump();
            std::terminate();
        }
    }

    [[nodiscard]] auto 
    IterativeSearchImpl::getBestResult() const 
        -> SearchResult
    {
        return my_current_result;
    }

    auto 
    IterativeSearchImpl::iterate (Color side, int depth) 
        -> SearchResult
    {
        std::stringstream outstr;
        outstr << "finding moves for " << asString (side);
        my_output->debug (std::move (outstr).str());

        my_nodes_visited = 0;
        my_alpha_beta_cutoffs = 0;

        auto start = std::chrono::system_clock::now();

        my_search_depth = depth;
        my_current_result = SearchResult {};
        search (my_original_board, side, depth, -Initial_Alpha, Initial_Alpha);

        auto end = std::chrono::system_clock::now();

        auto result = getBestResult();

        logSearchTime (*my_output, my_nodes_visited, start, end);

        my_total_nodes_visited += my_nodes_visited;
        my_total_alpha_beta_cutoffs += my_alpha_beta_cutoffs;

        {
            std::stringstream progress_str;
            progress_str << "nodes visited = " << my_nodes_visited
                         << ", alpha-beta cutoffs = " << my_alpha_beta_cutoffs;
            my_output->debug (std::move (progress_str).str());
        }

        if (result.timed_out)
        {
            std::stringstream progress_str;
            progress_str << "Search timed out"
                         << "\n";
            my_output->info (std::move (progress_str).str());
        }
        else if (result.move.has_value())
        {
            Move best_move = *result.move;
            std::stringstream progress_str;
            progress_str << "move selected = " << asString (best_move)
                         << " [ score: " << result.score << " ]\n";
            my_output->info (std::move (progress_str).str());
        }

        return result;
    }
}
