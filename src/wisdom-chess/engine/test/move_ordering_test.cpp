#include "wisdom-chess/engine/move_ordering.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "KillerTable" )
{
    KillerTable table;
    Move first = toMove ("g1 f3", Color::White);
    Move second = toMove ("e2 e4", Color::White);
    Move third = toMove ("d2 d4", Color::White);

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
        table.store (3, toMove ("e4xd5", Color::White));
        table.store (3, toMove ("e5 d6 ep", Color::White));

        CHECK( table.getKillers (3) == KillerMoves { first, nullopt } );
    }

    SUBCASE( "A promotion is not stored" )
    {
        table.store (3, first);
        table.store (3, toMove ("b7 b8 (Q)", Color::White));
        table.store (3, toMove ("b7xa8 (N)", Color::White));

        CHECK( table.getKillers (3) == KillerMoves { first, nullopt } );
    }

    SUBCASE( "Castling is stored" )
    {
        Move castle = toMove ("o-o", Color::White);
        table.store (3, castle);

        CHECK( table.getKillers (3) == KillerMoves { castle, nullopt } );
    }
}

TEST_CASE( "CutoffHistory" )
{
    CutoffHistory history;
    Move knight = toMove ("g1 f3", Color::White);
    Move pawn = toMove ("e2 e4", Color::White);

    SUBCASE( "A new table scores every move zero" )
    {
        CHECK( history.getScore (Color::White, knight) == 0 );
        CHECK( history.getScore (Color::Black, knight) == 0 );
    }

    SUBCASE( "A cutoff adds the square of the depth left" )
    {
        history.store (Color::White, knight, 3);
        CHECK( history.getScore (Color::White, knight) == 9 );

        history.store (Color::White, knight, 2);
        CHECK( history.getScore (Color::White, knight) == 13 );

        CHECK( history.getScore (Color::White, pawn) == 0 );
    }

    SUBCASE( "Each side has its own counters" )
    {
        history.store (Color::White, knight, 3);
        CHECK( history.getScore (Color::Black, knight) == 0 );
    }

    SUBCASE( "A capture or a promotion is not counted" )
    {
        Move capture = toMove ("e4xd5", Color::White);
        Move promotion = toMove ("b7 b8 (Q)", Color::White);

        history.store (Color::White, capture, 3);
        history.store (Color::White, promotion, 3);

        CHECK( history.getScore (Color::White, capture) == 0 );
        CHECK( history.getScore (Color::White, promotion) == 0 );
    }

    SUBCASE( "A counter stops at its maximum" )
    {
        int cutoffs_to_reach_max = CutoffHistory::Max_Score / (Max_Search_Depth * Max_Search_Depth);
        for (int i = 0; i <= cutoffs_to_reach_max; i++)
            history.store (Color::White, knight, Max_Search_Depth);

        CHECK( history.getScore (Color::White, knight) == CutoffHistory::Max_Score );
    }
}
