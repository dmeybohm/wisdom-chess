#ifndef WISDOM_CHESS_GLOBAL_HPP
#define WISDOM_CHESS_GLOBAL_HPP

#include <cstdint>
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
#include <chrono>
#include <iosfwd>
#include <cctype>
#include <bitset>
#include <cassert>

#ifdef HAS_MS_GSL
#include <gsl/gsl>
#else
#include "gsl.hpp"
#endif

//#include <parallel_hashmap/phmap.h>

namespace wisdom
{
    using zstring = gsl::zstring;
    using czstring = gsl::czstring;
    using gsl::not_null;
    using std::string;
    using std::optional;
    using std::vector;
    using std::unique_ptr;
    using std::make_unique;
    using std::make_shared;
    using std::nullopt;
    using std::array;

    namespace chrono = std::chrono;

    constexpr int Num_Players = 2;

    constexpr int Num_Rows = 8;
    constexpr int Num_Columns = 8;

    constexpr int First_Row = 0;
    constexpr int First_Column = 0;

    constexpr int Last_Row = 7;
    constexpr int Last_Column = 7;

    constexpr int King_Column = 4;
    constexpr int King_Rook_Column = 7;
    constexpr int Queen_Rook_Column = 0;

    // Where the color is vulnerable to en passant:
    constexpr int White_En_Passant_Row = 5;
    constexpr int Black_En_Passant_Row = 2;

    constexpr int Kingside_Castled_King_Column = 6;
    constexpr int Queenside_Castled_King_Column = 2;
    constexpr int Kingside_Castled_Rook_Column = 5;
    constexpr int Queenside_Castled_Rook_Column = 3;

    // Infinity score.
    constexpr int Infinity = 65536;
    constexpr int Negative_Infinity = -1 * Infinity;

    // Initial Alpha value.
    constexpr int Initial_Alpha = Infinity * 3;

    // Absolute max depth searched.
    constexpr int Max_Depth = 16;

    // Max time spent searching.
    constexpr int Max_Search_Seconds = 5;

    // Minimum amount behind the computer must feel in order to
    // accept a draw offer.
    constexpr int Min_Draw_Score = -300;

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

        [[nodiscard]] auto message () const noexcept -> const string&
        {
            return my_message;
        }

        [[nodiscard]] auto extra_info () const noexcept -> const string&
        {
            return my_message;
        }

        [[nodiscard]] const char* what () const noexcept override
        {
            return this->my_message.c_str();
        }
    };
}

#endif //WISDOM_CHESS_GLOBAL_HPP
