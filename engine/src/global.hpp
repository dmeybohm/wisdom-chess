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
#include <bitset>
#include <cassert>
#include <type_traits>
#include <random>

#include <gsl/gsl>
#include <gsl/narrow>
#include <type_safe/flag.hpp>
#include <type_safe/flag_set.hpp>

namespace doctest
{
    class String;
}

namespace wisdom
{
    using zstring = gsl::zstring;
    using czstring = gsl::czstring;
    using gsl::not_null;
    using std::array;
    using std::bitset;
    using std::make_shared;
    using std::make_unique;
    using std::nullopt;
    using std::optional;
    using std::pair;
    using std::string;
    using std::unique_ptr;
    using std::shared_ptr;
    using std::vector;
    using std::string_view;
    using std::span;

    template <typename T>
    using observer_ptr = T*;

    template <typename T>
    using nonnull_observer_ptr = gsl::not_null<observer_ptr<T>>;

    namespace chrono = std::chrono;

    enum MaterialWeight
    {
        WeightNone = 0,
        WeightKing = 1500,
        WeightQueen = 1000,
        WeightRook = 500,
        WeightBishop = 320,
        WeightKnight = 305,
        WeightPawn = 100,
    };

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

    inline constexpr int Kingside_Castled_King_Column = 6;
    inline constexpr int Queenside_Castled_King_Column = 2;
    inline constexpr int Kingside_Castled_Rook_Column = 5;
    inline constexpr int Queenside_Castled_Rook_Column = 3;

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
        = Num_Squares * WeightQueen *
        std::max (Material_Score_Scale, Position_Score_Scale);
    static_assert (Max_Non_Checkmate_Score > 100'000);
    static_assert (Max_Non_Checkmate_Score * 2 < Initial_Alpha);

    // Default absolute max depth searched.
    inline constexpr int Default_Max_Depth = 16;

    // Default max time spent searching.
    inline constexpr int Default_Max_Search_Seconds = 5;

    // Minimum amount behind the computer must feel in order to
    // accept a draw offer.
    inline constexpr int Min_Draw_Score = -500;

    // constexpr versoin of narrow_cast (no exception at runtime:
    template <typename Target, typename Source> constexpr auto
    narrow_cast (Source value) noexcept
        -> Target
    {
        static_assert (std::is_arithmetic_v<Source>);
        static_assert (std::is_arithmetic_v<Target>);

        // Check if Source can fit into Target without truncation
        if (std::is_constant_evaluated())
        {
            if (value < std::numeric_limits<Target>::min() ||
                value > std::numeric_limits<Target>::max())
            {
                // At compile-time, trigger an error if there's truncation
                std::terminate ();
            }
        }

        return gsl::narrow_cast<Target> (value);
    }

    // constexpr versoin of narrow (exception at runtime):
    template <typename Target, typename Source> constexpr auto
    narrow (Source value)
        -> Target
    {
        static_assert (std::is_arithmetic_v<Source>);
        static_assert (std::is_arithmetic_v<Target>);

        // Check if Source can fit into Target without truncation
        if (std::is_constant_evaluated())
        {
            if (value < std::numeric_limits<Target>::min() ||
                value > std::numeric_limits<Target>::max())
            {
                // At compile-time, trigger an error if there's truncation
                throw std::runtime_error("narrow_cast: narrowing occurred");
            }
        }

        return gsl::narrow<Target> (value);
    }

    // constexpr version of tolower():
    constexpr auto
    toLower (int ch) noexcept
        -> int
    {
        if (ch >= 'A' && ch <= 'Z')
        {
            return ch + ('a' - 'A');
        }
        return ch;
    }

    // Errors in this application.
    class Error : public std::exception
    {
    private:
        string my_message;
        string my_extra_info;

    public:
        Error (string message, string extra_info) noexcept
            : my_message { std::move (message) }
            , my_extra_info { std::move (extra_info) }
        {
        }

        explicit Error (string message) noexcept :
            Error (std::move (message), "")
        {}

        Error (const Error& src) noexcept
            : Error (src.my_message, src.my_extra_info)
        {}

        [[nodiscard]] auto message() const noexcept -> const string&
        {
            return my_message;
        }

        [[nodiscard]] auto extra_info() const noexcept -> const string&
        {
            return my_message;
        }

        [[nodiscard]] const char* what() const noexcept override
        {
            return this->my_message.c_str();
        }
    };

}
