#include "catch.hpp"
#include "board_builder.hpp"

#include "board.h"
#include "move_tree.h"
#include "generate.h"

TEST_CASE( "En passant state starts out as negative 1", "[en-passant]")
{
    struct board board;

    REQUIRE( !is_en_passant_vulnerable (board, COLOR_WHITE) );
    REQUIRE( !is_en_passant_vulnerable (board, COLOR_BLACK) );

    board_builder builder;
    std::vector<enum piece_type> back_rank {
            PIECE_ROOK,   PIECE_KNIGHT, PIECE_BISHOP, PIECE_QUEEN, PIECE_KING,
            PIECE_BISHOP, PIECE_KNIGHT, PIECE_ROOK
    };
    builder.add_row_of_same_color ("a8", COLOR_BLACK, back_rank);
    builder.add_row_of_same_color_and_piece ("a7", COLOR_BLACK, PIECE_PAWN);
    builder.add_piece ("e5", COLOR_WHITE, PIECE_PAWN);
    builder.add_row_of_same_color ("a1", COLOR_WHITE, back_rank);

    struct board builder_board = builder.build();

    REQUIRE( !is_en_passant_vulnerable (builder_board, COLOR_WHITE) );
    REQUIRE( !is_en_passant_vulnerable (builder_board, COLOR_BLACK) );
}

TEST_CASE( "En passant moves work on the right", "[en-passant]" )
{
    board_builder builder;
    std::vector<enum piece_type> back_rank {
            PIECE_ROOK,   PIECE_KNIGHT, PIECE_BISHOP, PIECE_QUEEN, PIECE_KING,
            PIECE_BISHOP, PIECE_KNIGHT, PIECE_ROOK
    };
    builder.add_row_of_same_color ("a8", COLOR_BLACK, back_rank);
    builder.add_row_of_same_color_and_piece ("a7", COLOR_BLACK, PIECE_PAWN);
    builder.add_piece ("e5", COLOR_WHITE, PIECE_PAWN);
    builder.add_row_of_same_color ("a1", COLOR_WHITE, back_rank);

    struct board board = builder.build();

    move_t pawn_move = parse_move ("f7f5");
    undo_move_t first_undo_state = do_move (board, COLOR_BLACK, pawn_move);
    REQUIRE( !is_en_passant_vulnerable (first_undo_state, COLOR_BLACK) );
    REQUIRE( !is_en_passant_vulnerable (first_undo_state, COLOR_WHITE) );

    move_list_t move_list = generate_moves (board, COLOR_WHITE);
    move_t en_passant_move = null_move;

    for (auto move : move_list)
    {
        if (is_en_passant_move(move))
            en_passant_move = move;
    }

    REQUIRE( !is_null_move(en_passant_move) );

    // Check move types:
    REQUIRE( is_en_passant_move(en_passant_move) );

    // Check position:
    REQUIRE( ROW(MOVE_SRC(en_passant_move)) == 3 );
    REQUIRE( COLUMN(MOVE_SRC(en_passant_move)) == 4 );
    REQUIRE( ROW(MOVE_DST(en_passant_move)) == 2 );
    REQUIRE( COLUMN(MOVE_DST(en_passant_move)) == 5 );

    undo_move_t en_passant_undo_state = do_move (board, COLOR_WHITE, en_passant_move);

    REQUIRE( en_passant_undo_state.category == MOVE_CATEGORY_EN_PASSANT );
    REQUIRE( is_en_passant_vulnerable (en_passant_undo_state, COLOR_BLACK) );
    REQUIRE( !is_en_passant_vulnerable (en_passant_undo_state, COLOR_WHITE) );

    piece_t en_passant_pawn = PIECE_AT (board, 2, 5);
    REQUIRE( PIECE_TYPE(en_passant_pawn) == PIECE_PAWN );
    REQUIRE( PIECE_COLOR(en_passant_pawn) == COLOR_WHITE );
    piece_t taken_pawn = PIECE_AT (board, 3, 4);
    REQUIRE( PIECE_COLOR(taken_pawn) == COLOR_NONE );
    REQUIRE( PIECE_TYPE(taken_pawn) == PIECE_NONE );

    undo_move (board, COLOR_WHITE, en_passant_move, en_passant_undo_state);
    REQUIRE( is_en_passant_vulnerable (board, COLOR_BLACK) );

    piece_t en_passant_pawn_space = PIECE_AT (board, 2, 5);
    REQUIRE( PIECE_TYPE(en_passant_pawn_space) == PIECE_NONE );
    REQUIRE( PIECE_COLOR(en_passant_pawn_space) == COLOR_NONE );
    piece_t en_passant_pawn_done = PIECE_AT (board, 3, 4);
    REQUIRE( PIECE_TYPE(en_passant_pawn_done) == PIECE_PAWN );
    REQUIRE( PIECE_COLOR(en_passant_pawn_done) == COLOR_WHITE );
    piece_t taken_pawn_undone = PIECE_AT (board, 3, 5);
    REQUIRE( PIECE_COLOR(taken_pawn_undone) == COLOR_BLACK );
    REQUIRE( PIECE_TYPE(taken_pawn_undone) == PIECE_PAWN );
}

