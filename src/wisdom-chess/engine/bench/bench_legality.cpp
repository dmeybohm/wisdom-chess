#include <nanobench.h>

#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/generate.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"

#include "wisdom-chess/engine/bench/bench_positions.hpp"

namespace wisdom::bench
{
    namespace
    {
        auto boardFromFen (czstring fen) -> Board
        {
            FenParser parser { fen };
            return parser.buildBoard();
        }

        auto colorFromFen (czstring fen) -> Color
        {
            FenParser parser { fen };
            return parser.getActivePlayer();
        }
    }

    void runLegalityBenchmarks (nonnull<ankerl::nanobench::Bench> bench)
    {
        struct PositionInfo
        {
            czstring name;
            czstring fen;
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

            bench->run (
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

            bench->run (
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
