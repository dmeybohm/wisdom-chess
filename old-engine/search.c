#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>

#include "piece.h"
#include "board.h"
#include "move_list.h"
#include "generate.h"
#include "evaluate.h"
#include "check.h"
#include "move_tree.h"
#include "debug.h"
#include "search.h"

DEFINE_DEBUG_CHANNEL (search, 0);
DEFINE_DEBUG_CHANNEL (quiesce, 1);

int search (struct board *board, color_t side, int depth, int start_depth,
            move_t *ret, int alpha, int beta, unsigned long pseudo_rand,
            move_tree_t **ret_variation, int no_quiesce, 
			move_tree_t *history);
int quiesce (struct board *board, color_t side, int alpha, int beta, int depth, 
             move_tree_t *history);

static int              nodes_visited, cutoffs;
static volatile int     signalled;

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

int search (struct board *board, color_t side, int depth, int start_depth,
            move_t *ret, int alpha, int beta, unsigned long pseudo_rand,
            move_tree_t **ret_variation, int no_quiesce, 
			move_tree_t *history) 
{
	int          score;
	int          best   = -INFINITY;
	move_list_t *moves;
	move_tree_t *new_leaf;
	move_t      *move;
	move_t       best_move;
	move_t       his_best;
	move_tree_t *best_variation = NULL, *new_variation = NULL;
	
	move_nullify (&best_move);

	moves = generate_moves (board, side, history);
	if (!moves)
	{
		*ret_variation = NULL;

		return evaluate (board, side, 1, NULL);
	}

	for_each_move (move, moves)
	{
		if (signalled)
		{
			if (best_variation)
			{
				move_tree_destroy (new_variation);
				best_variation = NULL;
			}

			break;
		}

		new_leaf = move_tree_new (history, *move);

		do_move (board, side, move);

		if (!was_legal_move (board, side, move))
		{
			move_tree_free (new_leaf);
			undo_move (board, side, move);
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
				score = evaluate (board, side, 0, move);
		}
		else
		{
			score = (- search (board, color_invert (side), depth-1, 
			                   start_depth, &his_best, -beta, -alpha, 
			                   pseudo_rand, &new_variation,
			                   no_quiesce, new_leaf));
		}

		undo_move (board, side, move);

		move_tree_free (new_leaf);

#if 0
		if (score > best || (score == best && 
			(pseudo_rand >> 16) > (pseudo_rand & 0xffff)))
#endif
		if (score > best)
		{
			best           = score;
			best_move      = *move;

			if (best_variation)
				move_tree_destroy (best_variation);

#if 0
			printf ("pseudo_rand >> 16 > pseudo_rand & 0xffff: %d\n",
			        (pseudo_rand >> 16) > (pseudo_rand & 0xffff));
#endif

			pseudo_rand ^= (pseudo_rand << 7 | pseudo_rand >> 7);

			best_variation = move_tree_new (new_variation, *move);
		}
		else
		{
			move_tree_destroy (new_variation);
			new_variation = NULL;
		}

		if (best > alpha)
			alpha = best;

		if (alpha >= beta)
		{
			cutoffs++;
			break;
		}
	}

	assert (ret_variation != NULL);

/* ARGH! No idea why principal variation is crashing things */
#if 1
	*ret_variation = best_variation;
#else
	*ret_variation = NULL;
	if (best_variation)
		move_tree_destroy (best_variation);
#endif

	move_list_destroy (moves);

	/* return the move if this is the last iteration */
	if (unlikely (depth == start_depth))
	{
		move_nullify (ret);
		if (!signalled)
			*ret = best_move;
#if 0
		print_tree (history, &best_move, best);
#endif
	}
			
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
		print_tree (history);
		return evaluate (board, side);
	}

	DBG (quiesce, "depth = %d\n", depth);

	if (best_score > alpha)
		alpha = best_score;
			
#if 0
	/* try the "null" move -- if no move is better than any move we
	 * have, then dont check any of the captures */
	null_score = (- search (board, !side, 1, 1, &best_move, -beta, -alpha,
	                        0, NULL, 1, history));

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

	time = (double) (end->tv_sec - start->tv_sec) + 
	       (double) ((end->tv_usec - start->tv_usec) / 1000000.0);

	rate = (time == 0 ? nodes : (double) (nodes / time));

	printf ("search took %.3f seconds, %.3f nodes/sec\n", 
			time, rate);
}

move_t iterate (struct board *board, color_t side,
                move_tree_t *history, int depth)
{
	move_t         best_move;
	int            best_score;
	struct timeval start, end;
	move_tree_t   *principal_variation;

	printf ("finding moves for %s\n", (side == COLOR_WHITE) ? "white":"black");

	nodes_visited = 0; cutoffs = 0;

	move_nullify (&best_move);

	gettimeofday (&start, NULL);

	best_score = search (board, side, depth, depth, &best_move, 
	                     -INFINITY, INFINITY, 
	                     (start.tv_usec >> 16) | (start.tv_sec << 16),
						 &principal_variation, 0, history);

	gettimeofday (&end, NULL);

	calc_time (nodes_visited, &start, &end);

	if (!is_null_move (best_move))
	{
		printf ("move selected = %s [ score: %d ]\n", move_str (best_move), 
	            best_score);
		printf ("nodes visited = %d, cutoffs = %d\n", nodes_visited, cutoffs);
	}

	/* principal variation could be null if search was interrupted */
	if (principal_variation)
	{
		printf ("principal variation: ");
		print_reversed_tree (principal_variation);

		move_tree_destroy (principal_variation);
	}

	return best_move;
}

void stop_search (int signum)
{
	signalled = 1;
}

move_t find_best_move (struct board *board, color_t side,
                       move_tree_t *history)
{
	int d;
	int max_depth = 12;
	move_t move, best_move;

	signalled = 0;
	signal (SIGALRM, stop_search);
	alarm (10);

	for (d = 1; d <= max_depth; d++)
	{
		move = iterate (board, side, history, d);

		if (signalled)
		{
			printf ("exiting early with depth %d\n", d);
			break;
		}

		do_move (board, side, &move);
		board_print (board);
		undo_move (board, side, &move);

		best_move = move;
	}


	do_move (board, side, &best_move);
	board_print (board);
	undo_move (board, side, &best_move);

	return best_move;
}

/* vi: set ts=4 sw=4: */