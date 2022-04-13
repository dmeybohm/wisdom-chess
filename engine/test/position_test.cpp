#include <doctest/doctest.h>
#include "board_builder.hpp"

#include "board.hpp"
#include "position.hpp"
#include "move.hpp"

using namespace wisdom;

TEST_CASE("Position is initialized correctly")
{
    Board board;

    CHECK(board.get_position ().individual_score (Color::White) < 0 );
    CHECK(board.get_position ().individual_score (Color::Black) < 0 );
    CHECK(board.get_position ().individual_score (Color::White) == board.get_position ().individual_score (Color::Black));
}

TEST_CASE( "Center pawn elevates position overall_score" )
{
    BoardBuilder builder;

    builder.add_piece("e1", Color::White, Piece::King);
    builder.add_piece("e8", Color::Black, Piece::King);

    builder.add_piece("e4", Color::White, Piece::Pawn);
    builder.add_piece("a6", Color::Black, Piece::Pawn);
    
    auto board = builder.build();

    CHECK(board->get_position ().overall_score (Color::White) > board->get_position ().overall_score (Color::Black));
}

TEST_CASE( "Capture updates position overall_score correctly")
{
    BoardBuilder builder;

    builder.add_piece("e1", Color::White, Piece::King);
    builder.add_piece("e8", Color::Black, Piece::King);

    builder.add_piece("e4", Color::White, Piece::Knight);
    builder.add_piece("d6", Color::Black, Piece::Pawn);

    auto board = builder.build();

    int initial_score_white = board->get_position ().overall_score (Color::White);
    int initial_score_black = board->get_position ().overall_score (Color::Black);

    Move e4xd6 = move_parse ("e4xd6", Color::White);

    UndoMove undo_state = board->make_move (Color::White, e4xd6);
    board->take_back (Color::White, e4xd6, undo_state);

    CHECK( initial_score_white == board->get_position ().overall_score (Color::White) );
    CHECK( initial_score_black == board->get_position ().overall_score (Color::Black) );
}

TEST_CASE( "En passant updates position overall_score correctly")
{
    BoardBuilder builder;

    builder.add_piece("e1", Color::White, Piece::King);
    builder.add_piece("e8", Color::Black, Piece::King);

    builder.add_piece("e5", Color::White, Piece::Pawn);
    builder.add_piece("d5", Color::Black, Piece::Pawn);

    auto board = builder.build();

    int initial_score_white = board->get_position ().overall_score (Color::White);
    int initial_score_black = board->get_position ().overall_score (Color::Black);

    Move e5xd5 = move_parse ("e5d6 ep", Color::White);
    CHECK(is_special_en_passant_move (e5xd5) );

    UndoMove undo_state = board->make_move (Color::White, e5xd5);
    board->take_back (Color::White, e5xd5, undo_state);

    CHECK( initial_score_white == board->get_position ().overall_score (Color::White) );
    CHECK( initial_score_black == board->get_position ().overall_score (Color::Black) );
}

TEST_CASE( "Castling updates position overall_score correctly")
{
    BoardBuilder builder;

    builder.add_piece("e1", Color::White, Piece::King);
    builder.add_piece("e8", Color::Black, Piece::King);

    builder.add_piece("h1", Color::White, Piece::Rook);
    builder.add_piece("a1", Color::White, Piece::Rook);
    builder.add_piece("d5", Color::Black, Piece::Pawn);

    auto board = builder.build();
    int initial_score_white = board->get_position ().overall_score (Color::White);
    int initial_score_black = board->get_position ().overall_score (Color::Black);

    std::vector castling_moves { "o-o", "o-o-o" };
    for (auto castling_move_in : castling_moves)
    {
        Move castling_move = move_parse (castling_move_in, Color::White);
        CHECK(is_special_castling_move (castling_move));

        UndoMove undo_state = board->make_move (Color::White, castling_move);
        board->take_back (Color::White, castling_move, undo_state);

        CHECK(initial_score_white == board->get_position ().overall_score (Color::White));
        CHECK(initial_score_black == board->get_position ().overall_score (Color::Black));
    }
}

TEST_CASE( "Promoting move updates position overall_score correctly")
{
    BoardBuilder builder;

    builder.add_piece("e1", Color::White, Piece::King);
    builder.add_piece("e8", Color::Black, Piece::King);

    builder.add_piece("h7", Color::White, Piece::Pawn);

    auto board = builder.build();
    int initial_score_white = board->get_position ().overall_score (Color::White);
    int initial_score_black = board->get_position ().overall_score (Color::Black);

    std::vector promoting_moves { "h7h8 (Q)", "h7h8 (R)", "h7h8 (B)", "h7h8 (N)" };
    for (auto promoting_move_in : promoting_moves)
    {
        Move castling_move = move_parse (promoting_move_in, Color::White);
        CHECK(is_promoting_move (castling_move));

        UndoMove undo_state = board->make_move (Color::White, castling_move);
        board->take_back (Color::White, castling_move, undo_state);

        CHECK( initial_score_white == board->get_position ().overall_score (Color::White) );
        CHECK( initial_score_black == board->get_position ().overall_score (Color::Black) );
    }
}

TEST_CASE("Double pawn moves are more appealing")
{
    Board board;

    auto e2e4 = move_parse ("e2e4");
    auto e7e5 = move_parse("e7e5");
    auto e7e6 = move_parse("e7e6");

    board.make_move (Color::White, e2e4);
    auto undo = board.make_move (Color::Black, e7e5);
    auto black_big_score = board.get_position ().individual_score (Color::Black);
    board.take_back (Color::Black, e7e5, undo);
    board.make_move (Color::Black, e7e6);
    auto black_small_score = board.get_position ().individual_score (Color::Black);

    REQUIRE( black_big_score > black_small_score );
}