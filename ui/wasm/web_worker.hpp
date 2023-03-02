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
        GameSettings my_game_settings {};

        // Send new settings to the worker.
        void sendSettings (GameSettings settings)
        {
            int whitePlayer = static_cast<int> (settings.whitePlayer);
            int blackPlayer = static_cast<int> (settings.blackPlayer);

            emscripten_wasm_worker_post_function_sig (engine_thread,
                                                      (void*)worker_receive_settings,
                                                      "iiii",
                                                      whitePlayer, blackPlayer,
                                                      settings.thinkingTime, settings.searchDepth);

        }

    public:
        // Initialize a new game with the default position.
        void sendNewGame (int newGameId)
        {

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
            sendPause ();
            sendSettings (my_game_settings);
            sendResume ();
        }
    };
}

#endif // WISDOMCHESS_WEB_WORKER_HPP
