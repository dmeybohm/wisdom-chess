#ifndef WIZDUMB_MULTITHREAD_SEARCH_H
#define WIZDUMB_MULTITHREAD_SEARCH_H

#include <memory>

#include "board.h"
#include "search.h"
#include "move_timer.h"

class multithread_search_handler;

class multithread_search
{
private:
    search_result_t result { null_move, 0, 0 };
    std::unique_ptr<multithread_search_handler> handler;

public:
    multithread_search (struct board &_board, enum color _side,
                        const move_history_t &_move_history, const move_timer &_timer);

    ~multithread_search();

    search_result_t search();
};

#endif //WIZDUMB_MULTITHREAD_SEARCH_H
