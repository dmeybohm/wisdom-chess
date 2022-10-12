#ifndef WISDOM_MOVE_LIST_HPP
#define WISDOM_MOVE_LIST_HPP

#include "global.hpp"
#include "move.hpp"

#include <iostream>

namespace wisdom
{
    struct move_list
    {
        gsl::owner<Move*> move_array;
        std::size_t capacity;
    };

    class MoveListAllocator
    {
    private:
        vector<unique_ptr<move_list>> my_move_list_ptrs;

    public:
        ~MoveListAllocator()
        {
            for (auto& ptr : my_move_list_ptrs)
            {
                if (ptr && ptr->move_array)
                {
                    free (ptr->move_array);
                    ptr->move_array = nullptr;
                }
            }
        }

        static constexpr std::size_t Initial_Size = 16;
        static constexpr std::size_t Size_Increment = 32;

        unique_ptr<move_list> alloc_move_list () noexcept;
        void dealloc_move_list (unique_ptr<move_list> move_list) noexcept;
    };

    void move_list_append (move_list& list, std::size_t position, Move move) noexcept;

    inline auto default_alloc_move_list () -> unique_ptr<move_list>
    {
        auto list = new move_list ();
        list->move_array = (Move*)malloc (sizeof (Move) * MoveListAllocator::Initial_Size);
        list->capacity = MoveListAllocator::Initial_Size;

        return unique_ptr<move_list> { list };
    }

    class MoveList
    {
    private:
        unique_ptr<move_list> my_moves_list;
        std::size_t my_size = 0;
        observer_ptr<MoveListAllocator> my_allocator;

        MoveList ()
            : my_allocator { nullptr }
        {
            my_moves_list = default_alloc_move_list ();
        }

    public:
        explicit MoveList (not_null<MoveListAllocator*> allocator) :
            my_allocator { allocator }
        {
            my_moves_list = allocator->alloc_move_list ();
        }

        ~MoveList () 
        {
            if (my_moves_list != nullptr)
            {
                if (my_allocator != nullptr)
                {
                    my_allocator->dealloc_move_list (std::move (my_moves_list));
                }
                else
                {
                    free (my_moves_list->move_array);
                    my_moves_list->move_array = nullptr;
                }
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

        MoveList (MoveList&& other) noexcept
            : my_moves_list { std::move (other.my_moves_list) }
            , my_size { other.my_size }
            , my_allocator { other.my_allocator }
        {
        }

        MoveList& operator= (MoveList&& other) noexcept
        {
            if (this != &other)
            {
                if (my_moves_list != nullptr && my_moves_list->move_array != nullptr)
                {
                    if (my_allocator == nullptr)
                    {
                        free (my_moves_list->move_array);
                        my_moves_list->move_array = nullptr;
                    }
                    else
                    {
                        my_allocator->dealloc_move_list (std::move (my_moves_list));
                        my_moves_list = nullptr;
                    }
                }
                my_moves_list = std::move (other.my_moves_list);
                my_allocator = other.my_allocator;
                my_size = other.my_size;
                other.my_allocator = nullptr;
                other.my_size = 0;
            }
            return *this;
        }

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

        [[nodiscard]] auto begin () const& noexcept -> const Move*
        {
            return my_moves_list->move_array;
        }
        void begin () const&& = delete;

        [[nodiscard]] auto end () const& noexcept -> const Move*
        {
            return my_moves_list->move_array + my_size;
        }
        void end () const&& = delete;

        [[nodiscard]] auto cbegin () const& noexcept -> const Move*
        {
            return my_moves_list->move_array;
        }
        void cbegin () const&& = delete;

        [[nodiscard]] auto cend () const& noexcept -> const Move*
        {
            return my_moves_list->move_array + my_size;
        }
        void cend () const&& = delete;

        [[nodiscard]] auto begin () & noexcept -> Move*
        {
            return my_moves_list->move_array;
        }
        void begin () && = delete;

        [[nodiscard]] auto end () & noexcept -> Move*
        {
            return my_moves_list->move_array + my_size;
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

        [[nodiscard]] size_t capacity () const noexcept
        {
            return my_moves_list->capacity;
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
            return my_moves_list->move_array;
        }
        void data () const&& = delete;

        [[nodiscard]] auto ptr () const& noexcept -> move_list*
        {
            return my_moves_list.get ();
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
