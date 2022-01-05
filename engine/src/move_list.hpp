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
        Move *move_array;
        std::size_t   size;
        std::size_t   capacity;
    };

    unique_ptr<move_list> alloc_move_list () noexcept;
    void dealloc_move_list (unique_ptr<move_list> move_list) noexcept;
    unique_ptr<move_list> copy_move_list (const move_list &move_list) noexcept;

    std::size_t move_list_size (move_list &ptr) noexcept;
    std::size_t move_list_capacity (move_list &ptr) noexcept;

    Move *move_list_begin (move_list &ptr) noexcept;
    Move *move_list_end (move_list &ptr) noexcept;

    void move_list_append (move_list &list, Move move) noexcept;
    void move_list_pop (move_list &list) noexcept;

    class MoveList
    {
    private:
        unique_ptr<move_list> my_moves_list = alloc_move_list ();

    public:
        MoveList () = default;

        ~MoveList () 
        {
            if (my_moves_list)
                dealloc_move_list (std::move (my_moves_list));
        }

        MoveList (Color color, std::initializer_list<czstring> list) noexcept;

        // Delete special copy members:
        MoveList (const MoveList &other) = delete;
        MoveList &operator= (MoveList other) = delete;

        // Defalut move members:
        MoveList (MoveList &&other) = default;
        MoveList &operator= (MoveList &&other) = default;

        void push_back (Move move) noexcept
        {
            move_list_append (*my_moves_list.get (), move);
        }

        void pop_back () noexcept
        {
            move_list_pop (*my_moves_list.get ());
        }

        [[nodiscard]] auto begin () const noexcept
        {
            return move_list_begin (*my_moves_list.get ());
        }

        [[nodiscard]] auto end () const noexcept
        {
            return move_list_end (*my_moves_list.get ());
        }

        [[nodiscard]] bool empty () const noexcept
        {
            return move_list_size (*my_moves_list.get ()) == 0;
        }

        [[nodiscard]] size_t size () const noexcept
        {
            return move_list_size (*my_moves_list.get ());
        }

        [[nodiscard]] size_t capacity () const noexcept
        {
            return move_list_capacity (*my_moves_list.get ());
        }

        [[nodiscard]] auto to_string () const -> string;

        bool operator== (const MoveList &other) const
        {
            return size () == other.size () && std::equal (
                    begin (),
                    end (),
                    other.begin (),
                    move_equals
            );
        }

        bool operator!= (const MoveList &other) const
        {
            return !(*this == other);
        }

        [[nodiscard]] auto data () const noexcept
        {
            return my_moves_list->move_array;
        }
    };

    auto to_string (const MoveList &list) -> string;

    std::ostream &operator<< (std::ostream &os, const MoveList &list);
}

#endif //WISDOM_MOVE_LIST_HPP
