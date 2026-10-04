#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/board_code.hpp"
#include "wisdom-chess/engine/castling.hpp"
#include "wisdom-chess/engine/generate.hpp"

#include "fatal_test.hpp"

using namespace wisdom;

namespace
{
    FATAL_CASE( "bad-castling-flags", "Precondition failed at .*castling\\.hpp" )
    {
        volatile uint8_t flags = 0x4;
        [[maybe_unused]] CastlingEligibility eligibility { flags };
    }

    FATAL_CASE( "bad-en-passant-row", "Precondition failed at .*board_code\\.hpp" )
    {
        Board board { BoardBuilder::fromDefaultPosition() };
        BoardCode code { board };
        code.setEnPassantTarget (
            Color::White,
            makeCoord (Black_En_Passant_Row, 0),
            EnPassantTargetState::Illegal
        );
    }

    FATAL_CASE(
        "need-pawn-promotion-bad-color",
        "Precondition failed at .*generate\\.cpp:[0-9]+: isColorValid \\(who\\)"
    )
    {
        Color volatile color = Color::None;
        [[maybe_unused]] bool promote = needPawnPromotion (0, color);
    }
}
