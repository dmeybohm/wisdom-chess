#ifndef WISDOMCHESS_BINDINGS_HPP
#define WISDOMCHESS_BINDINGS_HPP

#include <emscripten.h>
#include <emscripten/wasm_worker.h>

#include "game.hpp"
#include "move.hpp"
#include "logger.hpp"

extern "C"
{
    // Reinitialize a new game.
    EMSCRIPTEN_KEEPALIVE void worker_reinitialize_game (int new_game_id);

    // Start searching for moves.
    EMSCRIPTEN_KEEPALIVE void start_search();

    // Receive a move on the main thread from the chess engine.
    EMSCRIPTEN_KEEPALIVE void main_thread_receive_move (int game_id, int packed_move);

    // Receive a move from the human player.
    EMSCRIPTEN_KEEPALIVE void worker_receive_move (int packed_move);

    // Receive a settings update.
    EMSCRIPTEN_KEEPALIVE void worker_receive_settings (int white_player, int black_player,
                                                        int thinking_time, int search_depth);

    // Tell the worker to stop searching for moves. This can be called from any thread.
    EMSCRIPTEN_KEEPALIVE void pause_worker (void);

    // Remove the unpause state from the worker. This can be called from any thread.
    EMSCRIPTEN_KEEPALIVE void unpause_worker (void);
}

#endif // WISDOMCHESS_BINDINGS_HPP
