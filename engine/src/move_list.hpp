#ifndef WISDOM_MOVE_LIST_HPP
#define WISDOM_MOVE_LIST_HPP

#include "global.hpp"
#include "move.hpp"

#include <iostream>

namespace wisdom
{
    using MoveVector = vector<Move>;

    struct move_list
    {
        Move* move_array;
        std::size_t capacity;
    };

    class MoveListAllocator
    {
    private:
        static constexpr std::size_t Initial_Size = 16;
        static constexpr std::size_t Size_Increment = 4;
        vector<unique_ptr<move_list>> my_move_list_ptrs;

    public:
        unique_ptr<move_list> alloc_move_list () noexcept;
        void dealloc_move_list (unique_ptr<move_list> move_list) noexcept;
    };

    std::size_t move_list_capacity (move_list& ptr) noexcept;

    Move* move_list_begin (move_list& ptr) noexcept;
    Move* move_list_end (move_list& ptr) noexcept;

    void move_list_append (move_list& list, std::size_t position, Move move) noexcept;

    class MoveList
    {
    private:
        unique_ptr<move_list> my_moves_list;
        std::size_t my_size = 0;
        gsl::not_null<MoveListAllocator*> my_allocator;

    public:
        explicit MoveList (gsl::not_null<MoveListAllocator*> allocator) :
            my_allocator { allocator }
        {
            my_moves_list = allocator->alloc_move_list ();
        }

        ~MoveList () 
        {
            if (my_moves_list)
                my_allocator->dealloc_move_list (std::move (my_moves_list));
        }

        MoveList (Color color, std::initializer_list<czstring> list) noexcept;

        // Delete special copy members:
        MoveList (const MoveList& other) = delete;
        MoveList& operator= (MoveList other) = delete;

        // Default move members:
        MoveList (MoveList&& other) = default;
        MoveList& operator= (MoveList&& other) = default;

        void push_back (Move move) noexcept
        {
            move_list_append (*my_moves_list, my_size, move);
            my_size++;
        }

        void pop_back () noexcept
        {
            assert (my_size > 0);
            my_size--;
        }

        [[nodiscard]] auto begin () const noexcept -> const Move*
        {
            return my_moves_list->move_array;
        }

        [[nodiscard]] auto end () const noexcept -> const Move*
        {
            // inline for performance:
            return my_moves_list->move_array + my_size;
        }

        [[nodiscard]] bool empty () const noexcept
        {
            return my_size == 0;
        }

        [[nodiscard]] size_t size () const noexcept
        {
            return my_size;
        }

        [[nodiscard]] size_t capacity () const noexcept
        {
            return move_list_capacity (*my_moves_list);
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

        [[nodiscard]] auto data () const noexcept -> Move*
        {
            return my_moves_list->move_array;
        }

        [[nodiscard]] auto ptr () const noexcept -> move_list*
        {
            return my_moves_list.get ();
        }
    };

    auto to_string (const MoveList& list) -> string;

    auto operator<< (std::ostream& os, const MoveList& list) ->
        std::ostream&;
}

#endif //WISDOM_MOVE_LIST_HPP
