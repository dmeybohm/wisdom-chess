#include <nanobench.h>

#include <chrono>
#include <iostream>
#include <iomanip>

#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/generate.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"

#include "bench_positions.hpp"

namespace wisdom::bench
{
    // Stripped-down perft: no capture/EP tracking, pure node count for speed.
    static auto perftCount (const Board& board, Color side, int depth) -> int64_t
    {
        if (depth == 0)
            return 1;

        int64_t nodes = 0;
        auto moves = generateAllPotentialMoves (board, side);

        for (auto move : moves)
        {
            Board new_board = board.withMove (side, move);
            if (!isLegalPositionAfterMove (new_board, side, move))
                continue;

            nodes += perftCount (new_board, colorInvert (side), depth - 1);
        }

        return nodes;
    }

    static auto boardFromFen (const char* fen) -> Board
    {
        FenParser parser { fen };
        return parser.buildBoard();
    }

    static auto colorFromFen (const char* fen) -> Color
    {
        FenParser parser { fen };
        return parser.getActivePlayer();
    }

    static void printNps (const char* label, int64_t nodes, double seconds)
    {
        double nps = seconds > 0.0 ? static_cast<double> (nodes) / seconds : 0.0;
        std::cout << "  " << label << ": "
                  << nodes << " nodes in "
                  << std::fixed << std::setprecision (3) << seconds << "s"
                  << " (" << std::fixed << std::setprecision (0) << nps << " NPS)\n";
    }

    void runPerftBenchmarks (ankerl::nanobench::Bench& bench)
    {
        // Nanobench: perft depth 4 starting position (~197K nodes).
        {
            auto board = boardFromFen (Starting_Position_Fen);
            auto color = colorFromFen (Starting_Position_Fen);

            bench.run ("perft/starting-depth4", [&] {
                auto nodes = perftCount (board, color, 4);
                ankerl::nanobench::doNotOptimizeAway (nodes);
            });
        }

        // Nanobench: perft depth 3 kiwipete (~97K nodes).
        {
            auto board = boardFromFen (Kiwipete_Fen);
            auto color = colorFromFen (Kiwipete_Fen);

            bench.run ("perft/kiwipete-depth3", [&] {
                auto nodes = perftCount (board, color, 3);
                ankerl::nanobench::doNotOptimizeAway (nodes);
            });
        }

        // Manual timing: perft depth 5 starting position (~4.8M nodes).
        {
            auto board = boardFromFen (Starting_Position_Fen);
            auto color = colorFromFen (Starting_Position_Fen);

            auto start = std::chrono::steady_clock::now();
            auto nodes = perftCount (board, color, 5);
            auto end = std::chrono::steady_clock::now();

            double seconds = std::chrono::duration<double> (end - start).count();
            printNps ("perft/starting-depth5", nodes, seconds);
        }

        // Manual timing: perft depth 4 kiwipete (~4M nodes).
        {
            auto board = boardFromFen (Kiwipete_Fen);
            auto color = colorFromFen (Kiwipete_Fen);

            auto start = std::chrono::steady_clock::now();
            auto nodes = perftCount (board, color, 4);
            auto end = std::chrono::steady_clock::now();

            double seconds = std::chrono::duration<double> (end - start).count();
            printNps ("perft/kiwipete-depth4", nodes, seconds);
        }
    }
}