TEST_CASE( "En passant moves work on the left", "[en-passant]" )
{
    board_builder builder;
    std::vector<enum piece_type> back_rank {
            PIECE_ROOK,   PIECE_KNIGHT, PIECE_BISHOP, PIECE_QUEEN, PIECE_KING,
            PIECE_BISHOP, PIECE_KNIGHT, PIECE_ROOK
    };
    builder.add_row_of_same_color ("a8", COLOR_BLACK, back_rank);
    builder.add_row_of_same_color_and_piece ("a7", COLOR_BLACK, PIECE_PAWN);
    builder.add_piece ("e5", COLOR_WHITE, PIECE_PAWN);
    builder.add_row_of_same_color ("a1", COLOR_WHITE, back_rank);

    struct board board = builder.build();
    move_t pawn_move = parse_move ("d7d5");
    undo_move_t first_undo_state = do_move (board, COLOR_BLACK, pawn_move);
    REQUIRE( !is_en_passant_vulnerable (first_undo_state, COLOR_BLACK) );
    REQUIRE( !is_en_passant_vulnerable (first_undo_state, COLOR_WHITE) );

    move_list_t move_list = generate_moves (board, COLOR_WHITE);
    move_t en_passant_move = null_move;

    for (auto move : move_list)
    {
        if (is_en_passant_move(move))
            en_passant_move = move;
    }

    REQUIRE( !is_null_move(en_passant_move) );

    // Check move types:
    REQUIRE( is_en_passant_move(en_passant_move) );

    // Check position:
    REQUIRE( ROW(MOVE_SRC(en_passant_move)) == 3 );
    REQUIRE( COLUMN(MOVE_SRC(en_passant_move)) == 4 );
    REQUIRE( ROW(MOVE_DST(en_passant_move)) == 2 );
    REQUIRE( COLUMN(MOVE_DST(en_passant_move)) == 3 );

    undo_move_t en_passant_undo_state = do_move (board, COLOR_WHITE, en_passant_move);

    REQUIRE( en_passant_undo_state.category == MOVE_CATEGORY_EN_PASSANT );
    REQUIRE( is_en_passant_vulnerable (en_passant_undo_state, COLOR_BLACK) );
    REQUIRE( !is_en_passant_vulnerable (en_passant_undo_state, COLOR_WHITE) );

    piece_t en_passant_pawn = PIECE_AT (board, 2, 3);
    REQUIRE( PIECE_TYPE(en_passant_pawn) == PIECE_PAWN );
    REQUIRE( PIECE_COLOR(en_passant_pawn) == COLOR_WHITE );
    piece_t taken_pawn = PIECE_AT (board, 3, 4);
    REQUIRE( PIECE_COLOR(taken_pawn) == COLOR_NONE );
    REQUIRE( PIECE_TYPE(taken_pawn) == PIECE_NONE );

    undo_move (board, COLOR_WHITE, en_passant_move, en_passant_undo_state);
    REQUIRE( is_en_passant_vulnerable (board, COLOR_BLACK) );

    piece_t en_passant_pawn_space = PIECE_AT (board, 2, 5);
    REQUIRE( PIECE_TYPE(en_passant_pawn_space) == PIECE_NONE );
    REQUIRE( PIECE_COLOR(en_passant_pawn_space) == COLOR_NONE );
    piece_t en_passant_pawn_done = PIECE_AT (board, 3, 4);
    REQUIRE( PIECE_TYPE(en_passant_pawn_done) == PIECE_PAWN );
    REQUIRE( PIECE_COLOR(en_passant_pawn_done) == COLOR_WHITE );
    piece_t taken_pawn_undone = PIECE_AT (board, 3, 3);
    REQUIRE( PIECE_COLOR(taken_pawn_undone) == COLOR_BLACK );
    REQUIRE( PIECE_TYPE(taken_pawn_undone) == PIECE_PAWN );
}

TEST_CASE( "En passant state is reset after en passant", "[en-passant]" )
{
    board_builder builder;
    std::vector<enum piece_type> back_rank {
            PIECE_ROOK,   PIECE_KNIGHT, PIECE_BISHOP, PIECE_QUEEN, PIECE_KING,
            PIECE_BISHOP, PIECE_KNIGHT, PIECE_ROOK
    };
    builder.add_row_of_same_color ("a8", COLOR_BLACK, back_rank);
    builder.add_row_of_same_color_and_piece ("a7", COLOR_BLACK, PIECE_PAWN);
    builder.add_piece ("e5", COLOR_WHITE, PIECE_PAWN);
    builder.add_row_of_same_color ("a1", COLOR_WHITE, back_rank);

    struct board board = builder.build();
    move_t first_move = parse_move ("d7d5");
    move_t en_passant = parse_move ("e5 d4 (ep)");

    undo_move_t first_undo = do_move (board, COLOR_BLACK, first_move);
    REQUIRE( is_en_passant_vulnerable (board, COLOR_BLACK) );
    REQUIRE( !is_en_passant_vulnerable (board, COLOR_WHITE) );

    undo_move_t second_undo = do_move (board, COLOR_WHITE, en_passant);
    REQUIRE( !is_en_passant_vulnerable (board, COLOR_BLACK) );
    REQUIRE( !is_en_passant_vulnerable (board, COLOR_WHITE) );

    undo_move (board, COLOR_WHITE, en_passant, second_undo);
    REQUIRE( is_en_passant_vulnerable (board, COLOR_BLACK) );
    REQUIRE( !is_en_passant_vulnerable (board, COLOR_WHITE) );

    undo_move (board, COLOR_BLACK, first_move, first_undo);
    REQUIRE( !is_en_passant_vulnerable (board, COLOR_BLACK) );
    REQUIRE( !is_en_passant_vulnerable (board, COLOR_WHITE) );
}
