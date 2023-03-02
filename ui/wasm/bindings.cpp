#include "bindings.hpp"
#include "web_logger.hpp"

EMSCRIPTEN_KEEPALIVE void worker_initialize_game()
{
    using namespace wisdom;
}

EMSCRIPTEN_KEEPALIVE void start_search()
{
    using namespace wisdom;
    auto game = wisdom::worker::get_game();
    const auto& logger = wisdom::worker::get_logger();
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

EMSCRIPTEN_KEEPALIVE void worker_receive_move (int packed_move)
{
    using namespace wisdom;

    auto game = wisdom::worker::get_game();
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
