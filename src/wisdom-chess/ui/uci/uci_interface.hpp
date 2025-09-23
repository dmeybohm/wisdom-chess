#pragma once

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/move.hpp"

#include <iostream>
#include <sstream>
#include <vector>

namespace wisdom
{
    class Logger;

    class UciInterface
    {
    public:
        UciInterface();
        ~UciInterface() = default;

        void run();

    private:
        void processCommand (const string& line);
        void handleUci();
        void handleIsReady();
        void handleNewGame();
        void handlePosition (const vector<string>& tokens);
        void handleGo (const vector<string>& tokens);
        void handleStop();
        void handleQuit();

        auto parsePosition (const vector<string>& tokens) -> bool;
        auto tokenizeCommand (const string& line) -> vector<string>;
        auto parseUciMove (const string& uci_move) -> optional<Move>;
        auto moveToUci (const Move& move) -> string;

        void sendEngineInfo();
        void sendBestMove (const optional<Move>& move);

        Game my_game;
        shared_ptr<Logger> my_logger;
        bool my_debug_mode = false;
    };
}
