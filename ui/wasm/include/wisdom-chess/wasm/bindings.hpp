#pragma once

#include <emscripten.h>
#include <emscripten/wasm_worker.h>

#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/logger.hpp"

extern "C"
{
    // Reinitialize a new game.
    EMSCRIPTEN_KEEPALIVE void workerReinitializeGame (int new_game_id);

    // Start searching for moves.
    EMSCRIPTEN_KEEPALIVE void startSearch();

    // Receive a move on the main thread from the chess engine.
    EMSCRIPTEN_KEEPALIVE void mainThreadReceiveMove (int game_id, int packed_move);

    // Receive a move from the human player.
    EMSCRIPTEN_KEEPALIVE void workerReceiveMove (int packed_move);

    // Receive a settings update.
    EMSCRIPTEN_KEEPALIVE void workerReceiveSettings (int white_player, int black_player,
                                                     int thinking_time, int search_depth);

    // Tell the worker to stop searching for moves. This can be called from any thread.
    EMSCRIPTEN_KEEPALIVE void pauseWorker (void);

    // Remove the unpause state from the worker. This can be called from any thread.
    EMSCRIPTEN_KEEPALIVE void unpauseWorker (void);

    // Update the draw status.
    EMSCRIPTEN_KEEPALIVE void mainThreadReceiveDrawStatus (int game_id, int draw_type, int color,
                                                           int draw_proposed);
}

