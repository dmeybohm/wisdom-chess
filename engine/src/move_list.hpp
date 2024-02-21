#pragma once

#include "global.hpp"
#include "move.hpp"

#include <iostream>

namespace wisdom
{
    inline constexpr std::ptrdiff_t Max_Move_List_Size = 256 - sizeof (std::ptrdiff_t);

    class MoveList
    {
    private:
        array<Move, Max_Move_List_Size> my_moves;
        std::ptrdiff_t my_size = 0;

    public:
        MoveList() = default;

        MoveList (Color color, std::initializer_list<czstring> list) noexcept;

        // Delete special copy members:
        MoveList (const MoveList& other) = delete;
        MoveList& operator= (const MoveList& other) = delete;

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
            my_moves[my_size++] = move;
        }

        void removeLast() noexcept
        {
            Expects (my_size > 0);
            my_size--;
        }

        [[nodiscard]] auto begin() const& noexcept
        {
            return my_moves.begin();
        }
        void begin() const&& = delete;

        [[nodiscard]] auto end() const& noexcept
        {
            return my_moves.begin() + my_size;
        }
        void end() const&& = delete;

        [[nodiscard]] auto cbegin() const& noexcept
        {
            return my_moves.cbegin();
        }
        void cbegin() const&& = delete;

        [[nodiscard]] auto cend() const& noexcept
        {
            return my_moves.cbegin() + my_size;
        }
        void cend() const&& = delete;

        [[nodiscard]] auto begin() & noexcept
        {
            return my_moves.begin();
        }
        void begin() && = delete;

        [[nodiscard]] auto end() & noexcept
        {
            return my_moves.begin() + my_size;
        }
        void end() && = delete;

        [[nodiscard]] auto empty() const noexcept -> bool
        {
            return isEmpty();
        }

        MoveList (MoveList&& other) noexcept
        {
            std::copy (other.begin(), other.end(), begin());
            my_size = other.my_size;
            other.my_size = 0;
        }

        auto operator= (MoveList&& other) noexcept -> MoveList&
        {
            if (&other != this) {
                std::copy (other.begin(), other.end(), begin());
                my_size = other.my_size;
                other.my_size = 0;
            }
            return *this;
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

        void ptr() const&& = delete;
    };

    auto asString (const MoveList& list) -> string;

    auto operator<< (std::ostream& os, const MoveList& list) -> std::ostream&;
}