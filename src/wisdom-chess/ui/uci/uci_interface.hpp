#pragma once

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/move_timer.hpp"
#include "wisdom-chess/engine/transposition_table.hpp"

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
        static constexpr int Default_Move_Overhead_Ms = 10;
        static constexpr int Max_Move_Overhead_Ms = 5000;

        int hash_size_mb = TranspositionTable::Default_Size_In_Megabytes;
        int default_depth = Default_Max_Depth;

        // Clock time kept back for everything outside the search: starting
        // it, answering, and the delay before the GUI reads the answer.
        int move_overhead_ms = Default_Move_Overhead_Ms;
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

        // The last "position" command applied, so that the next one can be
        // recognized as continuing the same game or not.
        vector<string> my_position_tokens;

        shared_ptr<Logger> my_logger;
        bool my_debug_mode = false;

        std::mutex my_game_mutex;
        std::atomic<int> my_search_id { 0 };
        std::atomic<bool> my_stop_requested { false };
        std::thread my_search_thread;

        UciSettings my_settings;

        // Declared after my_settings so that it can be sized from it. The
        // search thread owns it while it runs; the main thread touches it
        // only after waitForSearchThread().
        TranspositionTable my_transposition_table;
    };

    // A logger that writes UCI "info" lines to standard output.
    [[nodiscard]] auto
    makeUciLogger (bool debug_enabled)
        -> std::shared_ptr<Logger>;
}
