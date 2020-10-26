extern "C"
{
#include "board.h"
}

#include "catch.hpp"
#include "board_builder.hpp"

TEST_CASE("Initial position for Northwest to Southeast attack", "[attack-vector]")
{
    CHECK( first_nw_to_se_coord(coord_alg("a8")) == coord_alg("a8") );
    CHECK( first_nw_to_se_coord(coord_alg("h1")) == coord_alg("a8") );
    CHECK( first_nw_to_se_coord(coord_alg("a1")) == coord_alg("a1") );
    CHECK( first_nw_to_se_coord(coord_alg("b1")) == coord_alg("a2") );
    CHECK( first_nw_to_se_coord(coord_alg("b2")) == coord_alg("a3") );
    CHECK( first_nw_to_se_coord(coord_alg("e4")) == coord_alg("a8") );
    CHECK( first_nw_to_se_coord(coord_alg("h8")) == coord_alg("h8") );
    CHECK( first_nw_to_se_coord(coord_alg("h7")) == coord_alg("g8") );
}

TEST_CASE("Initial position for Northeast to Southwest attack", "[attack-vector]")
{
    CHECK( first_ne_to_sw_coord(coord_alg("a8")) == coord_alg("a8") );
    CHECK( first_ne_to_sw_coord(coord_alg("h1")) == coord_alg("h1") );
    CHECK( first_ne_to_sw_coord(coord_alg("a7")) == coord_alg("b8") );
    CHECK( first_ne_to_sw_coord(coord_alg("b7")) == coord_alg("c8") );
    CHECK( first_ne_to_sw_coord(coord_alg("a1")) == coord_alg("h8") );
    CHECK( first_ne_to_sw_coord(coord_alg("b1")) == coord_alg("h7") );
    CHECK( first_ne_to_sw_coord(coord_alg("b2")) == coord_alg("h8") );
    CHECK( first_ne_to_sw_coord(coord_alg("e4")) == coord_alg("h7") );
    CHECK( first_ne_to_sw_coord(coord_alg("h8")) == coord_alg("h8") );
    CHECK( first_ne_to_sw_coord(coord_alg("h7")) == coord_alg("h7") );
}

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

TEST_CASE("Northeast to southwest bishop and queen attack vectors calculate correctly", "[attack-vector]")
{
    board_builder builder;

    builder.add_piece ("h8", COLOR_WHITE, PIECE_QUEEN);
    builder.add_piece ("a1", COLOR_WHITE, PIECE_BISHOP);

    struct board *board = builder.build();

    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("h8")) == 1 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("g7")) == 2 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("f6")) == 2 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("e5")) == 2 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("d4")) == 2 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("c3")) == 2 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("b2")) == 2 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("a1")) == 1 );

    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("a2")) == 0 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("b3")) == 0 );
}

TEST_CASE("Horizontal rook and queen attack vectors calculate correctly", "[attack-vector]")
{
    board_builder builder;

    builder.add_piece ("a5", COLOR_WHITE, PIECE_QUEEN);
    builder.add_piece ("h5", COLOR_WHITE, PIECE_ROOK);

    struct board *board = builder.build();

    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("a5")) == 1 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("b5")) == 2 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("c5")) == 2 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("d5")) == 2 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("e5")) == 2 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("f5")) == 2 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("g5")) == 2 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("h5")) == 1 );

    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("a2")) == 0 );
    CHECK( attack_vector_count (&board->attacks, COLOR_WHITE, coord_alg("b3")) == 0 );
}
