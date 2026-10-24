#include <nanobench.h>

#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/threats.hpp"

#include "wisdom-chess/engine/bench/bench_positions.hpp"

namespace wisdom::bench
{

    void runThreatBenchmarks (nonnull<ankerl::nanobench::Bench> bench)
    {
        // King NOT threatened (common fast path): starting position, white king.
        {
            auto board = Board::fromFen (Starting_Position_Fen);
            auto king_coord = board.getKingPosition (Color::White);

            bench->run ("isKingThreatened/not-threatened", [&] {
                auto result = isKingThreatened (board, Color::White, king_coord);
                ankerl::nanobench::doNotOptimizeAway (result);
            });
        }

        // King IS threatened: position 4, black king is in check.
        {
            auto board = Board::fromFen (Position4_Fen);
            // In position 4, black's king on e8 is threatened.
            auto king_coord = board.getKingPosition (Color::Black);

            bench->run ("isKingThreatened/threatened", [&] {
                auto result = isKingThreatened (board, Color::Black, king_coord);
                ankerl::nanobench::doNotOptimizeAway (result);
            });
        }

        // Sweep all 64 squares: complex position (kiwipete), amortized average.
        {
            auto board = Board::fromFen (Kiwipete_Fen);

            bench->run ("isKingThreatened/sweep-64-squares", [&] {
                int count = 0;
                for (auto coord : Board::allCoords())
                {
                    count += isKingThreatened (board, Color::White, coord) ? 1 : 0;
                }
                ankerl::nanobench::doNotOptimizeAway (count);
            });
        }

        // Many-queens position: heavy threat-checking load.
        {
            auto board = Board::fromFen (Many_Queens_Fen);
            auto king_coord = board.getKingPosition (Color::Black);

            bench->run ("isKingThreatened/many-queens", [&] {
                auto result = isKingThreatened (board, Color::Black, king_coord);
                ankerl::nanobench::doNotOptimizeAway (result);
            });
        }
    }
}
