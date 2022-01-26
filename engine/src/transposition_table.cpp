#include "transposition_table.hpp"
#include "board.hpp"

#include <ostream>

namespace wisdom
{

    RelativeTransposition::RelativeTransposition (const Board& board, int _score, int _relative_depth,
                                                  const VariationGlimpse& _variation_glimpse) :
            RelativeTransposition (board.get_code ().hash_code (),
                                   board.get_code (), _score, _relative_depth, _variation_glimpse)
    {}

    auto operator<< (std::ostream& os, const BaseTransposition& transposition) -> std::ostream&
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
