#ifndef WISDOMCHESS_BINDINGS_HPP
#define WISDOMCHESS_BINDINGS_HPP

#include <emscripten.h>
#include <emscripten/wasm_worker.h>

#include "game.hpp"
#include "move.hpp"
#include "logger.hpp"

extern "C"
{
    EMSCRIPTEN_KEEPALIVE void worker_reinitialize_game (int new_game_id);

    EMSCRIPTEN_KEEPALIVE void start_search();

    EMSCRIPTEN_KEEPALIVE void main_thread_receive_move (int game_id, int packed_move);

    EMSCRIPTEN_KEEPALIVE void worker_receive_move (int packed_move);

    EMSCRIPTEN_KEEPALIVE void worker_receive_settings (int white_player, int black_player,
                                                        int thinking_time, int search_depth);

    EMSCRIPTEN_KEEPALIVE void worker_manager_pause_worker (void);

    EMSCRIPTEN_KEEPALIVE void worker_manager_resume_worker (void);

}

#endif // WISDOMCHESS_BINDINGS_HPP
