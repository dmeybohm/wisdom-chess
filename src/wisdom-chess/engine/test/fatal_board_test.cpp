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
        "color-index-without-color",
        "Precondition failed at .*piece\\.hpp:[0-9]+: who == Color::White \\|\\| who == Color::Black"
    )
    {
        Color volatile color = Color {};
        [[maybe_unused]] auto index = colorIndex (color);
    }

    FATAL_CASE(
        "colored-piece-without-color",
        "Precondition failed at .*piece\\.hpp:[0-9]+: color == Color::White \\|\\| color == Color::Black"
    )
    {
        Color volatile color = Color {};
        [[maybe_unused]] auto piece = ColoredPiece::make (color, Piece::Pawn);
    }

    FATAL_CASE(
        "pawn-direction-without-color",
        "Precondition failed at .*coord\\.hpp:[0-9]+: color == Color::Black \\|\\| color == Color::White"
    )
    {
        Color volatile color = Color {};
        [[maybe_unused]] auto direction = pawnDirection (color);
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
        "Precondition failed at .*board_builder\\.hpp:[0-9]+: !existing \\|\\| pieceType"
    )
    {
        BoardBuilder builder;
        builder.addPiece ("a8", Color::Black, Piece::King);
        builder.addPiece ("a8", Color::White, Piece::Bishop);
    }
}
