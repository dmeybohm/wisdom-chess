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
        // The hash code identifying this position.
        BoardHashCode hash_code;

        // The identifier for the board.
        BoardCode board_code;

        // The score for this position.
        int score;

        // How deeply this position was analyzed.
        int relative_depth;

        Transposition () = default;
        Transposition (const Transposition &other) = default;

        static Transposition from_defaults ()
        {
            return Transposition { 0, BoardCode{}, Negative_Infinity, 0 };
        }

        Transposition (const Board &board, int _score, int _relative_depth);

        Transposition (const BoardCode &_code, int _score, int _relative_depth) :
                hash_code { _code.hash_code() }, board_code { _code }, score { _score },
                relative_depth { _relative_depth }
        {}

        Transposition (BoardHashCode _hash_code, const BoardCode &_board_code, int _score, int _relative_depth) :
                hash_code { _hash_code }, board_code { _board_code }, score { _score },
                relative_depth { _relative_depth }
        {}

        bool operator== (const Transposition &other) const
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
        std::size_t operator() (const wisdom::Transposition &transposition) const
        {
            return transposition.hash_code;
        }
    };
}



#endif //WISDOM_TRANSPOSITION_TABLE_HPP
