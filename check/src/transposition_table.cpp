#include "transposition_table.hpp"
#include "board.hpp"

namespace wisdom
{
    std::optional<Transposition> TranspositionTable::lookup (BoardHashCode hash, Color who)
    {
        verify();

        auto map_iterator = my_map.find (hash);
        if (map_iterator == my_map.end ())
            return {};

        auto list_iterator = map_iterator->second;
        auto value = *list_iterator;

        my_list.splice (my_list.begin(), my_list, list_iterator);
        verify();

        return value.with_color (who);
    }

    void TranspositionTable::add (Transposition transposition, Color who)
    {
        verify();

        auto map_iterator = my_map.find (transposition.code);
        if (map_iterator != my_map.end ())
        {
            my_list.erase (map_iterator->second);
            my_map.erase (map_iterator);
            my_num_elements--;
            verify ();
        }

        transposition = transposition.with_color (who);
        my_list.push_front (transposition);
        my_map.insert (make_pair (transposition.code, my_list.begin ()));
        my_num_elements++;

        if (my_num_elements > Max_Transpositions)
            drop_last ();

        verify();

        assert (my_map.size() == my_list.size());
    }

    void TranspositionTable::drop_last ()
    {
        auto last_iterator = my_list.end();
        last_iterator--;
        my_map.erase (last_iterator->code);
        my_list.pop_back ();
        my_num_elements--;
    }

    void TranspositionTable::verify () const
    {
        if (static_cast<size_t>(my_map.size ()) != static_cast<size_t>(my_list.size ()))
            assert (my_map.size () == my_list.size ());
        if (my_map.size () != my_num_elements)
            assert (my_map.size () == my_num_elements);
        if (my_list.size () != my_num_elements)
            assert (my_list.size () == my_num_elements);
    }

    Transposition::Transposition (const Board &board, int _score) :
        code { board.code.hash_code() },
        score { _score }
    {
    }
}
