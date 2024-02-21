#pragma once

#include "global.hpp"
#include "move.hpp"

#include <iostream>

namespace wisdom
{
    inline constexpr std::ptrdiff_t Max_Move_List_Size = 256 - sizeof (std::ptrdiff_t);

    class MoveList // NOLINT(*-pro-type-member-init)
    {
    private:
        std::ptrdiff_t my_size = 0;
        array<Move, Max_Move_List_Size> my_moves;

    public:
        MoveList() = default;

        MoveList (Color color, std::initializer_list<czstring> list) noexcept;

        MoveList (const MoveList& other) // NOLINT(*-pro-type-member-init)
        {
            std::copy (other.my_moves.begin(), other.my_moves.begin() + other.my_size, my_moves.begin());
            my_size = other.my_size;
        }

        MoveList& operator= (const MoveList& other)
        {
            if (&other != this)
            {
                std::copy (other.my_moves.begin(), other.my_moves.begin() + other.my_size, my_moves.begin());
                my_size = other.my_size;
            }
            return *this;
        }

        void push_back (Move move) noexcept
        {
            append (move);
        }

        void pop_back() noexcept
        {
            removeLast();
        }

        void append (Move move) noexcept
        {
            Expects (my_size < Max_Move_List_Size);
            my_moves[my_size++] = move;
        }

        void removeLast() noexcept
        {
            Expects (my_size > 0);
            my_size--;
        }

        [[nodiscard]] auto begin() const noexcept
        {
            return my_moves.begin();
        }

        [[nodiscard]] auto end() const noexcept
        {
            return my_moves.begin() + my_size;
        }

        [[nodiscard]] auto cbegin() const noexcept
        {
            return my_moves.cbegin();
        }

        [[nodiscard]] auto cend() const noexcept
        {
            return my_moves.cbegin() + my_size;
        }

        [[nodiscard]] auto begin() noexcept
        {
            return my_moves.begin();
        }

        [[nodiscard]] auto end() noexcept
        {
            return my_moves.begin() + my_size;
        }

        [[nodiscard]] auto empty() const noexcept -> bool
        {
            return isEmpty();
        }

        [[nodiscard]] auto isEmpty() const noexcept -> bool
        {
            return my_size == 0;
        }

        [[nodiscard]] size_t size() const noexcept
        {
            return my_size;
        }

        [[nodiscard]] auto asString() const -> string;

        auto operator== (const MoveList& other) const -> bool
        {
            return size() == other.size() &&
                std::equal (begin(), end(), other.begin(), moveEquals);
        }

        auto operator!= (const MoveList& other) const -> bool
        {
            return !(*this == other);
        }

        [[nodiscard]] auto data() const& noexcept
        {
            return my_moves;
        }
        void data() const&& = delete;
    };

    auto asString (const MoveList& list) -> string;

    auto operator<< (std::ostream& os, const MoveList& list) -> std::ostream&;
}