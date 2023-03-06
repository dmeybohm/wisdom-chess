#include "bindings.hpp"
#include "web_logger.hpp"
#include "game_settings.hpp"
#include "web_types.hpp"

using namespace wisdom;

namespace wisdom::worker
{
    struct GameState
    {
        std::unique_ptr<wisdom::Game> game;
        wisdom::GameSettings settings {};
        int game_id {};

        GameState () :
            game { std::make_unique<wisdom::Game> ()}
        {
            update_settings (settings);
        }

        [[nodiscard]] static auto get_state() -> observer_ptr<GameState>
        {
            static auto instance = std::make_unique<GameState>();
            return instance.get();
        }

        [[nodiscard]] static auto get_game() -> observer_ptr<Game>
        {
            return GameState::get_state()->game.get();
        }

        [[nodiscard]] static auto map_human_depth_to_computer_depth (int human_depth) -> int
        {
            return human_depth * 2 - 1;
        }

        void update_settings (GameSettings new_settings)
        {
            settings = new_settings;
            game->set_search_timeout (std::chrono::seconds { settings.thinkingTime });
            game->set_max_depth (
                GameState::map_human_depth_to_computer_depth (settings.searchDepth)
            );
            game->set_players ({
                map_player (settings.whitePlayer),
                map_player (settings.blackPlayer)
            });
        }
    };
}

using namespace wisdom::worker;

EMSCRIPTEN_KEEPALIVE void worker_reinitialize_game (int new_game_id)
{
    auto state = GameState::get_state();

    state->game_id = new_game_id;

    auto new_game = std::make_unique<Game> ();
    state->game = std::move (new_game);
    state->update_settings (state->settings);

//    newGame->set_periodic_function ()

}

EMSCRIPTEN_KEEPALIVE void start_search()
{
    auto state = GameState::get_state();
    auto game = GameState::get_game();

    const auto& logger = wisdom::worker::get_logger();

    auto move = game->find_best_move(
        logger,
        game->get_current_turn()
    );
    if (!move.has_value())
        throw wisdom::Error { "No moves found." };
    game->move (*move);
    emscripten_wasm_worker_post_function_vii (
        EMSCRIPTEN_WASM_WORKER_ID_PARENT,
        main_thread_receive_move,
        state->game_id,
        move->to_int()
    );
}

EMSCRIPTEN_KEEPALIVE void worker_receive_move (int packed_move)
{
    auto game = GameState::get_game();
    auto unpacked_move = Move::from_int (packed_move);
    game->move (unpacked_move);

    if (game->get_current_player() == Player::ChessEngine)
        start_search();
}

void worker_receive_settings (int white_player, int black_player, int thinking_time,
                              int search_depth)
{
    auto state = GameState::get_state();

    state->update_settings (GameSettings {
        .whitePlayer = static_cast<WebPlayer> (white_player),
        .blackPlayer = static_cast<WebPlayer> (black_player),
        .thinkingTime = thinking_time,
        .searchDepth = search_depth
    });
}

EM_JS (void, receiveMoveFromWorker, (int game_id, const char* str),
{
   receiveWorkerMessage ('computerMoved', game_id, UTF8ToString (str));
})

EMSCRIPTEN_KEEPALIVE void main_thread_receive_move (int game_id, int packed_move)
{
    auto& logger = wisdom::worker::get_logger();
    logger.debug (std::to_string (packed_move));

    Move unpacked_move = Move::from_int (packed_move);
    auto state = GameState::get_state();
    std::string str = to_string (unpacked_move);
    receiveMoveFromWorker (state->game_id, str.c_str());
}

EMSCRIPTEN_KEEPALIVE void worker_manager_pause_worker ()
{

}

EMSCRIPTEN_KEEPALIVE void worker_manager_resume_worker ()
{

}
