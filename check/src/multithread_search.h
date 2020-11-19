#ifndef WIZDUMB_MULTITHREAD_SEARCH_H
#define WIZDUMB_MULTITHREAD_SEARCH_H

#include "board.h"
#include "search.h"
#include "timer.h"

#include <mutex>
#include <vector>
#include <thread>
#include <chrono>

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

class multithread_search
{
public:
    multithread_search (struct board &_board, enum color _side,
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

#endif //WIZDUMB_MULTITHREAD_SEARCH_H
