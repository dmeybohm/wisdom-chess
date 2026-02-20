# Benchmark Move Generation & Legality Checking

## Goal
Add a benchmarking suite that measures position generation rate (NPS) and move
legality checking performance, isolated from transposition tables, search, and
evaluation.

## Approach
Use **nanobench** (header-only, via CPM) for micro-benchmarks, plus manual
timing for deeper perft runs. Create a new `bench/` directory alongside `test/`
under the engine.

## Benchmark Categories

### Move Generation
- `generateAllPotentialMoves()` on each position (pseudo-legal)
- `generateLegalMoves()` on each position (includes legality filtering)

### Threat Detection
- `isKingThreatened()` where king is NOT threatened (common fast path)
- `isKingThreatened()` where king IS threatened (worst case)
- Sweep all 64 squares on a complex position (amortized average)

### Legality
- `Board::withMove` + `isLegalPositionAfterMove` for all pseudo-legal moves
- `Board::withMove` alone (isolate copy cost vs threat detection cost)

### Perft (Headline NPS Metric)
- Stripped-down `perftCount()` function (no capture/EP tracking, pure speed)
- Perft depth 4 starting position (~197K nodes) via nanobench
- Perft depth 3 kiwipete (~97K nodes) via nanobench
- Perft depth 5 starting position (~4.8M nodes) with manual timing + NPS output
- Perft depth 4 kiwipete (~4M nodes) with manual timing + NPS output

## Test Positions
- Starting position
- Kiwipete (complex midgame, many captures/checks)
- Position 3 & 4 from chessprogramming.org perft suite
- Dense midgame (Italian Game)
- Many-queens endgame (heavy threat checking)

## Key Design Decisions

- **nanobench over raw chrono**: Automatic iteration tuning, outlier detection,
  `doNotOptimizeAway()`, Linux perf counter integration
- **Separate from perft test suite**: Existing `perft.cpp`/`perft_test.cpp`
  track correctness. Benchmarks track *speed* — a different concern
- **No TT, no search, no eval**: The `perftCount()` function only calls
  `generateAllPotentialMoves`, `Board::withMove`, and
  `isLegalPositionAfterMove`
