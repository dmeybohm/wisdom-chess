#ifndef WISDOM_CHESS_GSL_HPP
#define WISDOM_CHESS_GSL_HPP

namespace gsl
{
    template <typename T>
    using not_null = T;

    using zstring = char*;
    using czstring = const char*;

    template <typename T>
    using owner = T;

    template <typename T, typename U>
    constexpr auto narrow_cast(U anything) -> T
    {
        return static_cast<T>(anything);
    }

    template <typename T, typename U>
    constexpr auto narrow(U anything) -> T
    {
        return static_cast<T>(anything);
    }
}

#endif // WISDOM_CHESS_GSL_HPP
