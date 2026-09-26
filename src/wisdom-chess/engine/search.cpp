#include <sstream>
#include <utility>

#include "wisdom-chess/engine/piece.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/search.hpp"
#include "wisdom-chess/engine/logger.hpp"
#include "wisdom-chess/engine/transposition_table.hpp"

namespace wisdom
{
    using SteadyClockTime = chrono::time_point<chrono::steady_clock>;

    // Quiescence plies past which a side in check is scored instead of
    // searching its evasions, so a series of checks cannot run on.
    static constexpr int Max_Quiescence_Evasion_Ply = 4;

    class IterativeSearchImpl
    {
    public:
        IterativeSearchImpl (
            const Board& board,
            const History& history,
            shared_ptr<Logger> output,
            MoveTimer timer,
            int total_depth,
            TranspositionTable& transposition_table
        )
            : my_original_board { Board { board } }
            , my_history { History { history } }
            , my_timer { std::move (timer) }
            , my_output { std::move (output) }
            , my_transposition_table { transposition_table }
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
                int alpha, int beta, int ply)
            -> int;

        // Search captures until the position is quiet, and return the best score.
        auto
        quiesce (const Board& board, Color side, int alpha, int beta, int ply,
                 int quiescence_ply)
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
        TranspositionTable& my_transposition_table;

        int my_total_depth;
        int my_nodes_visited = 0;
        int my_alpha_beta_cutoffs = 0;
        int64_t my_total_nodes_visited = 0;
        int my_total_alpha_beta_cutoffs = 0;
        int64_t my_quiescence_nodes_visited = 0;
        int64_t my_total_quiescence_nodes_visited = 0;
        Color my_searching_color = Color::None;

