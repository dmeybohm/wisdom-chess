#pragma once

#include "wisdom-chess/engine/global.hpp"

namespace wisdom
{
    // ASCII-only, constexpr replacements for the <cctype> functions, which
    // take an int that must be EOF or fit in an unsigned char and so cannot
    // be given a plain char above 0x7f.
    [[nodiscard]] constexpr auto
    isLower (char ch) noexcept
        -> bool
    {
        return ch >= 'a' && ch <= 'z';
    }

    [[nodiscard]] constexpr auto
    isUpper (char ch) noexcept
        -> bool
    {
        return ch >= 'A' && ch <= 'Z';
    }

    [[nodiscard]] constexpr auto
    isAlpha (char ch) noexcept
        -> bool
    {
        return isLower (ch) || isUpper (ch);
    }

    [[nodiscard]] constexpr auto
    isDigit (char ch) noexcept
        -> bool
    {
        return ch >= '0' && ch <= '9';
    }

    [[nodiscard]] constexpr auto
    isSpace (char ch) noexcept
        -> bool
    {
        return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\v' || ch == '\f' || ch == '\r';
    }

    [[nodiscard]] constexpr auto
    toLower (char ch) noexcept
        -> char
    {
        return isUpper (ch) ? static_cast<char> (ch + ('a' - 'A')) : ch;
    }

    [[nodiscard]] constexpr auto
    toUpper (char ch) noexcept
        -> char
    {
        return isLower (ch) ? static_cast<char> (ch - ('a' - 'A')) : ch;
    }

    // "Chomp" the last newline of a string.
    [[nodiscard]] auto 
    chomp (const string& str) 
        -> string;

    // Split the string into a vector of strings.
    [[nodiscard]] auto 
    split (const string& source, const string& separator) 
        -> vector<string>;

    // Join the vector of strings into a single string separated by separator.
    [[nodiscard]] auto 
    join (const vector<string>& strings, const string& separator) 
        -> string;

    // Convert the string to an integer.
    [[nodiscard]] auto 
    toInt (const string& str) 
        -> optional<int>;
}
