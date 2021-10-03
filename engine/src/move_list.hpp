#ifndef WISDOM_MOVE_LIST_HPP
#define WISDOM_MOVE_LIST_HPP

#include "global.hpp"
#include "move.hpp"

namespace wisdom
{
    using MoveVector = vector<Move>;

    class MoveList
    {
    private:
        unique_ptr<MoveVector> my_moves = allocate_move_vector ();

    public:
        MoveList () = default;

        ~MoveList () 
        {
            if (my_moves)
                deallocate_move_vector (std::move (my_moves));
        }

        MoveList (Color color, std::initializer_list<czstring> list) noexcept;
        MoveList (MoveList &&other) noexcept = default;

        MoveList (const MoveList &other)
        {
            my_moves->reserve (other.my_moves->size());
            std::copy (other.my_moves->begin (), other.my_moves->end (), std::back_inserter (*this->my_moves));
        }

        MoveList &operator= (MoveList other)
        {
            std::swap (this->my_moves, other.my_moves);
            return *this;
        }

        MoveList &operator= (MoveList &&other) = default;

        void push_back (Move move)
        {
            my_moves->push_back (move);
        }

        void pop_back ()
        {
            my_moves->pop_back ();
        }

        [[nodiscard]] auto begin () const noexcept
        {
            return my_moves->begin ();
        }

        [[nodiscard]] auto end () const noexcept
        {
            return my_moves->end ();
        }

        void append (const MoveList &other)
        {
            my_moves->insert (my_moves->end (), other.begin (), other.end ());
        }

        [[nodiscard]] bool empty () const noexcept
        {
            return my_moves->empty ();
        }

        [[nodiscard]] size_t size () const noexcept
        {
            return my_moves->size ();
        }

        [[nodiscard]] size_t capacity () const noexcept
        {
            return my_moves->capacity();
        }

        [[nodiscard]] auto to_string () const -> string;

        bool operator== (const MoveList &other) const
        {
            return my_moves->size () == other.my_moves->size () && std::equal (
                    my_moves->begin (),
                    my_moves->end (),
                    other.my_moves->begin (),
                    move_equals
            );
        }

        bool operator!= (const MoveList &other) const
        {
            return !(*this == other);
        }

        [[nodiscard]] auto data () const noexcept
        {
            return my_moves->data ();
        }

        static auto allocate_move_vector () -> unique_ptr<MoveVector>;
        static void deallocate_move_vector (std::unique_ptr<MoveVector> ptr);
    };

    auto to_string (const MoveList &list) -> string;

    std::ostream &operator<< (std::ostream &os, const MoveList &list);
}

#endif //WISDOM_MOVE_LIST_HPP
