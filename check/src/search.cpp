
#include <cstdio>
#include <cassert>
#include <ctime>
#include <iostream>
#include <sstream>
#include <chrono>
#include <memory>

#include "piece.h"
#include "board.h"
#include "generate.h"
#include "evaluate.h"
#include "check.h"
#include "move_tree.h"
#include "search.h"
#include "move_timer.h"
#include "board_check.h"
#include "move_history.hpp"
#include "multithread_search.h"

enum {
    MAX_DEPTH = 16,
    MAX_SEARCH_SECONDS = 10,
};

thread_local int nodes_visited, cutoffs;

using system_clock_t = std::chrono::time_point<std::chrono::system_clock>;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;
using std::chrono::seconds;

search_result_t search (struct board &board, Color side, int depth, int start_depth,
                        int alpha, int beta, unsigned long pseudo_rand,
                        std::unique_ptr<move_tree_t> &best_variation, int no_quiesce, struct move_timer &timer,
                        move_history_t &move_history)
{
    std::unique_ptr<move_tree_t> new_variation { nullptr };
	board_check_t                board_check;
	size_t                       illegal_move_count = 0;
    search_result_t              result;

    best_variation.reset (nullptr);
	move_list_t moves = generate_moves (board, side);

	if (moves.empty())
	{
		result.score = evaluate (board, side, start_depth - depth);
		return result;
	}

	for (auto move : moves)
	{
		if (timer.is_triggered())
			break;

		board_check_init (&board_check, board);
        undo_move_t undo_state = do_move (board, side, move);

		if (!was_legal_move (board, side, move))
		{
		    illegal_move_count++;
            undo_move (board, side, move, undo_state);
            board_check_validate (&board_check, board, side, move);
			continue;
		}

		nodes_visited++;

		move_history.push_back (move);
		search_result_t other_search_result { .move = move };
		if (depth <= 0)
		{

#if 0
			if (!no_quiesce)
				score = quiesce (board, side, alpha, beta, 0, new_leaf);
			else
#endif

				other_search_result.score = evaluate_and_check_draw (board, side, start_depth - depth,
                                     move, move_history);

		}
		else
		{
			other_search_result = search (board, color_invert (side),
                               depth - 1, start_depth, -beta, -alpha,
			                   pseudo_rand, new_variation,
			                   no_quiesce, timer, move_history);
			other_search_result.score *= -1;
		}

        undo_move (board, side, move, undo_state);
        board_check_validate (&board_check, board, side, move);

		move_history.pop_back ();

		if (other_search_result.score > result.score || result.score == -INITIAL_ALPHA)
		{
			result.score = other_search_result.score;
			result.move = move;

			if (new_variation == nullptr)
			    new_variation = std::make_unique<move_tree_t>();

			new_variation->push_front (move);
			best_variation = std::move (new_variation);
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

static void calc_time (int nodes, system_clock_t start, system_clock_t end)
{
    auto duration = end - start;
    milliseconds ms = std::chrono::duration_cast<milliseconds>(duration);
    double seconds = ms.count() / 1000.0;

    std::stringstream progress_str;
	progress_str << "search took " << seconds << ", " << nodes / seconds << " nodes/sec\n";
	std::cout << progress_str.str();
}

move_t iterate (struct board &board, Color side,
                move_history_t &move_history, struct move_timer &timer, int depth)
{
	std::unique_ptr<move_tree_t>   principal_variation;

	std::stringstream outstr;
	outstr << "finding moves for " << to_string(side) << "\n";
	std::cout << outstr.str();

	nodes_visited = 0;
	cutoffs = 0;

	auto start = std::chrono::system_clock::now();

	search_result_t result = search (board, side, depth, depth,
                         -INITIAL_ALPHA, INITIAL_ALPHA, 0,
                         principal_variation, 0, timer, move_history);

    auto end = std::chrono::system_clock::now();

	calc_time (nodes_visited, start, end);

	if (!is_null_move (result.move))
	{
	    std::stringstream progress_str;
	    progress_str << "move selected = " << to_string(result.move) << " [ score: "
	        << result.score << " ]\n";
	    progress_str << "nodes visited = " << nodes_visited << ", cutoffs = " << cutoffs << "\n";
	    std::cout << progress_str.str();
	}

	// principal variation could be null if search was interrupted
	if (principal_variation != nullptr)
    {
	    std::stringstream variation_str;
		variation_str << "principal variation: " << principal_variation->to_string() << "\n";
		std::cout << variation_str.str();
    }

	return result.move;
}

move_t find_best_move (struct board &board, Color side, move_history_t &move_history)
{
    move_timer overdue_timer { MAX_SEARCH_SECONDS };

    multithread_search search { board, side, move_history, overdue_timer };
    search_result_t result = search.search();

    return result.move;
}

// Get the score for a checkmate discovered X moves away.
// Checkmates closer to the current position are more valuable than those
// further away.
int checkmate_score_in_moves (int moves)
{
    return INFINITE + INFINITE / (1 + moves);
}
