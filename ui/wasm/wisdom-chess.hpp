#ifndef WISDOMCHESS_WISDOM_CHESS_HPP
#define WISDOMCHESS_WISDOM_CHESS_HPP

#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <iostream>

#include "web_types.hpp"
#include "web_move.hpp"
#include "web_game.hpp"

namespace wisdom
{
    struct GameSettings
    {
        enum WebPlayer whitePlayer = Human;
        enum WebPlayer blackPlayer = ChessEngine;
        int thinkingTime = 2;
        int searchDepth = 3;
    };

    class WebWorker
    {
    private:
        GameSettings my_game_settings {};

    public:
        // Initialize a new game with the default position.
        void sendNewGame()
        {

        }

        // Send new settings to the worker.
        void sendSettings (GameSettings settings)
        {

        }

        // Pause the worker.
        void sendPause()
        {
        }

        // Resume the worker.
        void sendResume()
        {

        }

        auto getCurrentGameSettings() -> GameSettings*
        {
            return new GameSettings { my_game_settings };
        }

        void setCurrentGameSettings (GameSettings* newSettings)
        {
            my_game_settings  = GameSettings { *newSettings };
            sendPause ();
            sendSettings (my_game_settings);
            sendResume ();
        }
    };


}


#endif // WISDOMCHESS_WISDOM_CHESS_HPP
