#include <nanobench.h>

#include <chrono>
#include <iostream>
#include <iomanip>
#include <random>
#include <string>

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

    // Search to a fixed depth with an empty transposition table.
    static auto searchToDepth (Game& game, TranspositionTable& table, int depth) -> SearchResult
    {
        table.clear();

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

    static void printSeconds (const std::string& label, double seconds)
    {
        std::cout << "  " << label << ": "
                  << std::fixed << std::setprecision (3) << seconds << "s\n";
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
    }
}