        // Counts the nodes that ended in a draw score. A node compares this
        // against the value it saw before searching its children, to find out
        // whether its own score depends on the path that reached it.
        size_t my_draw_nodes = 0;
    };

    IterativeSearch::~IterativeSearch() = default;

    // Private constructor for factory functions
    IterativeSearch::IterativeSearch (unique_ptr<IterativeSearchImpl> impl)
        : impl { std::move (impl) }
    {
    }

    // Factory function implementation
    auto IterativeSearch::create (
        const Board& board,
        const History& history,
        shared_ptr<Logger> logger,
        const MoveTimer& timer,
        int max_depth,
        TranspositionTable& transposition_table
    ) -> IterativeSearch
    {
        return IterativeSearch {
            make_unique<IterativeSearchImpl> (
                Board { board },
                history,
                std::move (logger),
                timer,
                max_depth,
                transposition_table
            )
        };
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
        int beta,
        int ply
    )
        -> int
    {
        if (isProbablyDrawingMove (parent_board, my_history))
        {
            my_draw_nodes++;
            return drawingScore (my_searching_color, side);
        }

        if (depth <= 0)
        {
            return quiesce (parent_board, side, alpha, beta, ply, 0);
        }

        int original_alpha = alpha;
        std::optional<Move> best_move {};
        int best_score = -Initial_Alpha;
        auto draw_nodes_before = my_draw_nodes;

        auto hash = parent_board.getCode().getHashCode();

        if (ply > 0)
        {
            if (auto tt_score = my_transposition_table.probe (hash, depth, alpha, beta, ply))
                return *tt_score;
        }

        auto tt_move = my_transposition_table.getBestMove (hash);
        auto moves = generateAllPotentialMoves (parent_board, side, tt_move);

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

            score = -1 * search (child_board, colorInvert (side), depth - 1, -beta, -alpha, ply + 1);

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

        if (!best_move.has_value())
        {
            // if there are no legal moves, then the current player is in a
            // stalemate or checkmate position.
            best_score = evaluateWithoutLegalMoves (parent_board, side, ply);
        }

        if (ply == 0)
        {
            my_current_result.move = best_move;
            my_current_result.score = best_score;
            my_current_result.depth = depth;
        }

        // A score that any node below this one derived from a repetition or
        // the fifty-move counter belongs to the path, not to the board, and
        // the table is keyed by the board alone. It must not be stored, even
        // when the drawing line lost: under another history that line may be
        // worth more than the one chosen here, which would make this score
        // wrong rather than merely pessimistic.
        bool score_depends_on_path = my_draw_nodes != draw_nodes_before;

        if (!my_current_result.timed_out && !score_depends_on_path)
        {
            BoundType bound_type = (best_score <= original_alpha) ? BoundType::UpperBound
                                 : (best_score >= beta) ? BoundType::LowerBound
                                 : BoundType::Exact;
            my_transposition_table.store (
                hash,
                best_score,
                depth,
                bound_type,
                best_move.value_or (Move {}),
                ply
            );
        }

        return best_score;
    }

    auto
    IterativeSearchImpl::quiesce ( // NOLINT(misc-no-recursion)
        const Board& board,
        Color side,
        int alpha,
        int beta,
        int ply,
        int quiescence_ply
    )
        -> int
    {
        // The main search has already checked the first node for a draw.
        if (quiescence_ply > 0 && isProbablyDrawingMove (board, my_history))
        {
            my_draw_nodes++;
            return drawingScore (my_searching_color, side);
        }

        bool in_check = isKingThreatened (board, side, board.getKingPosition (side));
        int best_score = -Initial_Alpha;
        MoveList moves;

        if (in_check)
        {
            if (quiescence_ply >= Max_Quiescence_Evasion_Ply)
            {
                return hasLegalMove (board)
                    ? evaluateWithoutMateTest (board, side)
                    : evaluateWithoutLegalMoves (board, side, ply);
            }

            moves = generateAllPotentialMoves (board, side);
        }
        else
        {
            best_score = evaluateWithoutMateTest (board, side);
            if (best_score >= beta)
                return best_score;
            if (best_score > alpha)
                alpha = best_score;

            moves = generateCaptures (board, side);
        }

        bool found_legal_move = false;

        for (auto move : moves)
        {
            if (my_timer.isTriggered())
            {
                my_current_result.timed_out = true;
                return -Initial_Alpha;
            }

            Board child_board = board.withMove (side, move);

            if (!isLegalPositionAfterMove (child_board, side, move))
                continue;

            found_legal_move = true;
            my_quiescence_nodes_visited++;

            my_history.addTentativePosition (child_board);

            int score = -1 * quiesce (child_board, colorInvert (side), -beta, -alpha,
                                      ply + 1, quiescence_ply + 1);

            my_history.removeLastTentativePosition();

            if (my_current_result.timed_out)
                return -Initial_Alpha;

            if (score > best_score)
                best_score = score;

            if (best_score > alpha)
                alpha = best_score;

            if (alpha >= beta)
                break;
        }

        if (in_check && !found_legal_move)
            best_score = evaluateWithoutLegalMoves (board, side, ply);

        return best_score;
    }

    static void
    logSearchTime (
        const Logger& output, 
        int nodes, 
        SteadyClockTime start, 
        SteadyClockTime end
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

            for (int depth = 1; depth <= my_total_depth; depth++)
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

            best_result.nodes = my_total_nodes_visited + my_total_quiescence_nodes_visited;
            best_result.quiescence_nodes = my_total_quiescence_nodes_visited;
            return best_result;
        }
        catch (const Error& e)
        {
            throw SearchError {
                e.message(),
                e.extra_info() + "\n" + my_original_board.asString()
            };
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
        my_quiescence_nodes_visited = 0;

        auto tt_stats_start = my_transposition_table.getStats();
        auto start = std::chrono::steady_clock::now();

        my_current_result = SearchResult {};
        search (my_original_board, side, depth, -Initial_Alpha, Initial_Alpha, 0);

        auto end = std::chrono::steady_clock::now();

        auto result = getBestResult();

        logSearchTime (*my_output, my_nodes_visited, start, end);

        my_total_nodes_visited += my_nodes_visited;
        my_total_alpha_beta_cutoffs += my_alpha_beta_cutoffs;
        my_total_quiescence_nodes_visited += my_quiescence_nodes_visited;

        {
            std::stringstream progress_str;
            progress_str << "nodes visited = " << my_nodes_visited
                         << ", quiescence nodes = " << my_quiescence_nodes_visited
                         << ", alpha-beta cutoffs = " << my_alpha_beta_cutoffs << "\n";
            auto tt_stats_end = my_transposition_table.getStats();
            auto hit_rate = computeHitRate (tt_stats_start, tt_stats_end);
            auto probes_this_iteration = tt_stats_end.probes - tt_stats_start.probes;
            auto hits_this_iteration = tt_stats_end.hits - tt_stats_start.hits;
            progress_str << "transposition table: entries = " << tt_stats_end.stored_entries
                << "/" << my_transposition_table.getSize()
                << ", probes = " << probes_this_iteration
                << ", hits = "  << hits_this_iteration
                << ", hit rate = " << hit_rate << "%";

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
