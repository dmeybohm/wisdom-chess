#pragma once

#include <emscripten/wasm_worker.h>

#include "wisdom-chess/ui/wasm/web_types.hpp"
#include "wisdom-chess/ui/wasm/game_settings.hpp"
#include "wisdom-chess/ui/wasm/bindings.hpp"
#include "wisdom-chess/ui/wasm/web_game.hpp"

namespace wisdom
{
    class GameModel
    {
    private:
        static int my_game_id;

        GameSettings my_game_settings {};

    public:
        [[nodiscard]] static auto
        currentGameId()
            -> int
        {
            return my_game_id;
        }

    private:
        // Send new settings to the worker.
        void sendSettings() const
        {
            int whitePlayer = static_cast<int> (my_game_settings.whitePlayer);
            int blackPlayer = static_cast<int> (my_game_settings.blackPlayer);

            emscripten_wasm_worker_post_function_sig (
                engine_thread,
                (void*)workerReceiveSettings,
                "iiii",
                whitePlayer,
                blackPlayer,
                my_game_settings.thinkingTime,
                my_game_settings.searchDepth
            );
        }

    public:
        GameModel() = default;

        // Initialize a new game with the default position.
        auto
        startNewGame()
            -> WebGame*
        {
            ++my_game_id;
            auto new_game = WebGame::newFromSettings (my_game_settings, my_game_id);
            emscripten_wasm_worker_post_function_vi (
                engine_thread,
                workerReinitializeGame,
                my_game_id
            );
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

        [[nodiscard]] auto 
        getCurrentGameSettings() const 
            -> GameSettings*
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

        [[nodiscard]] auto 
        getFirstHumanPlayerColor() 
            -> WebColor
        {
            if (my_game_settings.whitePlayer == Human)
                return WebColor::White;
            if (my_game_settings.blackPlayer == Human)
                return WebColor::Black;
            return WebColor::NoColor;
        }

        [[nodiscard]] auto 
        getSecondHumanPlayerColor() 
            -> WebColor
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

