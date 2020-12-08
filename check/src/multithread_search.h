#ifndef WIZDUMB_MULTITHREAD_SEARCH_H
#define WIZDUMB_MULTITHREAD_SEARCH_H

#include <memory>

#include "board.h"
#include "search.h"
#include "move_timer.h"

namespace wisdom
{
    class output;
}

class multithread_search_handler;
class history;

class multithread_search
{
private:
    search_result_t result { null_move, 0, 0 };
    std::unique_ptr<multithread_search_handler> handler;

public:
    multithread_search (board &board, Color side, wisdom::output &output,
                        const class history &history, const move_timer &timer);

    ~multithread_search();

    search_result_t search();
};

#endif //WIZDUMB_MULTITHREAD_SEARCH_H
