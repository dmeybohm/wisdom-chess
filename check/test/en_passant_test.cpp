#include "catch.hpp"

extern "C"
{
#include "../src/board.h"
#include "../src/board_positions.h"
#include "../src/move_tree.h"
#include "../src/generate.h"
}

TEST_CASE( "En passant moves work", "[en-passant]" )
{
    struct board *board = board_new();
    enum piece_type back_rank[] =
    {
        PIECE_ROOK,   PIECE_KNIGHT, PIECE_BISHOP, PIECE_QUEEN, PIECE_KING,
        PIECE_BISHOP, PIECE_KNIGHT, PIECE_ROOK, PIECE_LAST
    };
    enum piece_type black_pawns[] =
    {
        PIECE_PAWN, PIECE_PAWN, PIECE_PAWN, PIECE_PAWN, PIECE_PAWN,
        PIECE_PAWN, PIECE_PAWN, PIECE_PAWN, PIECE_LAST
    };
    enum piece_type fifth_rank[] =
    {
        PIECE_NONE, PIECE_NONE, PIECE_NONE, PIECE_NONE, PIECE_PAWN,
        PIECE_NONE, PIECE_NONE, PIECE_NONE, PIECE_LAST
    };

    struct board_positions positions[] =
    {
        { 0, COLOR_BLACK, back_rank },
        { 1, COLOR_BLACK, black_pawns },
        { 3, COLOR_WHITE, fifth_rank },
        { 7, COLOR_WHITE, back_rank },
        { 0, COLOR_NONE, NULL }
    };

    board_init_from_positions (board, positions);

    move_t pawn_move = move_create (1, 5, 3, 5);
    do_move (board, COLOR_BLACK, &pawn_move);
    move_tree_t *history = move_tree_new (NULL, pawn_move);

    move_list_t *move_list = generate_moves (board, COLOR_WHITE, history);
    move_t *mv, *en_passant_move;

    int en_passant_count = 0;
    for_each_move (mv, move_list)
    {
        if (is_en_passant_move(mv))
        {
            en_passant_move = mv;
            en_passant_count++;
        }
    }

    REQUIRE( en_passant_count == 1 );
    // Check move types:
    CHECK( is_capture_move(en_passant_move) );
    CHECK( PIECE_TYPE(move_get_taken(en_passant_move)) == PIECE_PAWN );
    CHECK( PIECE_COLOR(move_get_taken(en_passant_move)) == COLOR_BLACK );

    // Check position:
    CHECK( ROW(MOVE_SRC(*en_passant_move)) == 3 );
    CHECK( COLUMN(MOVE_SRC(*en_passant_move)) == 4 );
    CHECK( ROW(MOVE_DST(*en_passant_move)) == 2 );
    CHECK( COLUMN(MOVE_DST(*en_passant_move)) == 5 );

    do_move (board, COLOR_WHITE, en_passant_move);

    piece_t en_passant_pawn = PIECE_AT (board, 2, 5);
    CHECK( PIECE_TYPE(en_passant_pawn) == PIECE_PAWN );
    CHECK( PIECE_COLOR(en_passant_pawn) == COLOR_WHITE );
    piece_t taken_pawn = PIECE_AT (board, 3, 4);
    CHECK( PIECE_COLOR(taken_pawn) == COLOR_NONE );
    CHECK( PIECE_TYPE(taken_pawn) == PIECE_NONE );

    undo_move (board, COLOR_WHITE, en_passant_move);

    piece_t en_passant_pawn_space = PIECE_AT (board, 2, 5);
    CHECK( PIECE_TYPE(en_passant_pawn_space) == PIECE_NONE );
    CHECK( PIECE_COLOR(en_passant_pawn_space) == COLOR_NONE );
    piece_t en_passant_pawn_done = PIECE_AT (board, 3, 4);
    CHECK( PIECE_TYPE(en_passant_pawn_done) == PIECE_PAWN );
    CHECK( PIECE_COLOR(en_passant_pawn_done) == COLOR_WHITE );
    piece_t taken_pawn_undone = PIECE_AT (board, 3, 5);
    CHECK( PIECE_COLOR(taken_pawn_undone) == COLOR_BLACK );
    CHECK( PIECE_TYPE(taken_pawn_undone) == PIECE_PAWN );

}