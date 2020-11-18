#ifndef WIZDUMB_MULTITHREAD_SEARCH_H
#define WIZDUMB_MULTITHREAD_SEARCH_H

#include "search.h"

move_t multithread_search (struct board &board, enum color side,
                           move_history_t &move_history, struct timer *timer, int depth);

#endif //WIZDUMB_MULTITHREAD_SEARCH_H
