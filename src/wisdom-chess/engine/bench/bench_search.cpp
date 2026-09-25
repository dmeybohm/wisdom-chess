#include <nanobench.h>

#include <chrono>
#include <iostream>
#include <iomanip>
#include <random>
#include <string>
#include <vector>

#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/generate.hpp"
#include "wisdom-chess/engine/logger.hpp"
#include "wisdom-chess/engine/move_timer.hpp"
#include "wisdom-chess/engine/search.hpp"
#include "wisdom-chess/engine/transposition_table.hpp"

#include "bench_positions.hpp"

namespace wisdom::bench
{
    // Play a fixed sequence of non-capturing moves from the starting position,
    // so the search runs against a long game history.
    static auto gameWithHistory (int plies) -> Game
    {
        Game game = Game::createStandardGame();
        std::mt19937_64 random { 20260919 };

        for (int i = 0; i < plies; i++)
        {
            auto legal_moves = generateLegalMoves (game.getBoard(), game.getCurrentTurn());
            MoveList quiet_moves;

            for (auto move : legal_moves)
            {
                if (!move.isAnyCapturing())
                    quiet_moves.append (move);
            }

            if (quiet_moves.isEmpty())
                break;

            auto index = random() % quiet_moves.size();
            game.move (*(quiet_moves.begin() + index));
        }

        return game;
    }

    // Search to a fixed depth with whatever the table already holds.
    static auto searchWithTable (Game& game, TranspositionTable& table, int depth) -> SearchResult
    {
        MoveTimer timer { 600 };
        auto search = IterativeSearch::create (
            game.getBoard(),
            game.getHistory(),
            makeNullLogger(),
            timer,
            depth,
            table
        );

        return search.iterativelyDeepen (game.getCurrentTurn());
    }

    // Search to a fixed depth with an empty transposition table.
    static auto searchToDepth (Game& game, TranspositionTable& table, int depth) -> SearchResult
    {
        table.clear();
        return searchWithTable (game, table, depth);
    }

    static void printSeconds (const std::string& label, double seconds)
    {
        std::cout << "  " << label << ": "
                  << std::fixed << std::setprecision (3) << seconds << "s\n";
    }

    // The moves of one game, chosen by the engine at a shallow depth. Building
    // the line this way keeps it legal and the positions realistic, and it is
    // deterministic, so both replays below walk exactly the same positions.
    static auto scriptedGame (int plies, int script_depth) -> std::vector<Move>
    {
        auto table = TranspositionTable::fromMegabytes (
            TranspositionTable::Default_Size_In_Megabytes
        );
        Game game = Game::createStandardGame();
        std::vector<Move> script;

        for (int i = 0; i < plies; i++)
        {
            auto result = searchToDepth (game, table, script_depth);
            if (!result.move.has_value())
                break;

            script.push_back (*result.move);
            game.move (*result.move);
        }

        return script;
    }

    struct ReplayResult
    {
        double seconds = 0.0;
        std::vector<optional<Move>> chosen;
    };

    // Search every position of the script to a fixed depth. The scripted move
    // is played rather than the chosen one, so that clearing the table cannot
    // send the two replays down different games.
    static auto replayScript (
        const std::vector<Move>& script,
        int depth,
        bool clear_before_each_search
    )
        -> ReplayResult
    {
        auto table = TranspositionTable::fromMegabytes (
            TranspositionTable::Default_Size_In_Megabytes
        );
        Game game = Game::createStandardGame();
        ReplayResult result;

        auto start = std::chrono::steady_clock::now();

        for (auto scripted_move : script)
        {
            if (clear_before_each_search)
                table.clear();

            auto search_result = searchWithTable (game, table, depth);
            result.chosen.push_back (search_result.move);
            game.move (scripted_move);
        }

        auto end = std::chrono::steady_clock::now();
        result.seconds = std::chrono::duration<double> (end - start).count();

        return result;
    }

