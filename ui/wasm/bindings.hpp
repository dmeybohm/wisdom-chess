//
// Created by dmeybohm on 2/11/2023.
//

#ifndef WISDOMCHESS_BINDINGS_HPP
#define WISDOMCHESS_BINDINGS_HPP

#include <emscripten.h>

#include "game.hpp"
#include "move.hpp"
#include "logger.hpp"

extern "C"
{
    inline wisdom::Game worker_game;
    inline std::unique_ptr<wisdom::Logger> worker_logger {
        wisdom::make_standard_logger (wisdom::Logger::LogLevel::LogLevel_Info)
    };

    EMSCRIPTEN_KEEPALIVE void worker_initialize_game()
    {
        using namespace wisdom;

        worker_game = { Player::Human, Player::ChessEngine };
    }

    void main_thread_receive_move (int packed_move);

    EMSCRIPTEN_KEEPALIVE void start_search()
    {
        using namespace wisdom;
        auto move = worker_game.find_best_move(*worker_logger, worker_game.get_current_turn());
        if (!move.has_value())
            throw wisdom::Error { "No moves found." };
        worker_game.move (*move);
        emscripten_wasm_worker_post_function_vi (
            EMSCRIPTEN_WASM_WORKER_ID_PARENT,
            main_thread_receive_move,
            move->to_int()
        );
    }

    // The worker receives a move:
    EMSCRIPTEN_KEEPALIVE void worker_receive_move (int packed_move)
    {
        using namespace wisdom;

        auto unpacked_move = Move::from_int (packed_move);
        worker_game.move (unpacked_move);

        if (worker_game.get_current_player() == Player::ChessEngine)
            start_search();
    }

    EM_JS (void, receiveMoveFromWorker, (const char* str),
    {
       receiveWorkerMessage ('computerMoved', UTF8ToString (str));
    })

    EMSCRIPTEN_KEEPALIVE void main_thread_receive_move (int packed_move)
    {
        using namespace wisdom;
        Move unpacked_move = Move::from_int (packed_move);
        std::string str = to_string (unpacked_move);
        receiveMoveFromWorker (str.c_str());
    }

    EM_JS (void, console_log, (const char* str), { console.log (UTF8ToString (str)) })


}

#endif // WISDOMCHESS_BINDINGS_HPP
