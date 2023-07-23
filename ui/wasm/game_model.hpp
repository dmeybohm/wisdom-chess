#ifndef WISDOMCHESS_GAME_MODEL_HPP
#define WISDOMCHESS_GAME_MODEL_HPP

#include "web_types.hpp"
#include "game_settings.hpp"
#include "bindings.hpp"
#include "web_game.hpp"

#include <emscripten/wasm_worker.h>

namespace wisdom
{
    class GameModel
    {
    private:
        static int my_game_id;

        GameSettings my_game_settings {};

        // Send new settings to the worker.
        void sendSettings() const
        {
            int whitePlayer = static_cast<int> (my_game_settings.whitePlayer);
            int blackPlayer = static_cast<int> (my_game_settings.blackPlayer);

            emscripten_wasm_worker_post_function_sig (engine_thread, (void*)workerReceiveSettings,
                                                      "iiii",
                                                      whitePlayer,
                                                      blackPlayer,
                                                      my_game_settings.thinkingTime,
                                                      my_game_settings.searchDepth);
        }

    public:
        GameModel() = default;

        // Initialize a new game with the default position.
        WebGame* startNewGame()
        {
            auto new_game = WebGame::newFromSettings (my_game_settings);
            emscripten_wasm_worker_post_function_vi (engine_thread, workerReinitializeGame,
                                                     ++my_game_id);
            return new_game;
        }

        // Pause the worker.
        void sendPause() const
        {
            pauseWorker();
        }

        // Resume the worker.
        void sendUnpause() const
        {
            unpauseWorker();

            // Resume searching:
            emscripten_wasm_worker_post_function_v (engine_thread, startSearch);
        }

        void notifyHumanMove (const WebMove* move) const
        {
            emscripten_wasm_worker_post_function_vi (engine_thread, workerReceiveMove,
                                                     move->getMove().toInt());
        }

        void notifyComputerMove() const
        {
            emscripten_wasm_worker_post_function_v (engine_thread, startSearch);
        }

        [[nodiscard]] auto getCurrentGameSettings() const -> GameSettings*
        {
            return new GameSettings { my_game_settings };
        }

        void setCurrentGameSettings (GameSettings* newSettings)
        {
            my_game_settings = GameSettings { *newSettings };
            sendPause();
            sendSettings();
            sendUnpause();
        }

        [[nodiscard]] auto getFirstHumanPlayerColor() -> WebColor
        {
            if (my_game_settings.whitePlayer == Human)
                return WebColor::White;
            if (my_game_settings.blackPlayer == Human)
                return WebColor::Black;
            return WebColor::NoColor;
        }

        [[nodiscard]] auto getSecondHumanPlayerColor() -> WebColor
        {
            if (my_game_settings.whitePlayer == Human &&
                my_game_settings.blackPlayer == Human)
            {
                return WebColor::Black;
            }

            return WebColor::NoColor;
        }
    };
}

#endif // WISDOMCHESS_GAME_MODEL_HPP
