#pragma once

#include <emscripten.h>
#include <emscripten/wasm_worker.h>

#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/logger.hpp"

extern "C"
{
    // Reinitialize a new game. Called from the main thread
    EMSCRIPTEN_KEEPALIVE void workerReinitializeGame (int new_game_id);

    // Start searching for moves. Called from the main thread.
    EMSCRIPTEN_KEEPALIVE void startSearch();

    // Called from the worker. Receive a move on the main thread from the chess engine.
    EMSCRIPTEN_KEEPALIVE void mainThreadReceiveMove (
        int game_id, 
        int packed_move
    );

    // Receive a move from the human player from the main thread.
    EMSCRIPTEN_KEEPALIVE void workerReceiveMove (int packed_move);

    // Receive a settings update from the main thread.
    EMSCRIPTEN_KEEPALIVE void workerReceiveSettings (
        int white_player, 
        int black_player, 
        int thinking_time, 
        int search_depth
    );

    // Called from the main thread to tell the worker to stop searching for moves.
    EMSCRIPTEN_KEEPALIVE void pauseWorker (void);

    // Called form the main thread to unpause the worker.
    EMSCRIPTEN_KEEPALIVE void unpauseWorker (void);

    // Update the draw status.
    EMSCRIPTEN_KEEPALIVE void mainThreadReceiveDrawStatus (
        int game_id, 
        int draw_type, 
        int color, 
        int draw_proposed
    );
}

