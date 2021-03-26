#include "transposition_table.hpp"
#include "board.hpp"

namespace wisdom
{
    std::optional<Transposition> TranspositionTable::lookup (BoardHashCode hash)
    {
        verify();

        auto map_iterator = my_map.find (hash);
        if (map_iterator == my_map.end ())
            return {};

        auto list_iterator = map_iterator->second;
        auto value = *list_iterator;

        drop_at_iterator (map_iterator);

        // Found a match. move it to the front of the list, and update the map to
        // point to the new iterator.
        auto new_iterator = my_list.insert (my_list.begin(), value);
        my_map[hash] = new_iterator;

        verify();

        return value;
    }

    void TranspositionTable::add (Transposition transposition)
    {
        verify();

        auto map_iterator = my_map.find (transposition.code);
        if (map_iterator != my_map.end ())
            drop_at_iterator (map_iterator);

        auto new_iterator = my_list.insert (my_list.begin(), transposition);
        my_map[transposition.code] = new_iterator;
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
        auto map_iterator = my_map.find (last_iterator->code);
        assert (last_iterator == map_iterator->second);
        drop_at_iterator (map_iterator);
    }

    void TranspositionTable::drop_at_iterator (TranspositionMapIterator map_iterator)
    {
        verify ();

        auto list_iterator = map_iterator->second;
        my_list.erase (list_iterator);
        my_map.erase (map_iterator);
        my_num_elements--;

        verify();
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
