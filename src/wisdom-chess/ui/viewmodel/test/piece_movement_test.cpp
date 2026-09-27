#include <doctest/doctest.h>

#include "wisdom-chess/ui/viewmodel/piece_movement.hpp"

using namespace wisdom;
using wisdom::ui::pieceMovement;

TEST_CASE( "pieceMovement" )
{
    SUBCASE( "A quiet move only moves the piece" )
    {
        auto movement = pieceMovement (moveParse ("e2 e4", Color::White));

        CHECK( movement.mover.src == coordParse ("e2") );
        CHECK( movement.mover.dst == coordParse ("e4") );
        CHECK( !movement.captured.has_value() );
        CHECK( !movement.castling_rook.has_value() );
        CHECK( movement.promoted_piece == Piece::None );
    }

    SUBCASE( "A capture removes the piece on the destination" )
    {
        auto movement = pieceMovement (moveParse ("e4xd5", Color::White));

        CHECK( movement.mover.dst == coordParse ("d5") );
        REQUIRE( movement.captured.has_value() );
        CHECK( *movement.captured == coordParse ("d5") );
    }

    SUBCASE( "En passant removes the pawn beside the destination" )
    {
        auto white = pieceMovement (moveParse ("e5 d6 ep", Color::White));
        REQUIRE( white.captured.has_value() );
        CHECK( *white.captured == coordParse ("d5") );

        auto black = pieceMovement (moveParse ("d4 e3 ep", Color::Black));
        REQUIRE( black.captured.has_value() );
        CHECK( *black.captured == coordParse ("e4") );
    }

    SUBCASE( "Castling moves the rook as well as the king" )
    {
        struct Case
        {
            czstring move;
            Color who;
            czstring king_dst;
            czstring rook_src;
            czstring rook_dst;
        };

        const Case cases[] = {
            { "o-o", Color::White, "g1", "h1", "f1" },
            { "o-o-o", Color::White, "c1", "a1", "d1" },
            { "o-o", Color::Black, "g8", "h8", "f8" },
            { "o-o-o", Color::Black, "c8", "a8", "d8" },
        };

        for (const auto& test : cases)
        {
            CAPTURE( test.move );
            CAPTURE( test.who );
            auto movement = pieceMovement (moveParse (test.move, test.who));

            CHECK( movement.mover.dst == coordParse (test.king_dst) );
            CHECK( !movement.captured.has_value() );
            REQUIRE( movement.castling_rook.has_value() );
            CHECK( movement.castling_rook->src == coordParse (test.rook_src) );
            CHECK( movement.castling_rook->dst == coordParse (test.rook_dst) );
        }
    }

    SUBCASE( "A promotion names the new piece" )
    {
        auto movement = pieceMovement (moveParse ("a7 a8(Q)", Color::White));

        CHECK( movement.promoted_piece == Piece::Queen );
        CHECK( !movement.captured.has_value() );
    }

    SUBCASE( "A promotion can capture" )
    {
        auto movement = pieceMovement (moveParse ("a7xb8(N)", Color::White));

        CHECK( movement.promoted_piece == Piece::Knight );
        REQUIRE( movement.captured.has_value() );
        CHECK( *movement.captured == coordParse ("b8") );
    }
}
