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

    public:
        static constexpr std::size_t Initial_Size = 16;
        static constexpr std::size_t Size_Increment = 32;

        static auto default_alloc_move_list () -> MoveListPtr
        {
            auto new_list = make_unique<Move[]> (Initial_Size + 1);
            new_list[0] = make_move_with_packed_capacity (Initial_Size);
            return new_list;
        }

        static void move_list_append (MoveListPtr& move_list_ptr, std::size_t position, Move move) noexcept
        {
            size_t old_capacity = extract_packed_capacity_from_move (move_list_ptr[0]);
            assert (position <= old_capacity);

            if (position == old_capacity)
            {
                size_t new_capacity = old_capacity + MoveListAllocator::Size_Increment;

                auto new_list = make_unique<Move[]> (new_capacity + 1);
                new_list[0] = make_move_with_packed_capacity (new_capacity);
                std::copy (&move_list_ptr[1], &move_list_ptr[1] + old_capacity, &new_list[1]);
                move_list_ptr = std::move (new_list);
            }

            move_list_ptr[position + 1] = move;
        }

        auto alloc_move_list() noexcept -> MoveListPtr
        {
            if (!my_move_list_ptrs.empty ())
            {
                auto move_list_end = std::move (my_move_list_ptrs.back ());
                my_move_list_ptrs.pop_back ();
                return move_list_end;
            }

            return default_alloc_move_list ();
        }

        void dealloc_move_list (MoveListPtr move_list) noexcept
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

        MoveList ()
            : my_allocator { nullptr }
        {
            my_moves_list = MoveListAllocator::default_alloc_move_list ();
        }

    public:
        explicit MoveList (not_null<MoveListAllocator*> allocator) :
            my_allocator { allocator }
        {
            my_moves_list = allocator->alloc_move_list ();
        }

        ~MoveList () 
        {
            if (my_moves_list != nullptr && my_allocator != nullptr)
            {
                my_allocator->dealloc_move_list (std::move (my_moves_list));
            }
        }

        // Public factory method for getting an uncached MoveList:
        static auto uncached () -> MoveList
        {
            return MoveList {};
        }

        MoveList (Color color, std::initializer_list<czstring> list) noexcept;

        // Delete special copy members:
        MoveList (const MoveList& other) = delete;
        MoveList& operator= (const MoveList& other) = delete;

        MoveList (MoveList&& other) noexcept = default;

        // Implement std::swappable
        friend void swap (MoveList& first, MoveList& second) noexcept
        {
            first.swap (second);
        }

        void swap (MoveList& other) noexcept
        {
            std::swap (my_moves_list, other.my_moves_list);
            std::swap (my_allocator, other.my_allocator);
            std::swap (my_size, other.my_size);
        }

        MoveList& operator= (MoveList&& other) noexcept = default;

        void push_back (Move move) noexcept
        {
            MoveListAllocator::move_list_append (my_moves_list, my_size, move);
            my_size++;
        }

        void pop_back () noexcept
        {
            assert (my_size > 0);
            my_size--;
        }

        [[nodiscard]] auto begin () const& noexcept -> const Move*
        {
            return &my_moves_list[1];
        }
        void begin () const&& = delete;

        [[nodiscard]] auto end () const& noexcept -> const Move*
        {
            return &my_moves_list[1] + my_size;
        }
        void end () const&& = delete;

        [[nodiscard]] auto cbegin () const& noexcept -> const Move*
        {
            return &my_moves_list[1];
        }
        void cbegin () const&& = delete;

        [[nodiscard]] auto cend () const& noexcept -> const Move*
        {
            return &my_moves_list[1] + my_size;
        }
        void cend () const&& = delete;

        [[nodiscard]] auto begin () & noexcept -> Move*
        {
            return &my_moves_list[1];
        }
        void begin () && = delete;

        [[nodiscard]] auto end () & noexcept -> Move*
        {
            return &my_moves_list[1] + my_size;
        }
        void end () && = delete;

        [[nodiscard]] bool empty () const noexcept
        {
            return my_size == 0;
        }

        [[nodiscard]] size_t size () const noexcept
        {
            return my_size;
        }

        [[nodiscard]] auto to_string () const -> string;

        auto operator== (const MoveList& other) const -> bool
        {
            return size () == other.size () && std::equal (
                    begin (),
                    end (),
                    other.begin (),
                    move_equals
            );
        }

        auto operator!= (const MoveList& other) const -> bool
        {
            return !(*this == other);
        }

        [[nodiscard]] auto data () const& noexcept -> Move*
        {
            return &my_moves_list[1];
        }
        void data () const&& = delete;

        [[nodiscard]] auto ptr () const& noexcept -> const MoveListPtr&
        {
            return my_moves_list;
        }
        void ptr () const&& = delete;

        [[nodiscard]] auto allocator () const& noexcept -> observer_ptr<MoveListAllocator>
        {
            return my_allocator;
        }
        void allocator () const&& = delete;
    };

    auto to_string (const MoveList& list) -> string;

    auto operator<< (std::ostream& os, const MoveList& list) ->
        std::ostream&;
}

#endif //WISDOM_MOVE_LIST_HPP
