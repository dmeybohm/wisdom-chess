#ifndef WISDOM_TRANSPOSITION_TABLE_HPP
#define WISDOM_TRANSPOSITION_TABLE_HPP

#include "global.hpp"
#include "board_code.hpp"
#include "variation_glimpse.hpp"

namespace wisdom
{
    constexpr std::size_t Max_Transpositions = 5000;

    struct BaseTransposition
    {
        // The hash code identifying this position.
        BoardHashCode hash_code;

        // The identifier for the board.
        BoardCode board_code;

        // The score for this position.
        int score;

        // How deeply this position was analyzed.
        int relative_depth;

        // Glimpse into what the engine was thinking for this position.
        VariationGlimpse variation_glimpse;

        BaseTransposition (const BaseTransposition &other) = default;

        BaseTransposition (BoardHashCode _hash_code, const BoardCode &_board_code, int _score, int _relative_depth,
                           const VariationGlimpse &_variation_glimpse ) :
                hash_code { _hash_code }, board_code { _board_code }, score { _score },
                relative_depth { _relative_depth },
                variation_glimpse { _variation_glimpse }
        {}

        bool operator== (const BaseTransposition &other) const
        {
            return other.hash_code == this->hash_code;
        }
    };

    struct RelativeTransposition : BaseTransposition
    {
        RelativeTransposition (const Board &board, int _score, int _relative_depth,
                               const VariationGlimpse &_variation_glimpse);

        RelativeTransposition (const BoardCode &_code, int _score, int _relative_depth,
                               const VariationGlimpse &_variation_glimpse) :
            BaseTransposition (_code.hash_code(), _code, _score, _relative_depth, _variation_glimpse)
        {}

        RelativeTransposition (BoardHashCode _hash_code, const BoardCode &_board_code, int _score, int _relative_depth,
                               const VariationGlimpse &_variation_glimpse) :
            BaseTransposition (_hash_code, _board_code, _score, _relative_depth, _variation_glimpse)
        {}

        static RelativeTransposition from_defaults ()
        {
            return RelativeTransposition { 0, BoardCode{}, Negative_Infinity, 0, {} };
        }
    };

    struct ColoredTransposition : BaseTransposition
    {
        explicit ColoredTransposition (const BaseTransposition &transposition, Color who) :
            BaseTransposition (transposition)
        {
            this->score *= who == Color::Black ? -1 : 1;
        }

        RelativeTransposition to_relative_transposition (Color who)
        {
            RelativeTransposition result {
                this->hash_code, this->board_code, this->score, this->relative_depth, this->variation_glimpse
            };
            result.score *= who == Color::Black ? -1 : 1;
            return result;
        }
    };

    using TranspositionList = std::list<ColoredTransposition>;
    using TranspositionListIterator = TranspositionList::iterator;
    using TranspositionMap = std::unordered_map<BoardHashCode, TranspositionListIterator>;


    class TranspositionTable final
    {
    private:
        TranspositionList my_list{};
        TranspositionMap my_map{};
        std::size_t my_num_elements = 0;

        void drop_last ();
        void verify () const;

    public:
        // Lookup the transposition.
        [[nodiscard]] std::optional<RelativeTransposition> lookup (BoardHashCode hash, Color who);

        // Add the transposition.
        void add (RelativeTransposition transposition, Color who);

        [[nodiscard]] std::size_t size() const
        {
            return my_num_elements;
        }
    };
}

namespace std
{
    template<>
    struct hash<wisdom::ColoredTransposition>
    {
        std::size_t operator() (const wisdom::ColoredTransposition &transposition) const
        {
            return transposition.hash_code;
        }
    };
}

#endif //WISDOM_TRANSPOSITION_TABLE_HPP
