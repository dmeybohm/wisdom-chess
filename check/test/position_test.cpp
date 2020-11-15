#include "catch.hpp"
#include "board_builder.hpp"

#include "board.h"
#include "position.h"
#include "move.h"

TEST_CASE("Position is initialized correctly", "[position-test]")
{
    struct board *board = board_new();

    CHECK( board->position.score[COLOR_INDEX_WHITE] < 0 );
    CHECK( board->position.score[COLOR_INDEX_BLACK] < 0 );
    CHECK( board->position.score[COLOR_INDEX_WHITE] ==
           board->position.score[COLOR_INDEX_BLACK]);
}

TEST_CASE( "Center pawn elevates position score", "[position-test]" )
{
    board_builder builder;

    builder.add_piece("e1", COLOR_WHITE, PIECE_KING);
    builder.add_piece("e8", COLOR_BLACK, PIECE_KING);

    builder.add_piece("e4", COLOR_WHITE, PIECE_PAWN);
    builder.add_piece("a6", COLOR_BLACK, PIECE_PAWN);
    struct board board = builder.build();

    CHECK( board->position.score[COLOR_INDEX_WHITE] > board->position.score[COLOR_INDEX_BLACK]);
}

TEST_CASE( "Capture updates position score correctly", "[position-test]")
{
    board_builder builder;

    builder.add_piece("e1", COLOR_WHITE, PIECE_KING);
    builder.add_piece("e8", COLOR_BLACK, PIECE_KING);

    builder.add_piece("e4", COLOR_WHITE, PIECE_KNIGHT);
    builder.add_piece("d6", COLOR_BLACK, PIECE_PAWN);

    struct board board = builder.build();

    int initial_score_white = position_score (&board->position, COLOR_WHITE);
    int initial_score_black = position_score (&board->position, COLOR_BLACK);

    move_t e4xd6 = move_parse ("e4xd6", COLOR_WHITE);

    undo_move_t undo_state = do_move (board, COLOR_WHITE, e4xd6);
    undo_move (board, COLOR_WHITE, e4xd6, undo_state);

    CHECK( initial_score_white == position_score (&board->position, COLOR_WHITE) );
    CHECK( initial_score_black == position_score (&board->position, COLOR_BLACK) );
}

TEST_CASE( "En passant updates position score correctly", "[position-test]")
{
    board_builder builder;

    builder.add_piece("e1", COLOR_WHITE, PIECE_KING);
    builder.add_piece("e8", COLOR_BLACK, PIECE_KING);

    builder.add_piece("e5", COLOR_WHITE, PIECE_PAWN);
    builder.add_piece("d5", COLOR_BLACK, PIECE_PAWN);

    struct board board = builder.build();

    int initial_score_white = position_score (&board->position, COLOR_WHITE);
    int initial_score_black = position_score (&board->position, COLOR_BLACK);

    move_t e5xd5 = move_parse ("e5d6 ep", COLOR_WHITE);
    CHECK( is_en_passant_move(e5xd5) );

    undo_move_t undo_state = do_move (board, COLOR_WHITE, e5xd5);
    undo_move (board, COLOR_WHITE, e5xd5, undo_state);

    CHECK( initial_score_white == position_score (&board->position, COLOR_WHITE) );
    CHECK( initial_score_black == position_score (&board->position, COLOR_BLACK) );
}

TEST_CASE( "Castling updates position score correctly", "[position-test]")
{
    board_builder builder;

    builder.add_piece("e1", COLOR_WHITE, PIECE_KING);
    builder.add_piece("e8", COLOR_BLACK, PIECE_KING);

    builder.add_piece("h1", COLOR_WHITE, PIECE_ROOK);
    builder.add_piece("a1", COLOR_WHITE, PIECE_ROOK);
    builder.add_piece("d5", COLOR_BLACK, PIECE_PAWN);

    struct board board = builder.build();

    int initial_score_white = position_score (&board->position, COLOR_WHITE);
    int initial_score_black = position_score (&board->position, COLOR_BLACK);

    auto castling_move_in = GENERATE( "o-o", "o-o-o");
    move_t castling_move = move_parse (castling_move_in, COLOR_WHITE);
    CHECK( is_castling_move(castling_move) );

    undo_move_t undo_state = do_move (board, COLOR_WHITE, castling_move);
    undo_move (board, COLOR_WHITE, castling_move, undo_state);

    CHECK( initial_score_white == position_score (&board->position, COLOR_WHITE) );
    CHECK( initial_score_black == position_score (&board->position, COLOR_BLACK) );
}

TEST_CASE( "Promoting move updates position score correctly", "[position-test]")
{
    board_builder builder;

    builder.add_piece("e1", COLOR_WHITE, PIECE_KING);
    builder.add_piece("e8", COLOR_BLACK, PIECE_KING);

    builder.add_piece("h7", COLOR_WHITE, PIECE_PAWN);

    struct board board = builder.build();

    int initial_score_white = position_score (&board->position, COLOR_WHITE);
    int initial_score_black = position_score (&board->position, COLOR_BLACK);

    auto promoting_move_in = GENERATE( "h7h8 (Q)", "h7h8 (R)", "h7h8 (B)", "h7h8 (N)");
    move_t castling_move = move_parse (promoting_move_in, COLOR_WHITE);
    CHECK( is_promoting_move(castling_move) );

    undo_move_t undo_state = do_move (board, COLOR_WHITE, castling_move);
    undo_move (board, COLOR_WHITE, castling_move, undo_state);

    CHECK( initial_score_white == position_score (&board->position, COLOR_WHITE) );
    CHECK( initial_score_black == position_score (&board->position, COLOR_BLACK) );
}