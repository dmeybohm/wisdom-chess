#ifndef WISDOM_TRANSPOSITION_TABLE_HPP
#define WISDOM_TRANSPOSITION_TABLE_HPP

#include <list>
#include <unordered_map>
#include <optional>
#include <functional>

#include "board_code.hpp"

namespace wisdom
{
    struct Transposition
    {
        BoardHashCode hash_code;
        int score;

        Transposition () = default;
        Transposition (const Transposition &other) = default;

        Transposition (const Board &board, int _score);
        Transposition (BoardHashCode _code, int _score) :
                hash_code { _code }, score { _score } {}

        bool operator==(const Transposition &other) const
        {
            return other.hash_code == this->hash_code;
        }

        Transposition with_color (Color who)
        {
            Transposition result { *this };
            result.score *= who == Color::Black ? -1 : 1;
            return result;
        }

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
        void verify () const;

    public:
        // Lookup the transposition.
        [[nodiscard]] std::optional<Transposition> lookup (BoardHashCode hash, Color who);

        // Add the transposition.
        void add (Transposition transposition, Color who);

        [[nodiscard]] std::size_t size() const
        {
            return my_num_elements;
        }
    };



}

namespace std
{
    template<>
    struct hash<wisdom::Transposition>
    {
        std::size_t operator()(const wisdom::Transposition &transposition) const
        {
            return transposition.hash_code;
        }
    };
}



#endif //WISDOM_TRANSPOSITION_TABLE_HPP
