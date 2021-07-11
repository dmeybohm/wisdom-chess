#ifndef WISDOM_CHESS_GLOBAL_H
#define WISDOM_CHESS_GLOBAL_H

#include <cstdint>
#include <exception>
#include <string>
#include <utility>
#include <vector>
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
//#include <mutex>
//#include <thread>
#include <cctype>
#include <bitset>

namespace wisdom
{
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

    constexpr int King_Castled_Rook_Column = 5;
    constexpr int Queen_Castled_Rook_Column = 3;

    // Infinity score.
    constexpr int Infinity = 65536;
    constexpr int Negative_Infinity = -1 * Infinity;

    // Initial Alpha value.
    constexpr int Initial_Alpha = Infinity * 3;

    // Absolute max depth searched.
    constexpr int Max_Depth = 16;

    // Max time spent searching.
    constexpr int Max_Search_Seconds = 10;

    // Errors in this application.
    class Error : public std::exception
    {
    private:
        std::string my_message;
        std::string my_extra_info;

    public:
        explicit Error (const std::string &message) : my_message { message }, my_extra_info { "" }
        {}

        Error (const std::string &message, const std::string &extra_info) :
            my_message { message }, my_extra_info { extra_info }
        {}

        [[nodiscard]] const std::string& message() const noexcept
        {
            return my_message;
        }

        [[nodiscard]] const std::string& extra_info() const noexcept
        {
            return my_message;
        }

        [[nodiscard]] const char *what () const noexcept override
        {
            return this->my_message.c_str();
        }
    };

    class AssertionError : public Error
    {
    private:
        const std::string my_file;
        int my_line;

    public:
        AssertionError (const std::string &condition, const std::string &file, int line) :
            Error {"Assertion " + condition + " failed at " + file + ":" + std::to_string(line) + " !",
                   condition
            },
            my_file { file },
            my_line { line }
        {}

        [[nodiscard]] const std::string& file() const noexcept
        {
            return my_file;
        }

        [[nodiscard]] int line() const noexcept
        {
            return my_line;
        }
    };
}

#ifdef NDEBUG

#define assert(condition)  do { } while(0)

#else // NDEBUG

#define assert(condition) \
    do {                  \
        if (!(condition)) \
            throw AssertionError (#condition, __FILE__, __LINE__); \
    } while (0)

#endif // NDEBUG

#endif //WISDOM_CHESS_GLOBAL_H
