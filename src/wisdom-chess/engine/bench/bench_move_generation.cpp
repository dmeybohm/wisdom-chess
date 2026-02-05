#include <nanobench.h>

#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/generate.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"

#include "bench_positions.hpp"

namespace wisdom::bench
{
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

    void runMoveGenerationBenchmarks (ankerl::nanobench::Bench& bench)
    {
        struct PositionInfo
        {
            const char* name;
            const char* fen;
        };

        PositionInfo positions[] = {
            { "starting",     Starting_Position_Fen },
            { "kiwipete",     Kiwipete_Fen },
            { "position3",    Position3_Fen },
            { "position4",    Position4_Fen },
            { "italian",      Italian_Game_Fen },
            { "many-queens",  Many_Queens_Fen },
        };

        for (const auto& pos : positions)
        {
            auto board = boardFromFen (pos.fen);
            auto color = colorFromFen (pos.fen);

            bench.run (
                string { "pseudolegal/" } + pos.name,
                [&] {
                    auto moves = generateAllPotentialMoves (board, color);
                    ankerl::nanobench::doNotOptimizeAway (moves);
                }
            );
        }

        for (const auto& pos : positions)
        {
            auto board = boardFromFen (pos.fen);
            auto color = colorFromFen (pos.fen);

            bench.run (
                string { "legal/" } + pos.name,
                [&] {
                    auto moves = generateLegalMoves (board, color);
                    ankerl::nanobench::doNotOptimizeAway (moves);
                }
            );
        }
    }
}
