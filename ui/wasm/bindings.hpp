//
// Created by dmeybohm on 2/11/2023.
//

#ifndef WISDOMCHESS_BINDINGS_HPP
#define WISDOMCHESS_BINDINGS_HPP

#include <emscripten.h>

#include "game.hpp"
#include "move.hpp"
#include "logger.hpp"

namespace wisdom::web
{

    class WebLogger : public wisdom::Logger
    {
    public:
        WebLogger() = default;

        void debug (const std::string& output) const override;
        void info (const std::string& output) const override;
    };

    [[nodiscard]] auto get_logger() -> const WebLogger&
    {
        using namespace wisdom;
        static std::unique_ptr<WebLogger> worker_logger = std::make_unique<WebLogger>();
        return *worker_logger;
    }

    [[nodiscard]] auto get_game () -> wisdom::observer_ptr<wisdom::Game>
    {
        using namespace wisdom;
        static std::unique_ptr<wisdom::Game> worker_game = std::make_unique<wisdom::Game> (
            Player::Human, Player::ChessEngine);
        return worker_game.get();
    }
}

extern "C"
{
    EMSCRIPTEN_KEEPALIVE void worker_initialize_game()
    {
        using namespace wisdom;
    }

    void main_thread_receive_move (int packed_move);

    EMSCRIPTEN_KEEPALIVE void start_search()
    {
        using namespace wisdom;
        auto game = wisdom::web::get_game();
        const auto& logger = wisdom::web::get_logger();
        auto move = game->find_best_move(
            logger,
            game->get_current_turn()
        );
        if (!move.has_value())
            throw wisdom::Error { "No moves found." };
        game->move (*move);
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

        auto game = wisdom::web::get_game();
        auto unpacked_move = Move::from_int (packed_move);
        game->move (unpacked_move);

        if (game->get_current_player() == Player::ChessEngine)
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

inline void wisdom::web::WebLogger::debug (const std::string& output) const
{
    console_log (output.c_str());
}

inline void wisdom::web::WebLogger::info (const std::string& output) const
{
    console_log (output.c_str());
}

#endif // WISDOMCHESS_BINDINGS_HPP
