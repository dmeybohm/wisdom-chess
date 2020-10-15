extern "C"
{
#include "attack_vector.h"
}

#include "catch.hpp"

TEST_CASE("Attack vector initialization works for king", "[attack-vector]")
{
    struct attack_vector attacks = { 0xff };

    attack_vector_init (&attacks);

    attack_vector_add (&attacks, board, COLOR_WHITE, coord_create (3, 3), PIECE_KING);

    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (2, 2)) == 1 );
    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (2, 3)) == 1 );
    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (2, 4)) == 1 );
    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (3, 2)) == 1 );
    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (3, 3)) == 0 );
    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (3, 4)) == 1 );
    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (4, 2)) == 1 );
    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (4, 3)) == 1 );
    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (4, 4)) == 1 );
}

TEST_CASE("Attack vector initialization works for pawn", "[attack-vector]")
{
    struct attack_vector attacks = { 0xff };

    attack_vector_init (&attacks);

    attack_vector_add (&attacks, board, COLOR_WHITE, coord_create (3, 3), PIECE_PAWN);
    attack_vector_add (&attacks, board, COLOR_BLACK, coord_create (1, 1), PIECE_PAWN);

    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (3, 3)) == 0 );
    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (2, 4)) == 1 );
    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (2, 2)) == 1 );
    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (2, 3)) == 0 );
    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (1, 1)) == 0 );
    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (2, 2)) == 1 );
    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (2, 1)) == 0 );
}

TEST_CASE("Attack vector initialization works for knight", "[attack-vector]")
{
    struct attack_vector attacks = { 0xff };

    attack_vector_init (&attacks);

    attack_vector_add (&attacks, board, COLOR_WHITE, coord_create (3, 3), PIECE_KNIGHT);

    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (2, 1)) == 1 );
    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (1, 2)) == 1 );
    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (1, 4)) == 1 );
    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (2, 5)) == 1 );
    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (4, 5)) == 1 );
    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (5, 4)) == 1 );
    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (5, 2)) == 1 );
    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (4, 1)) == 1 );

    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (3, 3)) == 0 );
    CHECK( attack_vector_count (&attacks, COLOR_WHITE, coord_create (4, 4)) == 0 );
}
