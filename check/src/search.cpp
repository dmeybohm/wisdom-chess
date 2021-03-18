#include <iostream>
#include <sstream>
#include <memory>

#include "piece.h"
#include "board.h"
#include "generate.h"
#include "evaluate.h"
#include "check.h"
#include "move_tree.h"
#include "search.h"
#include "move_timer.h"
#include "move_history.hpp"
#include "multithread_search.h"
#include "output.hpp"
#include "history.hpp"

enum
{
    Max_Depth = 16,
    Max_Search_Seconds = 10,
};

thread_local int nodes_visited, cutoffs;

using system_clock_t = std::chrono::time_point<std::chrono::system_clock>;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;
using std::chrono::seconds;
using wisdom::Output;

SearchResult
search (Board &board, Color side, Output &output, History &history, MoveTimer &timer,
        int depth, int start_depth, int alpha, int beta,
        std::unique_ptr<MoveTree> &variation)
{
    std::unique_ptr<MoveTree> new_variation {nullptr };
	size_t                       illegal_move_count = 0;
    SearchResult              result;

    variation.reset (nullptr);
	MoveList moves = generate_moves (board, side);

	if (moves.empty())
	{
		result.score = evaluate (board, side, start_depth - depth);
		return result;
	}

	for (auto move : moves)
	{
		if (timer.is_triggered())
			break;

        UndoMove undo_state = do_move (board, side, move);

		if (!was_legal_move (board, side, move))
		{
		    illegal_move_count++;
            undo_move (board, side, move, undo_state);
			continue;
		}

		nodes_visited++;

        history.add_position_and_move (board, move);
		SearchResult other_search_result { .move = move };
		if (depth <= 0)
		{
            other_search_result.score = evaluate_and_check_draw (board, side, start_depth - depth,
                                                                 move, history);
		}
		else
		{
			other_search_result = search (board, color_invert (side),
                                          output, history, timer,
                                          depth - 1, start_depth, -beta, -alpha, new_variation);
			other_search_result.score *= -1;
		}

        history.remove_position_and_last_move (board);
        undo_move (board, side, move, undo_state);

		if (other_search_result.score > result.score || result.score == -INITIAL_ALPHA)
		{
			result.score = other_search_result.score;
			result.move = move;

			if (new_variation == nullptr)
			    new_variation = std::make_unique<MoveTree>();

			new_variation->push_front (move);
            variation = std::move (new_variation);
			new_variation.reset( nullptr );
		}
		else
		{
			new_variation.reset (nullptr);
		}

		if (result.score > alpha)
			alpha = result.score;

		if (alpha >= beta)
		{
			cutoffs++;
			break;
		}
	}

	// if there are no legal moves, then the current player is in a stalemate or checkmate position.
	if (moves.size() == illegal_move_count)
    {
	    auto [my_king_row, my_king_col] = king_position (board, side);
        result.score = is_king_threatened (board, side, my_king_row, my_king_col) ?
                -1 * checkmate_score_in_moves (start_depth - depth) : 0;
    }

	if (timer.is_triggered())
        result.move = null_move;

    return result;
}

static void calc_time (Output &output, int nodes, system_clock_t start, system_clock_t end)
{
    auto duration = end - start;
    milliseconds ms = std::chrono::duration_cast<milliseconds>(duration);
    double seconds = ms.count() / 1000.0;

    std::stringstream progress_str;
	progress_str << "search took " << seconds << ", " << nodes / seconds << " nodes/sec";
    output.println (progress_str.str ());
}

Move iterate (Board &board, Color side, Output &output,
              History &history, MoveTimer &timer, int depth)
{
	std::unique_ptr<MoveTree> principal_variation;

	std::stringstream outstr;
	outstr << "finding moves for " << to_string(side);
    output.println (outstr.str ());

	nodes_visited = 0;
	cutoffs = 0;

	auto start = std::chrono::system_clock::now();

	SearchResult result = search (board, side, output, history, timer,
                                  depth, depth, -INITIAL_ALPHA, INITIAL_ALPHA,
                                  principal_variation);

    auto end = std::chrono::system_clock::now();

	calc_time (output, nodes_visited, start, end);

	if (!is_null_move (result.move))
	{
	    std::stringstream progress_str;
	    progress_str << "move selected = " << to_string(result.move) << " [ score: "
	        << result.score << " ]\n";
	    progress_str << "nodes visited = " << nodes_visited << ", cutoffs = " << cutoffs;
        output.println (progress_str.str ());
	}

	// principal variation could be null if search was interrupted
	if (principal_variation != nullptr)
    {
	    std::stringstream variation_str;
		variation_str << "principal variation: " << principal_variation->to_string();
        output.println (variation_str.str ());
    }

	return result.move;
}

Move find_best_move (Board &board, Color side, Output &output, History &history)
{
    MoveTimer overdue_timer {Max_Search_Seconds };

    MultithreadSearch search {board, side, output, history, overdue_timer };
    SearchResult result = search.search();

    return result.move;
}

// Get the score for a checkmate discovered X moves away.
// Checkmates closer to the current position are more valuable than those
// further away.
int checkmate_score_in_moves (int moves)
{
    return INFINITE + INFINITE / (1 + moves);
}
