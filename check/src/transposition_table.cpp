#include "transposition_table.hpp"
#include "board.hpp"

namespace wisdom
{
    std::optional<Transposition> TranspositionTable::lookup (BoardHashCode hash)
    {
        auto map_iterator = my_map.find (hash);
        if (map_iterator == my_map.end ())
            return {};

        auto pair = *map_iterator;
        auto list_iterator = pair.second;
        auto value = *list_iterator;

        // Found a match. move it to the front of the list, and update the map to
        // point to the new iterator.
        my_list.erase (list_iterator);
        auto new_iterator = my_list.insert (my_list.begin(), value);
        my_map[hash] = new_iterator;

        return value;
    }

    void TranspositionTable::add (Transposition transposition)
    {
        auto map_iterator = my_map.find (transposition.code);
        if (map_iterator != my_map.end ())
            drop_at_iterator (map_iterator);

        auto new_iterator = my_list.insert (my_list.begin(), transposition);
        my_map[transposition.code] = new_iterator;
        nr_elements++;

        if (nr_elements > Max_Transpositions)
            drop_last ();
    }

    void TranspositionTable::drop_last ()
    {
        auto last_iterator = my_list.end();
        last_iterator--;
        auto map_iterator = my_map.find( last_iterator->code);
        drop_at_iterator (map_iterator);
    }

    void TranspositionTable::drop_at_iterator (TranspositionMapIterator map_iterator)
    {
        auto pair = *map_iterator;
        auto list_iterator = pair.second;
        my_list.erase (list_iterator);
        my_map.erase (map_iterator);
        nr_elements--;
    }

    Transposition::Transposition (const Board &board, int _score) :
        code { board.code.hash_code() },
        score { _score }
    {
    }
}
