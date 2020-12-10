#include <catch/catch.hpp>
#include "board_builder.hpp"

#include "board.h"
#include "position.h"
#include "move.h"

TEST_CASE("Position is initialized correctly", "[position-test]")
{
    struct board board;

    CHECK( board.position.score[COLOR_INDEX_WHITE] < 0 );
    CHECK( board.position.score[COLOR_INDEX_BLACK] < 0 );
    CHECK( board.position.score[COLOR_INDEX_WHITE] ==
           board.position.score[COLOR_INDEX_BLACK]);
}

TEST_CASE( "Center pawn elevates position score", "[position-test]" )
{
    board_builder builder;

    builder.add_piece("e1", Color::White, Piece::King);
    builder.add_piece("e8", Color::Black, Piece::King);

    builder.add_piece("e4", Color::White, Piece::Pawn);
    builder.add_piece("a6", Color::Black, Piece::Pawn);
    
    struct board board = builder.build();

    CHECK( board.position.score[COLOR_INDEX_WHITE] > board.position.score[COLOR_INDEX_BLACK]);
}

TEST_CASE( "Capture updates position score correctly", "[position-test]")
{
    board_builder builder;

    builder.add_piece("e1", Color::White, Piece::King);
    builder.add_piece("e8", Color::Black, Piece::King);

    builder.add_piece("e4", Color::White, Piece::Knight);
    builder.add_piece("d6", Color::Black, Piece::Pawn);

    struct board board = builder.build();

    int initial_score_white = position_score (&board.position, Color::White);
    int initial_score_black = position_score (&board.position, Color::Black);

    move_t e4xd6 = move_parse ("e4xd6", Color::White);

    undo_move_t undo_state = do_move (board, Color::White, e4xd6);
    undo_move (board, Color::White, e4xd6, undo_state);

    CHECK( initial_score_white == position_score (&board.position, Color::White) );
    CHECK( initial_score_black == position_score (&board.position, Color::Black) );
}

TEST_CASE( "En passant updates position score correctly", "[position-test]")
{
    board_builder builder;

    builder.add_piece("e1", Color::White, Piece::King);
    builder.add_piece("e8", Color::Black, Piece::King);

    builder.add_piece("e5", Color::White, Piece::Pawn);
    builder.add_piece("d5", Color::Black, Piece::Pawn);

    struct board board = builder.build();

    int initial_score_white = position_score (&board.position, Color::White);
    int initial_score_black = position_score (&board.position, Color::Black);

    move_t e5xd5 = move_parse ("e5d6 ep", Color::White);
    CHECK( is_en_passant_move(e5xd5) );

    undo_move_t undo_state = do_move (board, Color::White, e5xd5);
    undo_move (board, Color::White, e5xd5, undo_state);

    CHECK( initial_score_white == position_score (&board.position, Color::White) );
    CHECK( initial_score_black == position_score (&board.position, Color::Black) );
}

TEST_CASE( "Castling updates position score correctly", "[position-test]")
{
    board_builder builder;

    builder.add_piece("e1", Color::White, Piece::King);
    builder.add_piece("e8", Color::Black, Piece::King);

    builder.add_piece("h1", Color::White, Piece::Rook);
    builder.add_piece("a1", Color::White, Piece::Rook);
    builder.add_piece("d5", Color::Black, Piece::Pawn);

    struct board board = builder.build();
    int initial_score_white = position_score (&board.position, Color::White);
    int initial_score_black = position_score (&board.position, Color::Black);

    auto castling_move_in = GENERATE( "o-o", "o-o-o");
    move_t castling_move = move_parse (castling_move_in, Color::White);
    CHECK( is_castling_move(castling_move) );

    undo_move_t undo_state = do_move (board, Color::White, castling_move);
    undo_move (board, Color::White, castling_move, undo_state);

    CHECK( initial_score_white == position_score (&board.position, Color::White) );
    CHECK( initial_score_black == position_score (&board.position, Color::Black) );
}

TEST_CASE( "Promoting move updates position score correctly", "[position-test]")
{
    board_builder builder;

    builder.add_piece("e1", Color::White, Piece::King);
    builder.add_piece("e8", Color::Black, Piece::King);

    builder.add_piece("h7", Color::White, Piece::Pawn);

    struct board board = builder.build();
    int initial_score_white = position_score (&board.position, Color::White);
    int initial_score_black = position_score (&board.position, Color::Black);

    auto promoting_move_in = GENERATE( "h7h8 (Q)", "h7h8 (R)", "h7h8 (B)", "h7h8 (N)");
    move_t castling_move = move_parse (promoting_move_in, Color::White);
    CHECK( is_promoting_move(castling_move) );

    undo_move_t undo_state = do_move (board, Color::White, castling_move);
    undo_move (board, Color::White, castling_move, undo_state);

    CHECK( initial_score_white == position_score (&board.position, Color::White) );
    CHECK( initial_score_black == position_score (&board.position, Color::Black) );
}