#include "multithread_search.h"
#include "timer.h"

#include <thread>
#include <iostream>
#include <sstream>

enum class multithread_params
{
    MAX_DEPTH = 16,
};

struct thread_params
{
    struct board board;
    move_history_t move_history;
    int seconds;
    enum color side;
    int depth;
    struct timer time {};

    thread_params (const struct board &_board, const move_history_t &_move_history, int _seconds,
                   enum color _side, int _depth) :
            board { _board },
            move_history { _move_history },  seconds { _seconds }, side { _side }, depth { _depth }
    {
        timer_init (&time, seconds);
    }

};

struct result_move
{
    move_t move;
    int depth;
};

static std::mutex mutex;
static int next_depth = 0;
static std::vector<thread_params> all_thread_params;
static std::vector<std::thread> threads;
static std::vector<result_move> result_moves;

static int get_next_depth()
{
    std::lock_guard guard (mutex);
    int d = next_depth;
    next_depth = d == 0 ? d + 1 : d + 2;
    return next_depth;
}

void add_result_move(move_t move, int depth)
{
    std::lock_guard guard (mutex);
    result_move result { move, depth };
    result_moves.push_back (result);
}

void do_thread(unsigned index)
{
    thread_params &params = all_thread_params[index];
    std::stringstream output;

    if (params.depth >= (int)multithread_params::MAX_DEPTH)
        return;

    move_t result = iterate (&params.board, params.side,
                           params.move_history, &params.time, params.depth);
    output << "Finished thread " << std::this_thread::get_id() << " with depth " << params.depth << "\n";
    output << "Move: " << to_string(result) << "\n";

    std::cout << output.str();
    if (result == null_move)
    {
        // probably timed out:
        return;
    }
    add_result_move (result, params.depth);

    // Continue with more depth:
    params.depth = get_next_depth();
    do_thread(index);
}

move_t multithread_search (struct board &board, enum color side,
                           move_history_t &move_history, struct timer *timer, int depth)
{
    unsigned int max_nr_threads = std::thread::hardware_concurrency();
    int seconds = (int)timer->seconds;

    threads.clear();
    all_thread_params.clear();
    result_moves.clear();

    next_depth = -1;

    // Create all the thread parameters first, to ensure they get allocated:
    for (unsigned i = 0; i < max_nr_threads; i++)
    {
        thread_params params{board, move_history, seconds, side, get_next_depth()};
        all_thread_params.push_back(params);
    }

    // Now create the threads:
    for (unsigned i = 0; i < max_nr_threads; i++)
    {
        std::thread thr = std::thread{do_thread, i};
        threads.push_back (std::move(thr));
    }

    // kill all the threads:
    sleep(seconds);
    for (auto &thr: threads)
    {
        thr.join();
    }

    result_move result { null_move, -1 };

    for (auto &result_move : result_moves)
    {
        if (result_move.depth > result.depth)
            result = result_move;
    }

    return result.move;
}