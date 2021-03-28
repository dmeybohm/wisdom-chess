#ifndef WISDOM_GLOBAL_H
#define WISDOM_GLOBAL_H

#include <cstdint>
#include <cassert>
#include <exception>
#include <string>
#include <utility>

namespace wisdom
{
    constexpr int Num_Players = 2;

    constexpr int8_t Num_Rows = 8;
    constexpr int8_t Num_Columns = 8;

    constexpr int8_t First_Row = 0;
    constexpr int8_t First_Column = 0;

    constexpr int8_t Last_Row = 7;
    constexpr int8_t Last_Column = 7;

    constexpr int8_t King_Column = 4;
    constexpr int8_t King_Rook_Column = 7;

    constexpr int8_t Queen_Rook_Column = 0;
    constexpr int8_t King_Castled_Rook_Column = 5;
    constexpr int8_t Queen_Castled_Rook_Column = 3;

    // Infinity score.
    constexpr int Infinity = 65536;
    constexpr int Negative_Infinity = -1 * Infinity;

    // Initial Alpha value.
    constexpr int Initial_Alpha = Infinity * 3;

    // Errors in this application.
    class Error : public std::exception
    {
    private:
        const std::string my_message;

    public:
        explicit Error (std::string message) : my_message {std::move( message )}
        {}

        [[nodiscard]] const std::string& message() const noexcept
        {
            return my_message;
        }

        [[nodiscard]] const char *what () const noexcept override
        {
            return this->my_message.c_str();
        }
    };
}

#endif //WISDOM_GLOBAL_H
