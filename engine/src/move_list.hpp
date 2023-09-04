#ifndef WISDOM_MOVE_LIST_HPP
#define WISDOM_MOVE_LIST_HPP

#include "global.hpp"
#include "move.hpp"

#include <iostream>

namespace wisdom
{
    using MoveListPtr = unique_ptr<Move[]>;

    class MoveListAllocator
    {
    private:
        vector<MoveListPtr> my_move_list_ptrs;

        MoveListAllocator() = default;

    public:
        static constexpr std::size_t Initial_Size = 16;
        static constexpr std::size_t Size_Increment = 32;

        static auto makeUnique() -> unique_ptr<MoveListAllocator>
        {
            // to keep constructor private, use raw  new list instead
            // of making std::unique_ptr a friend function:
            return unique_ptr<MoveListAllocator> (new MoveListAllocator());
        }

        static auto defaultAllocMoveList() -> MoveListPtr
        {
            auto new_list = std::make_unique<Move[]> (Initial_Size + 1);
            new_list[0] = Move::makeAsPackedCapacity (Initial_Size);
            return new_list;
        }

        static void moveListAppend (MoveListPtr& move_list_ptr, std::size_t position,
                                    Move move) noexcept
        {
            size_t old_capacity = move_list_ptr[0].toUnpackedCapacity();
            assert (position <= old_capacity);

            if (position == old_capacity)
            {
                size_t new_capacity = old_capacity * 1.25;
                Expects (new_capacity + 1 > old_capacity);

                auto new_list = std::make_unique<Move[]> (new_capacity + 1);
                new_list[0] = Move::makeAsPackedCapacity (new_capacity);
                std::copy (&move_list_ptr[1], &move_list_ptr[1] + old_capacity, &new_list[1]);
                move_list_ptr = std::move (new_list);
            }

            move_list_ptr[position + 1] = move;
        }

        auto allocMoveList() noexcept -> MoveListPtr
        {
            if (!my_move_list_ptrs.empty())
            {
                auto move_list_end = std::move (my_move_list_ptrs.back());
                my_move_list_ptrs.pop_back();
                return move_list_end;
            }

            return defaultAllocMoveList();
        }

        void deallocMoveList (MoveListPtr move_list) noexcept
        {
            if (move_list != nullptr)
                my_move_list_ptrs.emplace_back (std::move (move_list));
        }
    };

    class MoveList
    {
    private:
        MoveListPtr my_moves_list;
        std::size_t my_size = 0;
        observer_ptr<MoveListAllocator> my_allocator;

        MoveList() : my_allocator { nullptr }
        {
            my_moves_list = MoveListAllocator::defaultAllocMoveList();
        }

    public:
        explicit MoveList (not_null<MoveListAllocator*> allocator) : my_allocator { allocator }
        {
            my_moves_list = allocator->allocMoveList();
        }

        ~MoveList()
        {
            if (my_moves_list != nullptr && my_allocator != nullptr)
            {
                my_allocator->deallocMoveList (std::move (my_moves_list));
            }
        }

        // Public factory method for getting an uncached MoveList:
        static auto uncached() -> MoveList
        {
            return MoveList {};
        }

        MoveList (Color color, std::initializer_list<czstring> list) noexcept;

        // Delete special copy members:
        MoveList (const MoveList& other) = delete;
        MoveList& operator= (const MoveList& other) = delete;

        MoveList (MoveList&& other) noexcept = default;
        MoveList& operator= (MoveList&& other) noexcept = default;

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
            MoveListAllocator::moveListAppend (my_moves_list, my_size, move);
            my_size++;
        }

        void removeLast() noexcept
        {
            Expects (my_size > 0);
            my_size--;
        }

        [[nodiscard]] auto begin() const& noexcept -> const Move*
        {
            return &my_moves_list[1];
        }
        void begin() const&& = delete;

        [[nodiscard]] auto end() const& noexcept -> const Move*
        {
            return &my_moves_list[1] + my_size;
        }
        void end() const&& = delete;

        [[nodiscard]] auto cbegin() const& noexcept -> const Move*
        {
            return &my_moves_list[1];
        }
        void cbegin() const&& = delete;

        [[nodiscard]] auto cend() const& noexcept -> const Move*
        {
            return &my_moves_list[1] + my_size;
        }
        void cend() const&& = delete;

        [[nodiscard]] auto begin() & noexcept -> Move*
        {
            return &my_moves_list[1];
        }
        void begin() && = delete;

        [[nodiscard]] auto end() & noexcept -> Move*
        {
            return &my_moves_list[1] + my_size;
        }
        void end() && = delete;

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
            return size() == other.size() && std::equal (begin(), end(), other.begin(), moveEquals);
        }

        auto operator!= (const MoveList& other) const -> bool
        {
            return !(*this == other);
        }

        [[nodiscard]] auto data() const& noexcept -> Move*
        {
            return &my_moves_list[1];
        }
        void data() const&& = delete;

        [[nodiscard]] auto ptr() const& noexcept -> const MoveListPtr&
        {
            return my_moves_list;
        }
        void ptr() const&& = delete;

        [[nodiscard]] auto allocator() const& noexcept -> observer_ptr<MoveListAllocator>
        {
            return my_allocator;
        }
        void allocator() const&& = delete;
    };

    auto asString (const MoveList& list) -> string;

    auto operator<< (std::ostream& os, const MoveList& list) -> std::ostream&;
}

#endif // WISDOM_MOVE_LIST_HPP
