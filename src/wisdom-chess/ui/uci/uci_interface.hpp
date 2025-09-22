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

        auto run() -> void;

    private:
        auto processCommand (const string& line) -> void;
        auto handleUci() -> void;
        auto handleIsReady() -> void;
        auto handleNewGame() -> void;
        auto handlePosition (const vector<string>& tokens) -> void;
        auto handleGo (const vector<string>& tokens) -> void;
        auto handleStop() -> void;
        auto handleQuit() -> void;

        auto parsePosition (const vector<string>& tokens) -> bool;
        auto tokenizeCommand (const string& line) -> vector<string>;
        auto parseUciMove (const string& uci_move) -> optional<Move>;
        auto moveToUci (const Move& move) -> string;

        auto sendEngineInfo() -> void;
        auto sendBestMove (const optional<Move>& move) -> void;

        Game my_game;
        shared_ptr<Logger> my_logger;
        bool my_debug_mode = false;
    };
}