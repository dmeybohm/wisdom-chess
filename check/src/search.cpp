#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <assert.h>
#include <sys/time.h>

#include "piece.h"
#include "board.h"
#include "generate.h"
#include "evaluate.h"
#include "check.h"
#include "move_tree.h"
#include "debug.h"
#include "search.h"
#include "timer.h"
#include "board_check.h"

#define MAX_DEPTH               16

#define MAX_SEARCH_SECONDS      10.0    /* seconds */

static int nodes_visited, cutoffs;

void print_tree_recur (move_tree_t *tree)
{
	if (tree->parent)
		print_tree_recur (tree->parent);

	if (!is_null_move (tree->move))
		printf ("[%s] ", move_str (tree->move));
}

#if 0
static void print_tree (move_tree_t *tree)
{
	printf ("{ ");
	print_tree_recur (tree);
#if 0
	printf("} best move: [%s] score: %d\n", move_str (*best), best_score);
#endif
	printf("}\n");
}
#endif

void print_reverse_recur (move_tree_t *tree)
{
	printf ("[%s] ", move_str (tree->move));

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

int search (struct board *board, enum color side, int depth, int start_depth,
            move_t *ret, int alpha, int beta, unsigned long pseudo_rand,
            move_tree_t **ret_variation, int no_quiesce, struct timer *timer,
			move_tree_t *history) 
{
	int           score;
	int           best   = -INITIAL_ALPHA; // make sure to select something
	move_tree_t  *new_leaf;
	move_t        best_move;
	move_t        his_best;
	move_tree_t  *best_variation = nullptr, *new_variation = nullptr;
	board_check_t board_check;
	size_t        illegal_move_count = 0;

	best_move = null_move;
    *ret = null_move;

	move_list_t moves = generate_moves (board, side);
	if (moves.empty())
	{
		*ret_variation = nullptr;
		return evaluate (board, side, start_depth - depth);
	}

	for (auto move : moves)
	{
		if (timer_is_triggered (timer))
		{
			if (best_variation)
			{
				move_tree_destroy (new_variation);
				best_variation = nullptr;
			}

			break;
		}

		new_leaf = move_tree_new (history, move);

		board_check_init (&board_check, board);
        undo_move_t undo_state = do_move (board, side, move);

		if (!was_legal_move (board, side, move))
		{
		    illegal_move_count++;
			move_tree_free (new_leaf);
            undo_move (board, side, move, undo_state);
            board_check_validate (&board_check, board, side, move);
			continue;
		}

		nodes_visited++;

		if (depth <= 0)
		{
#if 0
			if (!no_quiesce)
				score = quiesce (board, side, alpha, beta, 0, new_leaf);
			else
#endif
				score = evaluate_and_check_draw (board, side, start_depth - depth,
                                     move, history);

		}
		else
		{
			score = (- search (board, color_invert (side), depth - 1,
			                   start_depth, &his_best, -beta, -alpha, 
			                   pseudo_rand, &new_variation,
			                   no_quiesce, timer, new_leaf));
		}

        undo_move (board, side, move, undo_state);
        board_check_validate (&board_check, board, side, move);

		move_tree_free (new_leaf);

		if (score > best || best == -INITIAL_ALPHA)
		{
			best           = score;
			best_move      = move;

			if (best_variation)
				move_tree_destroy (best_variation);

			best_variation = move_tree_new (new_variation, move);
		}
		else
		{
			move_tree_destroy (new_variation);
			new_variation = nullptr;
		}

		if (best > alpha)
			alpha = best;

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
        best = is_king_threatened (board, side, my_king_row, my_king_col) ?
                -1 * checkmate_score_in_moves (start_depth - depth) : 0;
    }

	// return the move if this is the last iteration
    if (!timer_is_triggered (timer))
        *ret = best_move;
			
    return best;
}

#if 0
int quiesce (struct board *board, color_t side, int alpha, int beta, int depth, 
             move_tree_t *history)
{
	move_list_t *captures;
	move_tree_t *new_leaf;
	move_t      *move;
	int          score;
	int          best_score;
	int          null_score;
	move_t       best_move;

	best_score = evaluate (board, side);
	if (best_score >= beta)
		return best_score;

	captures = generate_captures (board, side, history);
	if (!captures)
	{
		printf ("null captures, history:");
		move_list_print (history);
		return evaluate (board, side);
	}

	DBG (quiesce, "depth = %d\n", depth);

	if (best_score > alpha)
		alpha = best_score;
			
#if 0
	/* try the "null" move -- if no move is better than any move we
	 * have, then dont check any of the captures */
	null_score = (- search (board, !side, 1, 1, &best_move, -beta, -alpha,
	                        0, nullptr, 1, history));

	printf ("quiesce: searching for null move\n");
	print_tree (history);

	for_each_move (move, captures)
	{
		do_move (board, side, move);

		score = evaluate (board, side);

		undo_move (board, side, move);
	}

	printf ("null_score: %d, best_score: %d\n", null_score, best_score);
	if (null_score > best_score)
	{
		printf ("quiesce: null move cutoff\n");
		return null_score;
	}

#endif

	for_each_move (move, captures)
	{
		nodes_visited++;

		new_leaf = move_tree_new (history, *move);

		do_move (board, side, move);

		score = (- quiesce (board, !side, -beta, -alpha, depth+1, new_leaf));

		undo_move (board, side, move);

		move_tree_free (new_leaf);

		if (score > best_score)
		{
			best_score = score;

			if (best_score >= beta)
				break;

			if (score > alpha)
				alpha = score;
		}
	}

	move_list_destroy (captures);

	return best_score;
}
#endif

static void calc_time (int nodes, struct timeval *start, struct timeval *end)
{
	double time;
	double rate;

	time = (double) end->tv_sec - start->tv_sec + 
	       (double) (end->tv_usec - start->tv_usec) / 1000000.0;

	rate = (time == 0 ? nodes : (double) nodes / time);

	printf ("search took %.3f seconds, %.3f nodes/sec\n", 
			time, rate);
}

move_t iterate (struct board *board, enum color side,
                move_tree_t *history, struct timer *timer, int depth)
{
	move_t         best_move;
	int            best_score;
	struct timeval start, end;
	move_tree_t   *principal_variation;

	printf ("finding moves for %s\n", (side == COLOR_WHITE) ? "white":"black");

	nodes_visited = 0; cutoffs = 0;

	best_move = move_null ();

	gettimeofday (&start, nullptr);

	best_score = search (board, side, depth, depth, &best_move,
                        -INITIAL_ALPHA, INITIAL_ALPHA,
	                     (start.tv_usec >> 16) | (start.tv_sec << 16),
                         &principal_variation, 0, timer, history);

	gettimeofday (&end, nullptr);

	calc_time (nodes_visited, &start, &end);

	if (!is_null_move (best_move))
	{
		printf ("move selected = %s [ score: %d ]\n", move_str (best_move), 
	            best_score);
		printf ("nodes visited = %d, cutoffs = %d\n", nodes_visited, cutoffs);
	}

	// principal variation could be null if search was interrupted
	if (principal_variation)
	{
		printf ("principal variation: ");
		print_reversed_tree (principal_variation);

		move_tree_destroy (principal_variation);
	}

	return best_move;
}

move_t find_best_move (struct board *board, enum color side,
                       move_tree_t *history)
{
	int d;
	int max_depth = MAX_DEPTH;
	move_t move, best_move;
    board_check_t board_check;
    bool stop_early = false;
    struct timer overdue_timer;

	timer_init (&overdue_timer, MAX_SEARCH_SECONDS);

	/*
	 * 2003-08-28: We should search by depths that are multiples
	 * of two. This way, we won't artificially inflate a line
	 * of play by not looking at the response (e.g. We see at
	 * the end of a sequence we can take the opponents queen with
	 * our own, we don't see that he can take back on the next move)
	 *
	 * The only exception is that we want to select SOME move quickly.
	 * TODO: we should pick a random move instead if we don't
	 * get a chance to look.
	 */
    for (d = 0; d <= max_depth; (d == 0 ? (d++) : (d += 2)))
	{
		move = iterate (board, side, history, &overdue_timer, d);

		if (timer_is_triggered(&overdue_timer))
		{
			printf ("exiting early with depth %d\n", d);
			break;
		}

		if (is_null_move(move))
		{
		    // I think this can probably happen if we run out of time at just the right time.
		    printf("Next best move is null move. Terminating.");
		    break;
		}

		board_check_init (&board_check, board);
        undo_move_t undo_state = do_move (board, side, move);
		board_print (board);

		best_move = move;
		if (d == 0 && is_checkmated (board, color_invert(side)))
            stop_early = true;

        undo_move (board, side, move, undo_state);
        board_check_validate (&board_check, board, side, move);

        if (stop_early)
            break;
	}

    if (is_null_move(best_move))
    {
        // TODO: this is possible in a stalement position
        // TODO: select random move in this case.
        printf ("best selected null move at depth %d\n", d);
        abort ();
    }

    board_check_init (&board_check, board);
    undo_move_t undo_state = do_move (board, side, best_move);
	board_print (board);
    undo_move (board, side, best_move, undo_state);
    board_check_validate (&board_check, board, side, best_move);

	return best_move;
}

// Get the score for a checkmate discovered X moves away.
// Checkmates closer to the current position are more valuable than those
// further away.
int checkmate_score_in_moves (size_t moves)
{
    return INFINITE + INFINITE / (1 + moves);
}

// vi: set ts=4 sw=4:
