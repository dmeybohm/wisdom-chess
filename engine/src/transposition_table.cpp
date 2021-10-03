#include "transposition_table.hpp"
#include "board.hpp"

#include <ostream>

namespace wisdom
{
    auto TranspositionTable::lookup (BoardHashCode hash, Color who)
        -> optional<RelativeTransposition>
    {
        verify();

        auto map_iterator = my_map.find (hash);
        if (map_iterator == my_map.end ())
            return nullopt;

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

    [[nodiscard]] auto TranspositionTable::lookup_board_for_depth (Board &board, Color who,
                                                                   int depth)
        -> optional<RelativeTransposition>
    {
        auto optional_transposition = lookup (board.code.hash_code (), who);

        if (optional_transposition.has_value ())
        {
            // Check the board codes are equal:
            if (optional_transposition->board_code != board.get_code ())
            {
                my_dupe_hashes++;
                return nullopt;
            }

            if (optional_transposition->relative_depth >= depth)
            {
                my_hits++;
                return optional_transposition;
            }
        }

        my_misses++;
        return nullopt;
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

        auto size1 = gsl::narrow<size_t>(my_map.size ());
        auto size2 = gsl::narrow<size_t>(my_list.size ());
        if (size1 != size2)
            assert (my_map.size () == my_list.size ());
        if (my_map.size () != my_num_elements)
            assert (my_map.size () == my_num_elements);
        if (my_list.size () != my_num_elements)
            assert (my_list.size () == my_num_elements);
#endif
    }

    void TranspositionTable::add (Board &board, int score, Color who, int relative_depth,
                                  const VariationGlimpse &variation_glimpse)
    {
        RelativeTransposition evaluation { board, score, relative_depth, variation_glimpse };
        add (evaluation, who);
    }

    RelativeTransposition::RelativeTransposition (const Board &board, int _score, int _relative_depth,
                                                  const VariationGlimpse &_variation_glimpse) :
            RelativeTransposition (board.get_code ().hash_code (),
                                   board.code, _score, _relative_depth, _variation_glimpse)
    {}

    auto operator<< (std::ostream &os, const BaseTransposition &transposition) -> std::ostream&
    {
        os << "{ hash_code: " << transposition.hash_code <<
            " board_code: " << transposition.board_code <<
            " score: " << transposition.score <<
            " relative_depth: " << transposition.relative_depth <<
            " variation_glimpse: " << transposition.variation_glimpse <<
            " }";
        return os;
    }
}
