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

    FATAL_CASE(
        "board-builder-invalid-coordinate",
        "Precondition failed at .*coord\\.hpp:[0-9]+: result\\.has_value\\(\\)"
    )
    {
        BoardBuilder builder;
        builder.addPiece ("a9", Color::White, Piece::Pawn);
    }

    FATAL_CASE(
        "board-builder-half-move-clock-out-of-range",
        "Precondition failed at .*board_builder\\.hpp:[0-9]+: new_half_moves_clock >= 0"
    )
    {
        BoardBuilder builder;
        int volatile clock = -1;
        builder.setHalfMovesClock (clock);
    }

    FATAL_CASE(
        "board-builder-full-moves-out-of-range",
        "Precondition failed at .*board_builder\\.hpp:[0-9]+: new_full_moves >= 1"
    )
    {
        BoardBuilder builder;
        int volatile full_moves = 0;
        builder.setFullMoves (full_moves);
    }

    FATAL_CASE(
        "board-builder-replace-king",
        "Precondition failed at .*board_builder\\.hpp:[0-9]+: pieceType \\(my_squares"
    )
    {
        BoardBuilder builder;
        builder.addPiece ("a8", Color::Black, Piece::King);
        builder.addPiece ("a8", Color::White, Piece::Bishop);
    }
}