    // Measures what a table that survives between moves is worth: the same
    // consecutive positions searched with a table cleared before every search
    // and with one cleared only before the first.
    static void runWarmTableBenchmark (int plies, int depth)
    {
        auto script = scriptedGame (plies, 3);

        auto cold = replayScript (script, depth, true);
        auto warm = replayScript (script, depth, false);

        size_t differing = 0;
        for (size_t i = 0; i < cold.chosen.size(); i++)
        {
            if (cold.chosen[i] != warm.chosen[i])
                differing++;
        }

        std::cout << "search/warm-table (" << script.size() << " positions at depth "
                  << depth << "):\n";
        printSeconds ("cleared before every search", cold.seconds);
        printSeconds ("cleared only before the first", warm.seconds);
        std::cout << "  moves chosen differently: " << differing << " of "
                  << cold.chosen.size() << "\n";
    }

    void runSearchBenchmarks (ankerl::nanobench::Bench& bench)
    {
        struct Scenario
        {
            const char* name;
            Game game;
            int depth;
            int deep_depth;
        };

        Scenario scenarios[] = {
            { "starting", Game::createGameFromFen (Starting_Position_Fen), 6, 8 },
            { "kiwipete", Game::createGameFromFen (Kiwipete_Fen), 6, 8 },
            { "italian", Game::createGameFromFen (Italian_Game_Fen), 6, 8 },
            { "history-80-plies", gameWithHistory (80), 6, 8 },
            { "history-200-plies", gameWithHistory (200), 6, 8 },
        };

        auto table = TranspositionTable::fromMegabytes (
            TranspositionTable::Default_Size_In_Megabytes
        );

        auto previous_iterations = bench.minEpochIterations();
        bench.minEpochIterations (1);

        for (auto& scenario : scenarios)
        {
            auto label = std::string { "search/" } + scenario.name
                + "-depth" + std::to_string (scenario.depth);

            bench.run (label, [&] {
                auto result = searchToDepth (scenario.game, table, scenario.depth);
                ankerl::nanobench::doNotOptimizeAway (result);
            });
        }

        bench.minEpochIterations (previous_iterations);

        // Manual timing: one deeper search per scenario.
        for (auto& scenario : scenarios)
        {
            auto label = std::string { "search/" } + scenario.name
                + "-depth" + std::to_string (scenario.deep_depth);

            auto start = std::chrono::steady_clock::now();
            auto result = searchToDepth (scenario.game, table, scenario.deep_depth);
            auto end = std::chrono::steady_clock::now();
            ankerl::nanobench::doNotOptimizeAway (result);

            printSeconds (label, std::chrono::duration<double> (end - start).count());
        }

        runWarmTableBenchmark (30, 6);
    }

    void runSearchReport (int max_depth)
    {
        struct ReportPosition
        {
            const char* name;
            const char* fen;
        };

        const ReportPosition positions[] = {
            { "starting", Starting_Position_Fen },
            { "kiwipete", Kiwipete_Fen },
            { "italian", Italian_Game_Fen },
            { "position3", Position3_Fen },
            { "position4", Position4_Fen },
            { "middlegame", Quiet_Middlegame_Fen },
        };

        auto table = TranspositionTable::fromMegabytes (
            TranspositionTable::Default_Size_In_Megabytes
        );

        std::cout << std::left << std::setw (12) << "position"
                  << std::right << std::setw (6) << "depth"
                  << "  " << std::left << std::setw (10) << "move"
                  << std::right << std::setw (8) << "score"
                  << std::setw (8) << "from"
                  << std::setw (12) << "nodes"
                  << std::setw (12) << "qnodes"
                  << std::setw (10) << "seconds" << "\n";

        for (const auto& position : positions)
        {
            Game game = Game::createGameFromFen (position.fen);

            for (int depth = 1; depth <= max_depth; depth++)
            {
                auto start = std::chrono::steady_clock::now();
                auto result = searchToDepth (game, table, depth);
                auto end = std::chrono::steady_clock::now();
                auto seconds = std::chrono::duration<double> (end - start).count();

                std::cout << std::left << std::setw (12) << position.name
                          << std::right << std::setw (6) << depth
                          << "  " << std::left << std::setw (10)
                          << (result.move.has_value() ? asString (*result.move) : "(none)")
                          << std::right << std::setw (8) << result.score
                          << std::setw (8) << result.depth
                          << std::setw (12) << result.nodes
                          << std::setw (12) << result.quiescence_nodes
                          << std::setw (10) << std::fixed << std::setprecision (3)
                          << seconds << "\n";
            }
        }
    }
}
