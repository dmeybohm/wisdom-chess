#pragma once

namespace wisdom::bench
{
    // Starting position.
    inline constexpr auto Starting_Position_Fen
        = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    // Kiwipete: complex midgame with many captures, checks, and pins.
    inline constexpr auto Kiwipete_Fen
        = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";

    // Position 3 from chessprogramming.org perft suite.
    inline constexpr auto Position3_Fen
        = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";

    // Position 4 from chessprogramming.org perft suite.
    inline constexpr auto Position4_Fen
        = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";

    // Dense midgame: Italian Game Giuoco Piano.
    inline constexpr auto Italian_Game_Fen
        = "r1bqk2r/pppp1ppp/2n2n2/2b1p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4";

    // Many-queens endgame: heavy threat-checking load.
    inline constexpr auto Many_Queens_Fen
        = "Q3k3/3Q1Q2/8/8/8/8/2q1q3/3K4 w - - 0 1";
}
