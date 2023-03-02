#ifndef WISDOMCHESS_WEB_WORKER_HPP
#define WISDOMCHESS_WEB_WORKER_HPP

#include "web_types.hpp"
#include "game_settings.hpp"

namespace wisdom
{
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

#endif // WISDOMCHESS_WEB_WORKER_HPP
