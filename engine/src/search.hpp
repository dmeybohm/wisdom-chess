#ifndef WISDOM_CHESS_SEARCH_H_
#define WISDOM_CHESS_SEARCH_H_

#include "global.hpp"
#include "board.hpp"
#include "piece.hpp"
#include "move.hpp"
#include "move_tree.hpp"
#include "move_history.hpp"
#include "logger.hpp"
#include "move_timer.hpp"
#include "variation_glimpse.hpp"
#include "analytics.hpp"
#include "search_result.hpp"

namespace wisdom
{
    class Logger;

    class History;

    class MoveTimer;

    class Analytics;

    class AnalyzedDecision;

    class AnalyzedPosition;

    // Find the best move using default algorithm.
    std::optional<Move> find_best_move (Board &board, Color side, Logger &output, History &history);

    // Find the best move using multiple threads.
    std::optional<Move> find_best_move_multithreaded (Board &board, Color side, Logger &output, History &history);

    // Get the score for checkmate in X moves.
    int checkmate_score_in_moves (int moves);

    // Whether the score indicates a checkmate of the opponent has been found.
    bool is_checkmating_opponent_score (int score);

    class IterativeSearch
    {
    public:
        IterativeSearch (Board &board,
                         History &history,
                         Logger &output,
                         MoveTimer timer,
                         int total_depth) :
                my_board { board },
                my_history { history },
                my_output { output },
                my_analytics { make_dummy_analytics () },
                my_timer { timer },
                my_total_depth { total_depth }
        {}

        IterativeSearch (Board &board,
                         History &history,
                         Logger &output,
                         Analytics *analytics,
                         MoveTimer timer,
                         int total_depth) :
                my_board { board },
                my_history { history },
                my_output { output },
                my_analytics { analytics },
                my_timer { timer },
                my_total_depth { total_depth }
        {}

        SearchResult iteratively_deepen (Color side);

        SearchResult iterate (Color side, int depth);

        SearchResult search (Color side, int depth, int alpha, int beta,
                             AnalyzedDecision *parent);

    private:
        Board &my_board;
        History &my_history;
        Logger &my_output;
        std::unique_ptr<Analytics> my_analytics;
        MoveTimer my_timer;
        int my_total_depth;

        SearchResult search_moves (Color side, int depth, int alpha, int beta,
                                   const ScoredMoveList &moves,
                                   AnalyzedDecision *decision);

        SearchResult recurse_or_evaluate (Color side, int depth, int alpha, int beta,
                                          Move move, AnalyzedDecision *parent, AnalyzedPosition *position);
    };
}

#endif // WISDOM_CHESS_SEARCH_H_
