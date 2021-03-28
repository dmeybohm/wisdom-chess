#ifndef WISDOM_MULTITHREAD_SEARCH_HPP
#define WISDOM_MULTITHREAD_SEARCH_HPP

#include <memory>

#include "board.hpp"
#include "search.hpp"
#include "move_timer.hpp"

namespace wisdom
{
    class Output;

    class MultithreadSearchHandler;

    class History;

    class MultithreadSearch
    {
    private:
        SearchResult result = SearchResult::from_initial ();
        std::unique_ptr<MultithreadSearchHandler> handler;

    public:
        MultithreadSearch (Board &board, Color side, wisdom::Output &output,
                           const History &history, const MoveTimer &timer);

        ~MultithreadSearch ();

        SearchResult search ();
    };

}
#endif //WISDOM_MULTITHREAD_SEARCH_HPP
