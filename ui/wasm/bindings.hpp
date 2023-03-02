#ifndef WISDOMCHESS_BINDINGS_HPP
#define WISDOMCHESS_BINDINGS_HPP

#include <emscripten.h>
#include <emscripten/wasm_worker.h>

#include "game.hpp"
#include "move.hpp"
#include "logger.hpp"

extern "C"
{
    void worker_initialize_game();

    void start_search();

    void main_thread_receive_move (int packed_move);

    void worker_receive_move (int packed_move);

    void worker_receive_settings (int whitePlayer, int blackPlayer,
                                  int thinkingTime, int searchDepth);

    void worker_manager_pause_worker (void);
}

#endif // WISDOMCHESS_BINDINGS_HPP
