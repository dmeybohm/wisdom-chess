#ifndef WISDOM_TRANSPOSITION_TABLE_HPP
#define WISDOM_TRANSPOSITION_TABLE_HPP

#include <list>
#include <unordered_map>
#include <optional>

#include "board_code.hpp"

namespace wisdom
{
    struct Transposition
    {
        BoardHashCode code;
        int score;

        Transposition () = default;
        Transposition (const Board &board, int _score);
        Transposition (BoardHashCode _code, int _score) :
            code { _code }, score { _score } {}
    };

    using TranspositionList = std::list<Transposition>;
    using TranspositionListIterator = TranspositionList::iterator;
    using TranspositionMap = std::unordered_map<BoardHashCode, TranspositionListIterator>;
    using TranspositionMapIterator = TranspositionMap::iterator;

    constexpr std::size_t Max_Transpositions = 100 * 1000;

    class TranspositionTable final
    {
    private:
        TranspositionList my_list{};
        TranspositionMap my_map{};
        std::size_t my_num_elements{};

        void drop_last ();
        void drop_at_iterator (TranspositionMapIterator map_iterator);

    public:
        // Lookup the transposition.
        [[nodiscard]] std::optional<Transposition> lookup (BoardHashCode hash);

        // Add the transposition.
        void add (Transposition transposition);

        [[nodiscard]] std::size_t size() const
        {
            return my_num_elements;
        }

        void verify () const;
    };
}

#endif //WISDOM_TRANSPOSITION_TABLE_HPP
