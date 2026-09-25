#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <iostream>
#include <string>

#include "wisdom-chess/engine/str.hpp"

namespace wisdom::bench
{
    void runMoveGenerationBenchmarks (ankerl::nanobench::Bench& bench);
    void runThreatBenchmarks (ankerl::nanobench::Bench& bench);
    void runLegalityBenchmarks (ankerl::nanobench::Bench& bench);
    void runPerftBenchmarks (ankerl::nanobench::Bench& bench);
    void runSearchBenchmarks (ankerl::nanobench::Bench& bench);
    void runSearchReport (int max_depth);
}

static void usage (const char* program)
{
    std::cerr << "usage: " << program << " [--search-report [max-depth]]\n";
}

auto main (int argc, char** argv) -> int
{
    if (argc > 1)
    {
        constexpr int Default_Report_Depth = 6;

        auto max_depth = argc > 2
            ? wisdom::toInt (argv[2])
            : std::optional<int> { Default_Report_Depth };

        if (std::string { argv[1] } != "--search-report" || argc > 3
            || !max_depth.has_value() || *max_depth < 1)
        {
            usage (argv[0]);
            return 1;
        }

        wisdom::bench::runSearchReport (*max_depth);
        return 0;
    }

    ankerl::nanobench::Bench bench;
    bench.warmup (3);
    bench.minEpochIterations (5);

    std::cout << "=== Wisdom Chess Benchmarks ===\n\n";

    std::cout << "--- Move Generation ---\n";
    wisdom::bench::runMoveGenerationBenchmarks (bench);

    std::cout << "\n--- Threat Detection ---\n";
    wisdom::bench::runThreatBenchmarks (bench);

    std::cout << "\n--- Legality Checking ---\n";
    wisdom::bench::runLegalityBenchmarks (bench);

    std::cout << "\n--- Perft ---\n";
    wisdom::bench::runPerftBenchmarks (bench);

    std::cout << "\n--- Search ---\n";
    wisdom::bench::runSearchBenchmarks (bench);

    return 0;
}
