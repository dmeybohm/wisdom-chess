#include "transposition_table.hpp"
#include "board.hpp"

#include <ostream>

namespace wisdom
{
    std::optional<RelativeTransposition> TranspositionTable::lookup (BoardHashCode hash, Color who)
    {
        verify();

        auto map_iterator = my_map.find (hash);
        if (map_iterator == my_map.end ())
            return std::nullopt;

        auto list_iterator = map_iterator->second;
        ColoredTransposition value = *list_iterator;

        my_list.splice (my_list.begin(), my_list, list_iterator);
        verify();

        // Re-invert back to original:
        return value.to_relative_transposition (who);
    }

    void TranspositionTable::add (RelativeTransposition transposition, Color who)
    {
        verify();

        auto map_iterator = my_map.find (transposition.hash_code);
        if (map_iterator != my_map.end ())
        {
            my_list.erase (map_iterator->second);
            my_map.erase (map_iterator);
            my_num_elements--;
            verify ();
        }

        ColoredTransposition colored_transposition = ColoredTransposition { transposition, who };
        my_list.push_front (colored_transposition);
        my_map.insert (make_pair (colored_transposition.hash_code, my_list.begin ()));
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
        my_map.erase (last_iterator->hash_code);
        my_list.pop_back ();
        my_num_elements--;
    }

    void TranspositionTable::verify () const
    {
#ifdef VERIFY_TRANSPOSITION_TABLE
        {
            std::unordered_set<Transposition> list_uniques;
            int cnt = 0;
            std::map<int, Transposition> dupes;
            for (Transposition t : my_list)
            {
                if (list_uniques.contains (t))
                {
                    dupes[cnt] = t;
                }
                list_uniques.insert (t);
                cnt++;
            }
            if (dupes.size() > 0)
            {
                assert(0);
            }
        }

        if (static_cast<size_t>(my_map.size ()) != static_cast<size_t>(my_list.size ()))
            assert (my_map.size () == my_list.size ());
        if (my_map.size () != my_num_elements)
            assert (my_map.size () == my_num_elements);
        if (my_list.size () != my_num_elements)
            assert (my_list.size () == my_num_elements);
#endif
    }

    RelativeTransposition::RelativeTransposition (const Board &board, int _score, int _relative_depth,
                                                  const VariationGlimpse &_variation_glimpse) :
            RelativeTransposition (board.code.hash_code(), board.code, _score, _relative_depth, _variation_glimpse)
    {}

    std::ostream &operator<< (std::ostream &os, const BaseTransposition &transposition)
    {
        os << "{ hash_code: " << transposition.hash_code << " board_code: " << transposition.board_code << " score: "
            << transposition.score << " relative_depth: " << transposition.relative_depth << " variation_glimpse: "
            << transposition.variation_glimpse << " }";
        return os;
    }
}
