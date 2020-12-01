
#include <cstdio>
#include <cassert>
#include <ctime>
#include <iostream>
#include <chrono>

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

static int nodes_visited, cutoffs;

using system_clock_t = std::chrono::time_point<std::chrono::system_clock>;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;
using std::chrono::seconds;

void print_tree_recur (move_tree_t *tree)
{
	if (tree->parent)
		print_tree_recur (tree->parent);

	if (!is_null_move (tree->move))
	    std::cout << "[" << to_string(tree->move) << "] ";
}

void print_reverse_recur (move_tree_t *tree)
{
    std::cout << "[" << to_string(tree->move) << "] ";

	if (tree->parent)
		print_reverse_recur (tree->parent);
}

void print_reversed_tree (move_tree_t *tree)
{
	printf ("{ ");
	if (tree)
		print_reverse_recur (tree);
	printf("}\n");
}

search_result_t search (struct board &board, enum color side, int depth, int start_depth,
                        int alpha, int beta, unsigned long pseudo_rand,
                        move_tree_t **ret_variation, int no_quiesce, struct move_timer &timer,
                        move_history_t &move_history)
{
	move_tree_t  *   best_variation = nullptr, *new_variation = nullptr;
	board_check_t    board_check;
	size_t           illegal_move_count = 0;
    search_result_t  result;

	move_list_t moves = generate_moves (board, side);
	if (moves.empty())
	{
		*ret_variation = nullptr;
		result.score = evaluate (board, side, start_depth - depth);
		return result;
	}

	for (auto move : moves)
	{
		if (timer.is_triggered())
		{
			if (best_variation)
			{
				move_tree_destroy (new_variation);
				best_variation = nullptr;
			}

			break;
		}

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
			                   pseudo_rand, &new_variation,
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

			if (best_variation)
				move_tree_destroy (best_variation);

			best_variation = move_tree_new (new_variation, move);
		}
		else
		{
			move_tree_destroy (new_variation);
			new_variation = nullptr;
		}

		if (result.score > alpha)
			alpha = result.score;

		if (alpha >= beta)
		{
			cutoffs++;
			break;
		}
	}

	assert (ret_variation != nullptr);

	*ret_variation = best_variation;

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
	std::cout << "search took " << seconds << ", " << nodes / seconds << " nodes/sec\n";
}

move_t iterate (struct board &board, enum color side,
                move_history_t &move_history, struct move_timer &timer, int depth)
{
	move_tree_t   *principal_variation;

	printf ("finding moves for %s\n", (side == COLOR_WHITE) ? "white":"black");

	nodes_visited = 0;
	cutoffs = 0;

	auto start = std::chrono::system_clock::now();

	search_result_t result = search (board, side, depth, depth,
                         -INITIAL_ALPHA, INITIAL_ALPHA, 0,
                         &principal_variation, 0, timer, move_history);

    auto end = std::chrono::system_clock::now();

	calc_time (nodes_visited, start, end);

	if (!is_null_move (result.move))
	{
		printf ("move selected = %s [ score: %d ]\n", to_string(result.move).c_str(),
	            result.score);
		printf ("nodes visited = %d, cutoffs = %d\n", nodes_visited, cutoffs);
	}

	// principal variation could be null if search was interrupted
	if (principal_variation)
	{
		printf ("principal variation: ");
		print_reversed_tree (principal_variation);

		move_tree_destroy (principal_variation);
	}

	return result.move;
}

move_t find_best_move (struct board &board, enum color side, move_history_t &move_history)
{
    move_timer overdue_timer { MAX_SEARCH_SECONDS };

    multithread_search search { board, side, move_history, overdue_timer };
    search_result_t result = search.search();

    return result.move;
}

// Get the score for a checkmate discovered X moves away.
// Checkmates closer to the current position are more valuable than those
// further away.
int checkmate_score_in_moves (size_t moves)
{
    return INFINITE + INFINITE / (1 + moves);
}

// vi: set ts=4 sw=4:
