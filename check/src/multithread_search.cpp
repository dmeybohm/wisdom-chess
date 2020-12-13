#include "multithread_search.h"
#include "move_timer.h"
#include "output.hpp"
#include "history.hpp"

#include <mutex>
#include <thread>
#include <iostream>
#include <sstream>
#include <memory>
#include <utility>
#include <vector>
#include <chrono>

constexpr int Max_Depth = 16;

using std::chrono::seconds;
using wisdom::output;

struct thread_params
{
    struct board board;
    Color side;
    class output &output;
    class history history;
    int depth;
    struct move_timer timer;

    thread_params (const struct board &board_, Color side_, class output &output_,
                   class history history_, struct move_timer timer_, int depth_) :
            board { board_ }, side { side_ }, output { output_ },
            history { std::move(history_) },  depth { depth_ },
            timer { timer_ }
    {
    }

};

class multithread_search_handler
{
public:
    multithread_search_handler (struct board &board_, Color side_, output &output_,
                                const class history &history_, const move_timer &timer_) :
            board { board_ }, side { side_ }, output { output_ },
            history { history_ }, timer { timer_ }
    {}

    search_result_t result()
    {
        // If already searched, return result.
        if (search_result.move != null_move)
            return search_result;

        search_result = do_multithread_search ();
        return search_result;
    }

private:
    struct board board;
    Color side;
    class output &output;
    const class history &history; // reference here, and copied into thread params.
    struct move_timer timer;
    search_result_t search_result;

    // Mutex to protect next_depth
    std::mutex mutex;
    int next_depth = -1;

    // Per thread variables;
    std::vector<thread_params> all_thread_params;
    std::vector<std::thread> threads;
    std::vector<search_result_t> result_moves;

    search_result_t do_multithread_search ();

    int get_next_depth ();

    void do_thread (unsigned index);

    void add_result (search_result_t result);
};

search_result_t multithread_search_handler::do_multithread_search ()
{
    unsigned int max_nr_threads = std::thread::hardware_concurrency();

    next_depth = -1;

    // Create all the thread parameters first, to ensure they get allocated:
    for (unsigned i = 0; i < max_nr_threads; i++)
    {
        thread_params params { board, side, output, history, timer, this->get_next_depth() };
        all_thread_params.push_back (params);
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
        thr.join ();
    }

    // todo handle empty case
    assert (!result_moves.empty());
    search_result_t result = result_moves.front();

    for (auto &result_move : result_moves)
    {
        if (result_move.depth > result.depth)
            result = result_move;
    }

    return result;
}

int multithread_search_handler::get_next_depth ()
{
    std::lock_guard guard { mutex };

    int d = next_depth;
    next_depth = d == 0 ? d + 1 : d + 2;
    return next_depth;
}

void multithread_search_handler::add_result (search_result_t result)
{
    std::lock_guard guard { mutex };

    result_moves.push_back (result);
}

void multithread_search_handler::do_thread (unsigned index)
{
    thread_params &params = all_thread_params[index];
    std::stringstream messages;

    if (params.depth >= Max_Depth)
        return;

    move_t result = iterate (params.board, params.side, params.output,
                             params.history, params.timer, params.depth);
    messages << "Finished thread " << std::this_thread::get_id() << " with depth " << params.depth << "\n";
    messages << "Move: " << to_string(result);

    params.output.println (messages.str ());

    if (result == null_move)
    {
        // probably timed out:
        return;
    }
    search_result_t synthesized_result { .move = result, .score = 0, .depth = params.depth };
    add_result (synthesized_result);

    // Continue with more depth:
    params.depth = get_next_depth();
    do_thread (index);
}

multithread_search::multithread_search (struct board &board, Color side, output &output,
                                        const history &history, const move_timer &timer)
{
	handler = std::make_unique<multithread_search_handler> (board, side, output, history, timer);
}

search_result_t multithread_search::search()
{
    if (result.move == null_move)
        result = handler->result();
    return result;
}

multithread_search::~multithread_search() = default;
