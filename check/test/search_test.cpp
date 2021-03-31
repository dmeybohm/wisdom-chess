#include "doctest/doctest.h"
#include "board_builder.hpp"

#include "board.hpp"
#include "search.hpp"
#include "move.hpp"
#include "move_timer.hpp"
#include "move_tree.hpp"
#include "logger.hpp"
#include "history.hpp"
#include "fen_parser.hpp"
#include "game.hpp"

wisdom::NullLogger discard_output;
using namespace wisdom;

// Mating moves: : 1.Ra6 f6 2.Bxf6 Rg7 3.Rxa8#
TEST_CASE("Can find mate in 3")
{
    BoardBuilder builder;
    MoveTimer large_timer { 30 };

    builder.add_pieces (
            Color::Black, {
                    { "a8", Piece::Rook },
                    { "g8", Piece::Rook },
                    { "h8", Piece::King },
                    { "f7", Piece::Pawn },
                    { "h7", Piece::Pawn },
            }
    );
    builder.add_pieces (
            Color::White, {
                    { "f6", Piece::Rook },
                    { "e5", Piece::Bishop },
                    { "h2", Piece::Pawn },
                    { "h1", Piece::King },
            }
    );

    Board board = builder.build ();
    std::unique_ptr<MoveTree> variation;
    History history;
    IterativeSearch search { board, history, discard_output, large_timer, 5 };

    SearchResult result = search.iteratively_deepen (Color::White);

    REQUIRE(result.score > Infinity);
    REQUIRE(result.move.has_value());
    REQUIRE(result.variation_glimpse.size () == 5);

    MoveList expected_moves = { Color::White, { "f6 a6", "f7 f6", "e5xf6", "g8 g7", "a6xa8" }};
    MoveList computed_moves = result.variation_glimpse.to_list ();

    REQUIRE(expected_moves == computed_moves);
}

//
// This position has multiple mating chances, as well as stalements, so this will test if the
// search can find the most efficient mate.
//
// Mating moves:
// ... Rd4+ 2. Ke5 f6#
// ... Bb7+ 2. Ke5 Re4#
//
TEST_CASE("Can find mate in 2 1/2")
{
    BoardBuilder builder;
    MoveTimer large_timer { 30 };

    builder.add_pieces (
            Color::Black, {
                    { "e8", Piece::Knight },
                    { "c7", Piece::King },
                    { "f7", Piece::Pawn },
                    { "a6", Piece::Pawn },
                    { "g6", Piece::Pawn },
                    { "c5", Piece::Pawn },
                    { "b4", Piece::Rook },
                    { "b3", Piece::Knight },
            }
    );

    builder.add_piece ("d5", Color::White, Piece::King);

    Board board = builder.build ();
    std::unique_ptr<MoveTree> variation;
    History history;
    IterativeSearch search { board, history, discard_output, large_timer, 5 };

    SearchResult result = search.iteratively_deepen (Color::Black);
    REQUIRE(result.move.has_value());

    // Used to return this before move reordering.
//    MoveList expected_mate = { Color::Black, { "e8 f6", "d5 e5", "f6 g4", "e5 d5", "b4 d4" }};
    MoveList expected_mate = { Color::Black, { "e8 f6", "d5 e5", "f6 d7", "e5 d5", "b4 d4" }};
//    MoveList expected_mate = { Color::Black, { "c7 d7", "d5 e5", "b4 b8", "e5 d5", "b8 c8" }};
    MoveList computed_moves = result.variation_glimpse.to_list ();

    REQUIRE(result.score > Infinity);
    REQUIRE(result.variation_glimpse.size () == 5);
    REQUIRE(expected_mate == computed_moves);
}

TEST_CASE("scenario with heap overflow 1")
{
    BoardBuilder builder;

    builder.add_pieces (
            Color::Black, {
                    { "c8", Piece::Rook },
                    { "f8", Piece::Rook },
                    { "h8", Piece::King }
            }
    );
    builder.add_pieces (
            Color::Black, {
                    { "c7", Piece::Pawn },
                    { "h7", Piece::Knight },
            }
    );
    builder.add_pieces (
            Color::Black, {
                    { "a6", Piece::Pawn },
                    { "c6", Piece::Bishop },
                    { "b5", Piece::Pawn },
                    { "d5", Piece::Pawn }
            }
    );

    builder.add_pieces (
            Color::White, {
                    { "e5", Piece::Pawn },
                    { "a3", Piece::Knight },
                    { "c3", Piece::Pawn },
                    { "e3", Piece::Pawn },
                    { "a2", Piece::Pawn },
                    { "b2", Piece::Pawn },
                    { "h2", Piece::Pawn },
                    { "b1", Piece::King },
                    { "g1", Piece::Rook }
            }
    );

    Board board = builder.build ();
    MoveTimer timer { 300 };
    History history;
    IterativeSearch search { board, history, discard_output, timer, 3 };

    SearchResult result = search.iteratively_deepen (Color::Black);
    REQUIRE(result.move.has_value());
}

TEST_CASE("Promoting move is taken if possible")
{
    BoardBuilder builder;
    MoveTimer large_timer { 30 };

    builder.add_pieces (
            Color::Black, {
                    { "d7", Piece::King },
                    { "d2", Piece::Pawn }
            }
    );
    builder.add_pieces (
            Color::White, {
                    { "a4", Piece::King },
                    { "h4", Piece::Pawn }
            }
    );

    History history;
    Board board = builder.build ();
    SearchResult result = search (
            board, Color::Black, discard_output, history, large_timer,
            1, 1, -Initial_Alpha, Initial_Alpha
    );

    REQUIRE(to_string (*result.move) == "d2 d1(Q)");
}

TEST_CASE("Promoted pawn is promoted to highest value piece even when capturing")
{
    FenParser parser { "rnb1kbnr/ppp1pppp/8/8/3B4/8/PPP1pPPP/RN2KB1R b KQkq - 0 1" };

    auto game = parser.build ();

    History history;
    MoveTimer timer { 30 };
    IterativeSearch search { game.board, history, discard_output, timer, 3 };

    SearchResult result = search.iteratively_deepen (Color::Black);
    REQUIRE(to_string (*result.move) == "e2xf1(Q)");
}

TEST_CASE("Finding moves regression test")
{
    FenParser parser { "r5rk/5p1p/5R2/4B3/8/8/7P/7K w - - 0 1" };

    auto game = parser.build ();

    History history;
    MoveTimer timer { 10 };
    IterativeSearch search { game.board, history, discard_output, timer, 1 };

    SearchResult result = search.iteratively_deepen (Color::White);
    REQUIRE( result.move.has_value () );
}

TEST_CASE("Promoting to correct piece")
{
    // todo: load test-messed-up.gam here and ensure promoted piece search with depth 3 is Queen
    FenParser fen { "r1bqk1nr/ppp2ppp/8/4p3/1bpP4/2P5/PP2NPPP/RNBQ1RK1 w Qkq - 0 1" };
    auto game = fen.build();

    History history;
    MoveTimer timer { 180, false };
    StandardLogger logger;
    IterativeSearch search { game.board, history, logger, timer, 3 };

    SearchResult result = search.iteratively_deepen (Color::Black);

//    SearchResult result = search (game.board, Color::Black, logger, history, timer,
//                                  3, 3, -Initial_Alpha, Initial_Alpha);

    REQUIRE( result.move.has_value () );

    do_move (game.board, Color::Black, *result.move);
    // assert the bishop has moved:
    INFO("Info:", to_string (*result.move) );
    REQUIRE( piece_at (game.board, coord_parse("b4")) != make_piece (Color::Black, Piece::Bishop) );
}