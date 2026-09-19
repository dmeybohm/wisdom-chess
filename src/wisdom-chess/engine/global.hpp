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
#include <cassert>
#include <type_traits>
#include <random>
#include <source_location>

#include <gsl/gsl>
#include <gsl/narrow>

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
        = Num_Squares * WeightQueen *
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

    // Default max time spent searching.
    inline constexpr int Default_Max_Search_Seconds = 2;

    // Minimum amount behind the computer must feel in order to
    // accept a draw offer.
    inline constexpr int Min_Draw_Score = -500;

    template <typename T>
    [[nodiscard]] constexpr auto
    isNegative (T value) noexcept
        -> bool
    {
        if constexpr (std::is_signed_v<T>)
            return value < T {};
        else
            return false;
    }

    // Whether converting the value to Target and back preserves it.
    template <typename Target, typename Source>
    [[nodiscard]] constexpr auto
    isLosslessConversion (Source value) noexcept
        -> bool
    {
        auto converted = static_cast<Target> (value);
        return static_cast<Source> (converted) == value
            && isNegative (converted) == isNegative (value);
    }

    // constexpr version of narrow_cast (no exception at runtime):
    template <typename Target, typename Source> constexpr auto
    narrow_cast (Source value) noexcept
        -> Target
    {
        static_assert (std::is_arithmetic_v<Source>);
        static_assert (std::is_arithmetic_v<Target>);

        // Check if Source can fit into Target without truncation
        if (std::is_constant_evaluated())
        {
            if (!isLosslessConversion<Target> (value))
            {
                // At compile-time, trigger an error if there's truncation
                std::terminate();
            }
        }

        return gsl::narrow_cast<Target> (value);
    }

    // constexpr version of narrow (exception at runtime):
    template <typename Target, typename Source> constexpr auto
    narrow (Source value)
        -> Target
    {
        static_assert (std::is_arithmetic_v<Source>);
        static_assert (std::is_arithmetic_v<Target>);

        // Check if Source can fit into Target without truncation
        if (std::is_constant_evaluated())
        {
            if (!isLosslessConversion<Target> (value))
            {
                // At compile-time, trigger an error if there's truncation
                throw std::runtime_error ("narrow_cast: narrowing occurred");
            }
        }

        return gsl::narrow<Target> (value);
    }

    // Converts to a narrower unsigned type, deliberately discarding the high bits.
    template <typename Target, typename Source>
    [[nodiscard]] constexpr auto
    truncate (Source value) noexcept
        -> Target
    {
        static_assert (std::is_unsigned_v<Source> && std::is_unsigned_v<Target>);
        static_assert (sizeof (Target) <= sizeof (Source));

        return static_cast<Target> (value);
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
        struct Text
        {
            string message;
            string extra_info;
        };

        // Shared, so that copying the exception cannot throw.
        shared_ptr<const Text> my_text;

    public:
        Error (string message, string extra_info)
            : my_text { make_shared<const Text> (Text { std::move (message), std::move (extra_info) }) }
        {
        }

        explicit Error (string message) :
            Error (std::move (message), "")
        {}

        // Declared so that there is no move, which would leave my_text empty.
        Error (const Error& src) noexcept = default;
        auto operator= (const Error& src) noexcept -> Error& = default;

        [[nodiscard]] auto message() const noexcept -> const string&
        {
            return my_text->message;
        }

        [[nodiscard]] auto extra_info() const noexcept -> const string&
        {
            return my_text->extra_info;
        }

        [[nodiscard]] const char* what() const noexcept override
        {
            return my_text->message.c_str();
        }
    };

    class PreconditionError : public Error
    {
    public:
        using Error::Error;
    };

    class PostconditionError : public Error
    {
    public:
        using Error::Error;
    };

    [[noreturn]] void
    throwPreconditionError (const std::source_location& location);

    [[noreturn]] void
    throwPostconditionError (const std::source_location& location);

    // Throws PreconditionError when the condition is false. In a constant
    // expression, a false condition is a compile error instead.
    constexpr void
    expects (
        bool condition,
        const std::source_location& location = std::source_location::current()
    )
    {
        if (!condition) [[unlikely]]
            throwPreconditionError (location);
    }

    [[noreturn]] void
    terminateOnPreconditionFailure (const std::source_location& location) noexcept;

    // Prints the failure and terminates when the condition is false. For
    // noexcept functions, where expects() could not propagate its exception.
    constexpr void
    noexcept_expects (
        bool condition,
        const std::source_location& location = std::source_location::current()
    ) noexcept
    {
        if (!condition) [[unlikely]]
            terminateOnPreconditionFailure (location);
    }

    // Throws PostconditionError when the condition is false.
    constexpr void
    ensures (
        bool condition,
        const std::source_location& location = std::source_location::current()
    )
    {
        if (!condition) [[unlikely]]
            throwPostconditionError (location);
    }
}
