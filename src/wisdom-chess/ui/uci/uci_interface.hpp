#pragma once

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/move_timer.hpp"

#include <atomic>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

namespace wisdom
{
    class Logger;

    struct UciSettings
    {
        int hash_size_mb = 16;
        int default_depth = Default_Max_Depth;
    };

    class UciInterface
    {
    public:
        UciInterface();
        ~UciInterface();

        void run();

    private:
        void processCommand (const string& line);
        void handleUci();
        void handleIsReady();
        void handleNewGame();
        void handlePosition (const vector<string>& tokens);
        void handleGo (const vector<string>& tokens);
        void handleSetOption (const vector<string>& tokens);
        void handleStop();
        void handleQuit();

        auto parsePosition (const vector<string>& tokens) -> bool;
        auto tokenizeCommand (const string& line) -> vector<string>;
        auto parseUciMove (const string& uci_move) -> optional<Move>;
        auto moveToUci (const Move& move) -> string;

        void sendEngineInfo();
        void sendBestMove (const optional<Move>& move);

        [[nodiscard]] auto
        buildNotifier (int initial_search_id)
            -> MoveTimer::PeriodicFunction;

        void waitForSearchThread();

        Game my_game;
        shared_ptr<Logger> my_logger;
        bool my_debug_mode = false;

        std::mutex my_game_mutex;
        std::atomic<int> my_search_id { 0 };
        std::thread my_search_thread;

        UciSettings my_settings;
    };
}
