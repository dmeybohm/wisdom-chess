#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"
#include "wisdom-chess/engine/generate.hpp"
#include "wisdom-chess/engine/move.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "toMove" )
{
    SUBCASE( "toMove parses captures and non-captures" )
    {
        Move capture = toMove ("a6xb7");
        Move non_capture = toMove ("e4 e8");

        CHECK( ( capture == toMove ("a6xb7", Color::White) ) );
        CHECK( ( non_capture == toMove ("e4 e8", Color::White) ) );
    }

    SUBCASE( "color matters in toMove" )
    {
        Move castle = toMove ("o-o", Color::Black);
        CHECK( ( coordRow (castle.getSrc()) == 0 ) );
        CHECK( ( coordRow (castle.getDst()) == 0 ) );
        CHECK( ( castle == toMove ("o-o", Color::Black) ) );
    }

    SUBCASE( "Castling needs a color" )
    {
        CHECK( !parseMove ("o-o", nullopt).has_value() );
        CHECK( !parseMove ("o-o-o", nullopt).has_value() );
    }

    SUBCASE( "Invalid moves are rejected" )
    {
        CHECK( !parseMove ("invalid", Color::White).has_value() );
        CHECK( !parseMove ("a8a8", Color::White).has_value() );
        CHECK( !parseMove ("e2e2", Color::White).has_value() );
    }

    SUBCASE( "Empty and whitespace-only input" )
    {
        CHECK( !parseMove ("", Color::White).has_value() );
        CHECK( !parseMove ("  \t ", Color::White).has_value() );
    }

    SUBCASE( "Bytes above 0x7f are rejected rather than classified" )
    {
        CHECK( !parseMove ("\xe9", Color::White).has_value() );
        CHECK( !parseMove ("\xc3\xa9", Color::White).has_value() );
        CHECK( !parseMove ("\xa0" "e2e4", Color::White).has_value() );
        CHECK( !parseMove ("\xd0-\xd0", Color::White).has_value() );
        CHECK( !parseMove ("\xff", Color::White).has_value() );
    }
}

namespace
{
    // A packed move by its fields: source, destination and kind, from the
    // low bits up.
    [[nodiscard]] constexpr auto
    packFields (int src, int dst, int kind)
        -> int
    {
        return src | (dst << 6) | (kind << 12);
    }
}

TEST_CASE( "parsePackedMove()" )
{
    SUBCASE( "Every move toPacked() gives comes back" )
    {
        // Between them these reach castling, en passant, captures and
        // promotions with and without a capture.
        for (czstring fen : {
                 "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                 "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
                 "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1",
                 "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
             })
        {
            CAPTURE( fen );
            FenParser parser { fen };
            auto board = parser.buildBoard();

            for (auto move : generateLegalMoves (board, board.getCurrentTurn()))
            {
                CAPTURE( asString (move) );
                CHECK( parsePackedMove (move.toPacked()) == move );
            }
        }

        for (auto move : {
                 toMove ("o-o", Color::White),
                 toMove ("o-o-o", Color::Black),
                 toMove ("e5 d6 ep"),
                 toMove ("b7 b8 (Q)"),
                 toMove ("b7xc8 (N)"),
                 toMove ("g2xh1 (B)"),
                 toMove ("a2 a1 (R)"),
             })
        {
            CAPTURE( asString (move) );
            CHECK( parsePackedMove (move.toPacked()) == move );
        }
    }

    SUBCASE( "A number outside sixteen bits is empty" )
    {
        CHECK( parsePackedMove (-1) == nullopt );
        CHECK( parsePackedMove (0x1'0000) == nullopt );
        CHECK( parsePackedMove (toMove ("e2 e4").toPacked() | 0x1'0000) == nullopt );
    }

    SUBCASE( "A move to its own square is empty" )
    {
        CHECK( parsePackedMove (0) == nullopt );
        CHECK( parsePackedMove (packFields (12, 12, Combined_Default)) == nullopt );
        CHECK( parsePackedMove (packFields (12, 12, Combined_Normal_Capture)) == nullopt );
    }

    SUBCASE( "A kind no move has is empty" )
    {
        CHECK( parsePackedMove (packFields (8, 0, Combined_Promote_Capture_Base + 3)).has_value() );

        for (int kind = Combined_Promote_Capture_Base + 4; kind < 16; kind++)
        {
            CAPTURE( kind );
            CHECK( parsePackedMove (packFields (8, 0, kind)) == nullopt );
        }
    }
}
