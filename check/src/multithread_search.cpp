#include "multithread_search.h"
#include "timer.h"

#include <thread>
#include <iostream>
#include <sstream>

constexpr int MAX_DEPTH = 16;

search_result_t multithread_search::do_multithread_search()
{
    unsigned int max_nr_threads = std::thread::hardware_concurrency();

    next_depth = -1;

    // Create all the thread parameters first, to ensure they get allocated:
    for (unsigned i = 0; i < max_nr_threads; i++)
    {
        thread_params params { board, move_history, timer, side, this->get_next_depth() };
        all_thread_params.push_back(params);
    }

    // Now create the threads:
    for (unsigned i = 0; i < max_nr_threads; i++)
    {
        std::thread thr = std::thread (&multithread_search::do_thread, this, i);
        threads.push_back (std::move(thr));
    }

    // kill all the threads:
    std::this_thread::sleep_for(timer.seconds);
    for (auto &thr: threads)
    {
        thr.join();
    }

    search_result_t result;

    for (auto &result_move : result_moves)
    {
        if (result_move.depth > result.depth)
            result = result_move;
    }

    return result;
}

int multithread_search::get_next_depth()
{
    std::lock_guard guard { mutex };

    int d = next_depth;
    next_depth = d == 0 ? d + 1 : d + 2;
    return next_depth;
}

void multithread_search::add_result(search_result_t result)
{
    std::lock_guard guard (mutex);

    result_moves.push_back (result);
}

void multithread_search::do_thread(unsigned index)
{
    thread_params &params = all_thread_params[index];
    std::stringstream output;

    if (params.depth >= MAX_DEPTH)
        return;

    move_t result = iterate (&params.board, params.side,
                             params.move_history, params.timer, params.depth);
    output << "Finished thread " << std::this_thread::get_id() << " with depth " << params.depth << "\n";
    output << "Move: " << to_string(result) << "\n";

    std::cout << output.str();
    if (result == null_move)
    {
        // probably timed out:
        return;
    }
    search_result_t synthesized_result { .move = result, .score = 0, .depth = params.depth };
    add_result (synthesized_result);

    // Continue with more depth:
    params.depth = get_next_depth();
    do_thread(index);
}