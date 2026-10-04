#pragma once

#include <cstdint>
#include <limits>
#include <exception>
#include <string>
#include <utility>
#include <vector>
#include <iterator>
#include <stdexcept>
#include <array>
#include <optional>
#include <memory>
#include <list>
#include <unordered_map>
#include <functional>
#include <forward_list>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <iosfwd>
#include <cctype>
#include <type_traits>
#include <random>
#include <source_location>

#include "wisdom-chess/engine/cast.hpp"
#include "wisdom-chess/engine/error.hpp"
#include "wisdom-chess/engine/ptr.hpp"
#include "wisdom-chess/engine/types.hpp"

namespace wisdom
{
    inline constexpr int Weight_None = 0;
    inline constexpr int Weight_King = 1500;
    inline constexpr int Weight_Queen = 1000;
    inline constexpr int Weight_Rook = 500;
    inline constexpr int Weight_Bishop = 320;
    inline constexpr int Weight_Knight = 305;
    inline constexpr int Weight_Pawn = 100;

    inline constexpr int Num_Players = 2;

    inline constexpr int Num_Rows = 8;
    inline constexpr int Num_Columns = 8;
    inline constexpr int Num_Squares = Num_Rows * Num_Columns;

    inline constexpr int First_Row = 0;
    inline constexpr int First_Column = 0;

    inline constexpr int Last_Row = 7;
    inline constexpr int Last_Column = 7;

    inline constexpr int King_Column = 4;
    inline constexpr int King_Rook_Column = 7;
    inline constexpr int Queen_Rook_Column = 0;

    // Where the color is vulnerable to en passant:
    inline constexpr int White_En_Passant_Row = 5;
    inline constexpr int Black_En_Passant_Row = 2;

    // Where a pawn starts, and the row it must stand on to capture en passant.
    inline constexpr int White_Pawn_Start_Row = 6;
    inline constexpr int Black_Pawn_Start_Row = 1;
    inline constexpr int White_Pawn_En_Passant_Capture_Row = 3;
    inline constexpr int Black_Pawn_En_Passant_Capture_Row = 4;

    inline constexpr int Kingside_Castled_King_Column = 6;
    inline constexpr int Queenside_Castled_King_Column = 2;
    inline constexpr int Kingside_Castled_Rook_Column = 5;
    inline constexpr int Queenside_Castled_Rook_Column = 3;

    // Largest move clocks accepted as input. No legal game is this long.
    inline constexpr int Max_Half_Move_Clock = 10'000;
    inline constexpr int Max_Full_Move_Number = 10'000;

    // Scale factor for the material and position scale. Used for balancing material
    // and position scores together.
    inline constexpr int Material_Score_Scale = 2;
    inline constexpr int Position_Score_Scale = 9;

    // Initial Alpha value for alpha-beta search.
    inline constexpr int Initial_Alpha = std::numeric_limits<int>::max() / 3;

    // Infinite score - regular scores can never be this high.
    // Checkmates are scored above this, depending on how far
    // away from the current position they are.
    inline constexpr int Max_Non_Checkmate_Score
        = Num_Squares * Weight_Queen *
        std::max (Material_Score_Scale, Position_Score_Scale);
    static_assert (Max_Non_Checkmate_Score > 100'000);
    static_assert (Max_Non_Checkmate_Score * 2 < Initial_Alpha);

    // Base checkmate score. Uses linear scoring: closer checkmates subtract
    // fewer moves (checkmateScoreInMoves(n) = Checkmate_Score - n).
    // This allows transposition table mate score adjustments to work correctly.
    inline constexpr int Checkmate_Score = Max_Non_Checkmate_Score * 2;
    static_assert (Checkmate_Score > Max_Non_Checkmate_Score);
    static_assert (Checkmate_Score < Initial_Alpha);

    // Default absolute max depth searched.
    inline constexpr int Default_Max_Depth = 16;

    // The deepest search a caller may ask for, in plies. The search's
    // per-ply tables are this long.
    inline constexpr int Max_Search_Depth = 64;
    static_assert (Default_Max_Depth <= Max_Search_Depth);

    // Default max time spent searching.
    inline constexpr int Default_Max_Search_Seconds = 2;

    // The computer accepts a draw offer only when its evaluation is at or
    // below this score, in pawns times the material scale.
    inline constexpr int Min_Draw_Score = -500;

    // What the search scores a draw it could claim on its own move, by
    // repetition or the fifty-move rule. Negative, so the engine plays on
    // unless it is this far behind. Independent of Min_Draw_Score, which
    // governs draw offers rather than search.
    inline constexpr int Search_Draw_Contempt = -500;
}
