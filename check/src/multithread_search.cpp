#include "multithread_search.h"
#include "timer.h"

#include <mutex>
#include <thread>
#include <iostream>
#include <sstream>
#include <memory>
#include <vector>
#include <chrono>

constexpr int MAX_DEPTH = 16;

using std::chrono::seconds;

struct thread_params
{
    struct board board;
    move_history_t move_history;
    enum color side;
    int depth;
    struct timer timer;

    thread_params (const struct board &_board, const move_history_t &_move_history, struct timer _timer,
                   enum color _side, int _depth) :
            board { _board },
            move_history { _move_history }, side { _side }, depth { _depth },
            timer { _timer }
    {
    }

};

class multithread_search_handler
{
public:
    multithread_search_handler (struct board &_board, enum color _side,
                        const move_history_t &_move_history, const timer &_timer) :
            board { _board }, side { _side }, move_history { _move_history }, timer { _timer }
    {}

    search_result_t result()
    {
        // If already searched, return result.
        if (search_result.move != null_move)
            return search_result;

        search_result = do_multithread_search();
        return search_result;
    }

private:
    struct board board;
    enum color side;
    move_history_t move_history;
    timer timer;
    search_result_t search_result;

    // Mutex to protect next_depth
    std::mutex mutex;
    int next_depth = -1;

    // Per thread variables;
    std::vector<thread_params> all_thread_params;
    std::vector<std::thread> threads;
    std::vector<search_result_t> result_moves;

    search_result_t do_multithread_search();

    int get_next_depth();

    void do_thread(unsigned index);

    void add_result(search_result_t result);
};

search_result_t multithread_search_handler::do_multithread_search()
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
        std::thread thr = std::thread (&multithread_search_handler::do_thread, this, i);
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

int multithread_search_handler::get_next_depth()
{
    std::lock_guard guard { mutex };

    int d = next_depth;
    next_depth = d == 0 ? d + 1 : d + 2;
    return next_depth;
}

void multithread_search_handler::add_result(search_result_t result)
{
    std::lock_guard guard (mutex);

    result_moves.push_back (result);
}

void multithread_search_handler::do_thread(unsigned index)
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

multithread_search::multithread_search(struct board &_board, enum color _side,
                                       const move_history_t &_move_history, const timer &_timer) :
    handler { std::make_unique<multithread_search_handler>(_board, _side, _move_history, _timer) }
{
}

search_result_t multithread_search::search()
{
    if (result.move == null_move)
        result = handler->result();
    return result;
}

multithread_search::~multithread_search()
{

}
