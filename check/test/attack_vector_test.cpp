extern "C"
{
#include "board.h"
#include "attack_vector.h"
}

#include "catch.hpp"
#include "board_builder.hpp"

TEST_CASE("Attack vector initialization works for king", "[attack-vector]")
{
    board_builder builder;

    builder.add_piece (3, 3, COLOR_WHITE, PIECE_KING);

    struct board *board = builder.build();

    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (2, 2)) == 1 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (2, 3)) == 1 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (2, 4)) == 1 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (3, 2)) == 1 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (3, 3)) == 0 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (3, 4)) == 1 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (4, 2)) == 1 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (4, 3)) == 1 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (4, 4)) == 1 );
}

TEST_CASE("Attack vector initialization works for pawn", "[attack-vector]")
{
    board_builder builder;

    builder.add_piece (3, 3, COLOR_WHITE, PIECE_PAWN);
    builder.add_piece (1, 1, COLOR_BLACK, PIECE_PAWN);

    struct board *board = builder.build();

    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (3, 3)) == 0 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (2, 4)) == 1 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (2, 2)) == 1 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (2, 3)) == 0 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (1, 1)) == 0 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (2, 2)) == 1 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (2, 1)) == 0 );
}

TEST_CASE("Attack vector initialization works for knight", "[attack-vector]")
{
    board_builder builder;

    builder.add_piece (3, 3, COLOR_WHITE, PIECE_KNIGHT);

    struct board *board = builder.build();

    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (2, 1)) == 1 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (1, 2)) == 1 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (1, 4)) == 1 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (2, 5)) == 1 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (4, 5)) == 1 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (5, 4)) == 1 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (5, 2)) == 1 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (4, 1)) == 1 );

    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (3, 3)) == 0 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_create (4, 4)) == 0 );
}

TEST_CASE("Northwest to southeast bishop and queen attack vectors calculate correctly", "[attack-vector]")
{
    board_builder builder;

    builder.add_piece ("a8", COLOR_WHITE, PIECE_QUEEN);
    builder.add_piece ("h1", COLOR_WHITE, PIECE_BISHOP);

    struct board *board = builder.build();

    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("a8")) == 1 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("b7")) == 2 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("c6")) == 2 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("d5")) == 2 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("e4")) == 2 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("f3")) == 2 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("g2")) == 2 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("h1")) == 1 );

    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("h2")) == 0 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("g3")) == 0 );
}
