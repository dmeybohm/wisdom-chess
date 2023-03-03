#ifndef WISDOMCHESS_WEB_WORKER_HPP
#define WISDOMCHESS_WEB_WORKER_HPP

#include "web_types.hpp"
#include "game_settings.hpp"
#include "bindings.hpp"

#include <emscripten/wasm_worker.h>

namespace wisdom
{
    class WebWorker
    {
    private:
        static int my_game_id;

        GameSettings my_game_settings {};

        // Send new settings to the worker.
        void sendSettings ()
        {
            int whitePlayer = static_cast<int> (my_game_settings.whitePlayer);
            int blackPlayer = static_cast<int> (my_game_settings.blackPlayer);

            emscripten_wasm_worker_post_function_sig (engine_thread,
                                                      (void*)worker_receive_settings,
                                                      "iiii",
                                                      whitePlayer,
                                                      blackPlayer,
                                                      my_game_settings.thinkingTime,
                                                      my_game_settings.searchDepth);
        }

    public:
        // Initialize a new game with the default position.
        void sendNewGame ()
        {
            emscripten_wasm_worker_post_function_vi (engine_thread,
                                                     worker_reinitialize_game,
                                                     ++my_game_id);
        }

        // Pause the worker.
        void sendPause()
        {
            emscripten_wasm_worker_post_function_v (engine_thread_manager,
                                                    worker_manager_pause_worker);
        }

        // Resume the worker.
        void sendResume()
        {
            emscripten_wasm_worker_post_function_v (engine_thread_manager,
                                                    worker_manager_resume_worker);
        }

        auto getCurrentGameSettings() -> GameSettings*
        {
            return new GameSettings { my_game_settings };
        }

        void setCurrentGameSettings (GameSettings* newSettings)
        {
            my_game_settings = GameSettings { *newSettings };
            sendPause();
            sendSettings();
            sendResume();
        }
    };
}

#endif // WISDOMCHESS_WEB_WORKER_HPP
