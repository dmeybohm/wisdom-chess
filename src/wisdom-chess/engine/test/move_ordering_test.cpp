#include "wisdom-chess/engine/move_ordering.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "KillerTable" )
{
    KillerTable table;
    Move first = moveParse ("g1 f3", Color::White);
    Move second = moveParse ("e2 e4", Color::White);
    Move third = moveParse ("d2 d4", Color::White);

    SUBCASE( "A new table has no killers" )
    {
        CHECK( table.getKillers (0) == KillerMoves { nullopt, nullopt } );
        CHECK( table.getKillers (Max_Search_Depth - 1) == KillerMoves { nullopt, nullopt } );
    }

    SUBCASE( "A store takes slot 0 and demotes the move that was there" )
    {
        table.store (3, first);
        CHECK( table.getKillers (3) == KillerMoves { first, nullopt } );

        table.store (3, second);
        CHECK( table.getKillers (3) == KillerMoves { second, first } );

        table.store (3, third);
        CHECK( table.getKillers (3) == KillerMoves { third, second } );
    }

    SUBCASE( "Each ply has its own slots" )
    {
        table.store (3, first);
        table.store (4, second);

        CHECK( table.getKillers (3) == KillerMoves { first, nullopt } );
        CHECK( table.getKillers (4) == KillerMoves { second, nullopt } );
        CHECK( table.getKillers (5) == KillerMoves { nullopt, nullopt } );
    }

    SUBCASE( "Storing a move already in a slot leaves the table alone" )
    {
        table.store (3, first);
        table.store (3, second);

        table.store (3, second);
        CHECK( table.getKillers (3) == KillerMoves { second, first } );

        table.store (3, first);
        CHECK( table.getKillers (3) == KillerMoves { second, first } );
    }

    SUBCASE( "A capture is not stored" )
    {
        table.store (3, first);
        table.store (3, moveParse ("e4xd5", Color::White));
        table.store (3, moveParse ("e5 d6 ep", Color::White));

        CHECK( table.getKillers (3) == KillerMoves { first, nullopt } );
    }

    SUBCASE( "A promotion is not stored" )
    {
        table.store (3, first);
        table.store (3, moveParse ("b7 b8 (Q)", Color::White));
        table.store (3, moveParse ("b7xa8 (N)", Color::White));

        CHECK( table.getKillers (3) == KillerMoves { first, nullopt } );
    }

    SUBCASE( "Castling is stored" )
    {
        Move castle = moveParse ("o-o", Color::White);
        table.store (3, castle);

        CHECK( table.getKillers (3) == KillerMoves { castle, nullopt } );
    }
}
