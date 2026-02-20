#include <nanobench.h>

#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/evaluate.hpp"
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

    void runLegalityBenchmarks (ankerl::nanobench::Bench& bench)
    {
        struct PositionInfo
        {
            const char* name;
            const char* fen;
        };

        PositionInfo positions[] = {
            { "starting",     Starting_Position_Fen },
            { "kiwipete",     Kiwipete_Fen },
            { "position4",    Position4_Fen },
        };

        // Board::withMove + isLegalPositionAfterMove for all pseudo-legal moves.
        for (const auto& pos : positions)
        {
            auto board = boardFromFen (pos.fen);
            auto color = colorFromFen (pos.fen);
            auto moves = generateAllPotentialMoves (board, color);

            bench.run (
                string { "withMove+legality/" } + pos.name,
                [&] {
                    int legal_count = 0;
                    for (auto move : moves)
                    {
                        Board new_board = board.withMove (color, move);
                        if (isLegalPositionAfterMove (new_board, color, move))
                            legal_count++;
                    }
                    ankerl::nanobench::doNotOptimizeAway (legal_count);
                }
            );
        }

        // Board::withMove alone: isolate copy cost.
        for (const auto& pos : positions)
        {
            auto board = boardFromFen (pos.fen);
            auto color = colorFromFen (pos.fen);
            auto moves = generateAllPotentialMoves (board, color);

            bench.run (
                string { "withMove-only/" } + pos.name,
                [&] {
                    Board last_board = board;
                    for (auto move : moves)
                    {
                        last_board = board.withMove (color, move);
                    }
                    ankerl::nanobench::doNotOptimizeAway (last_board);
                }
            );
        }
    }
}
