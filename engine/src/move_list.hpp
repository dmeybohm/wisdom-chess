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
        constexpr MoveList() = default; // NOLINT(*-pro-type-member-init)

        constexpr MoveList (Color color, std::initializer_list<czstring> list) noexcept
            : MoveList {}
        {
            for (auto&& it : list)
            {
                append (moveParse (it, color));
                color = colorInvert (color);
            }
        }

        constexpr MoveList (const MoveList& other) // NOLINT(*-pro-type-member-init)
        {
            std::copy (
                other.my_moves.begin(),
                other.my_moves.begin() + other.my_size,
                my_moves.begin()
            );
            my_size = other.my_size;
        }

        [[nodiscard]] static constexpr auto
        fromZeroInitialized()
            -> MoveList
        {
            auto list = MoveList {};
            std::fill (
                std::begin (list.my_moves), 
                std::end (list.my_moves), 
                Move {} 
            );
            return list;
        }

        constexpr auto
        operator= (const MoveList& other)
            -> MoveList&
        {
            if (&other != this)
            {
                std::copy (
                    other.my_moves.begin(),
                    other.my_moves.begin() + other.my_size,
                    my_moves.begin()
                );
                my_size = other.my_size;
            }
            return *this;
        }

        constexpr void
        push_back (Move move) noexcept
        {
            append (move);
        }

        constexpr void
        pop_back() noexcept
        {
            removeLast();
        }

        constexpr void
        append (Move move) noexcept
        {
            my_moves[my_size++] = move;
        }

        constexpr void
        removeLast() noexcept
        {
            Expects (my_size > 0);
            my_size--;
        }

        [[nodiscard]]
        constexpr auto
        begin() const noexcept
        {
            return my_moves.begin();
        }

        [[nodiscard]] constexpr auto
        end() const noexcept
        {
            return my_moves.begin() + my_size;
        }

        [[nodiscard]] constexpr auto
        cbegin() const noexcept
        {
            return my_moves.cbegin();
        }

        [[nodiscard]] constexpr auto
        cend() const noexcept
        {
            return my_moves.cbegin() + my_size;
        }

        [[nodiscard]] constexpr auto
        begin() noexcept
        {
            return my_moves.begin();
        }

        [[nodiscard]] constexpr auto
        end() noexcept
        {
            return my_moves.begin() + my_size;
        }

        [[nodiscard]] constexpr auto
        empty() const noexcept
            -> bool
        {
            return isEmpty();
        }

        [[nodiscard]] constexpr auto
        isEmpty() const noexcept
            -> bool
        {
            return my_size == 0;
        }

        [[nodiscard]] constexpr auto
        size() const noexcept
            -> size_t
        {
            return my_size;
        }

        [[nodiscard]] auto asString() const -> string;

        constexpr auto
        operator== (const MoveList& other) const
            -> bool
        {
            return size() == other.size() &&
                std::equal (begin(), end(), other.begin());
        }

        constexpr auto
        operator!= (const MoveList& other) const
            -> bool
        {
            return !(*this == other);
        }

        [[nodiscard]] constexpr auto
        data() const& noexcept
        {
            return my_moves;
        }
        void data() const&& = delete;
    };

    auto asString (const MoveList& list) -> string;

    auto operator<< (std::ostream& os, const MoveList& list) -> std::ostream&;
}
