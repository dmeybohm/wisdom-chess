#ifndef WIZDUMB_MULTITHREAD_SEARCH_H
#define WIZDUMB_MULTITHREAD_SEARCH_H

#include <memory>

#include "board.h"
#include "search.h"
#include "move_timer.h"

namespace wisdom
{
    class Output;
}

class MultithreadSearchHandler;
class MoveHistory;

class MultithreadSearch
{
private:
    SearchResult result {null_move, 0, 0 };
    std::unique_ptr<MultithreadSearchHandler> handler;

public:
    MultithreadSearch (Board &board, Color side, wisdom::Output &output,
                       const class MoveHistory &history, const MoveTimer &timer);

    ~MultithreadSearch ();

    SearchResult search ();
};

#endif //WIZDUMB_MULTITHREAD_SEARCH_H
