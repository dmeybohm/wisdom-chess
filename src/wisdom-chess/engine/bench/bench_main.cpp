#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <iostream>

namespace wisdom::bench
{
    void runMoveGenerationBenchmarks (ankerl::nanobench::Bench& bench);
    void runThreatBenchmarks (ankerl::nanobench::Bench& bench);
    void runLegalityBenchmarks (ankerl::nanobench::Bench& bench);
    void runPerftBenchmarks (ankerl::nanobench::Bench& bench);
}

auto main() -> int
{
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

    return 0;
}
