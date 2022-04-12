#include "analytics.hpp"
#include "board.hpp"
#include "board_builder.hpp"
#include "check.hpp"
#include "fen_parser.hpp"
#include "game.hpp"
#include "history.hpp"
#include "logger.hpp"
#include "move.hpp"
#include "move_timer.hpp"
#include "move_tree.hpp"
#include "search.hpp"
#include "tests.hpp"

namespace wisdom::test
{
    using namespace wisdom;

    struct SearchHelper
    {
        History history {};

        IterativeSearch build (Board& board, int depth, int time = 30)
        {
            MoveTimer timer { time };
            return { board, history, make_null_logger (), timer, depth };
        }
    };
}

using wisdom::test::SearchHelper;
using namespace wisdom;

// Mating moves: : 1.Ra6 f6 2.Bxf6 Rg7 3.Rxa8#
TEST_CASE ("Can find mate in 3")
{
    BoardBuilder builder;

    builder.add_pieces (Color::Black,
                        {
                            { "a8", Piece::Rook },
                            { "g8", Piece::Rook },
                            { "h8", Piece::King },
                            { "f7", Piece::Pawn },
                            { "h7", Piece::Pawn },
                        });
    builder.add_pieces (Color::White,
                        {
                            { "f6", Piece::Rook },
                            { "e5", Piece::Bishop },
                            { "h2", Piece::Pawn },
                            { "h1", Piece::King },
                        });

    auto board = builder.build ();
    SearchHelper helper;
    IterativeSearch search = helper.build (*board, 5);

    SearchResult result = search.iteratively_deepen (Color::White);

    CHECK (result.score > Infinity);
    CHECK (result.move.has_value ());
}

//
// This position has multiple mating chances, as well as stalements, so this will test if the
// search can find the most efficient mate.
//
// Mating moves:
// ... Rd4+ 2. Ke5 f6#
// ... Bb7+ 2. Ke5 Re4#
//
TEST_CASE ("Can find mate in 2 1/2")
{
    BoardBuilder builder;

    builder.add_pieces (Color::Black,
                        {
                            { "e8", Piece::Knight },
                            { "c7", Piece::King },
                            { "f7", Piece::Pawn },
                            { "a6", Piece::Pawn },
                            { "g6", Piece::Pawn },
                            { "c5", Piece::Pawn },
                            { "b4", Piece::Rook },
                            { "b3", Piece::Knight },
                        });

    builder.add_piece ("d5", Color::White, Piece::King);

    auto board = builder.build ();
    SearchHelper helper;
    IterativeSearch search = helper.build (*board, 5);

    SearchResult result = search.iteratively_deepen (Color::Black);
    REQUIRE (result.move.has_value ());

    // Used to return this before move reordering.
    //    MoveList expected_mate = { Color::Black, { "e8 f6", "d5 e5", "f6 g4", "e5 d5", "b4 d4" }};
    //    MoveList expected_mate = { Color::Black, { "e8 f6", "d5 e5", "f6 d7", "e5 d5", "b4 d4" }};
    //    MoveList expected_mate = { Color::Black, { "c7 d7", "d5 e5", "b4 b8", "e5 d5", "b8 c8" }};
    MoveList computed_moves = result.variation_glimpse.to_list ();

    REQUIRE (result.score > Infinity);
    //    REQUIRE(expected_mate == computed_moves);
}

TEST_CASE ("scenario with heap overflow 1")
{
    BoardBuilder builder;

    builder.add_pieces (Color::Black,
                        { { "c8", Piece::Rook }, { "f8", Piece::Rook }, { "h8", Piece::King } });
    builder.add_pieces (Color::Black,
                        {
                            { "c7", Piece::Pawn },
                            { "h7", Piece::Knight },
                        });
    builder.add_pieces (Color::Black,
                        { { "a6", Piece::Pawn },
                          { "c6", Piece::Bishop },
                          { "b5", Piece::Pawn },
                          { "d5", Piece::Pawn } });

    builder.add_pieces (Color::White,
                        { { "e5", Piece::Pawn },
                          { "a3", Piece::Knight },
                          { "c3", Piece::Pawn },
                          { "e3", Piece::Pawn },
                          { "a2", Piece::Pawn },
                          { "b2", Piece::Pawn },
                          { "h2", Piece::Pawn },
                          { "b1", Piece::King },
                          { "g1", Piece::Rook } });

    auto board = builder.build ();
    SearchHelper helper;
    IterativeSearch search = helper.build (*board, 3, 300);

    SearchResult result = search.iteratively_deepen (Color::Black);
    REQUIRE (result.move.has_value ());
}

TEST_CASE ("Promoting move is taken if possible")
{
    BoardBuilder builder;

    builder.add_pieces (Color::Black, { { "d7", Piece::King }, { "d2", Piece::Pawn } });
    builder.add_pieces (Color::White, { { "a4", Piece::King }, { "h4", Piece::Pawn } });

    auto board = builder.build ();
    SearchHelper helper;

    IterativeSearch search = helper.build (*board, 1, 30);
    auto result = search.iteratively_deepen (Color::Black);
    REQUIRE (to_string (*result.move) == "d2 d1(Q)");
}

TEST_CASE ("Promoted pawn is promoted to highest value piece even when capturing")
{
    FenParser parser { "rnb1kbnr/ppp1pppp/8/8/3B4/8/PPP1pPPP/RN2KB1R b KQkq - 0 1" };

    auto game = parser.build ();

    SearchHelper helper;
    IterativeSearch search = helper.build (game.get_board (), 3);

    auto board_str = game.get_board().to_string();
    INFO( board_str );

    SearchResult result = search.iteratively_deepen (Color::Black);
    REQUIRE (to_string (*result.move) == "e2xf1(Q)");
}

TEST_CASE ("Finding moves regression test")
{
    FenParser parser { "r5rk/5p1p/5R2/4B3/8/8/7P/7K w - - 0 1" };

    auto game = parser.build ();
    SearchHelper helper;
    History history;
    MoveTimer timer { 10 };
    IterativeSearch search = helper.build (game.get_board (), 1, 10);

    SearchResult result = search.iteratively_deepen (Color::White);
    REQUIRE (result.move.has_value ());
}

TEST_CASE ("Bishop is not sacrificed scenario 1")
{
    FenParser fen { "r1bqk1nr/ppp2ppp/8/4p3/1bpP4/2P5/PP2NPPP/RNBQ1RK1 w kq - 0 1" };
    auto game = fen.build ();

    SearchHelper helper;
    History history;
    IterativeSearch search = helper.build (game.get_board (), 3, 180);

    SearchResult result = search.iteratively_deepen (Color::Black);

    REQUIRE (result.move.has_value ());

    game.move (*result.move);

    // assert the bishop has moved:
    INFO ("Info:", to_string (*result.move));
    REQUIRE (game.get_board ().piece_at (coord_parse ("b4"))
             != make_piece (Color::Black, Piece::Bishop));
}

TEST_CASE ("Bishop is not sacrificed scenario 2 (as white)")
{
    FenParser fen { "2b2bnr/pr1ppkpp/p1p5/q3P3/2P2N2/BP3N2/P2P1PPP/R3K2R w KQ - 2 1" };
    auto game = fen.build ();

    SearchHelper helper;
    IterativeSearch search = helper.build (game.get_board (), 3, 180);

    SearchResult result = search.iteratively_deepen (Color::White);

    REQUIRE (result.move.has_value ());

    game.move (*result.move);

    // assert the bishop has moved:
    INFO ("Info:", to_string (*result.move));
    auto a3_piece = game.get_board ().piece_at (coord_parse ("a3"));
    bool bishop_sac = a3_piece != make_piece (Color::White, Piece::Bishop);
    bool is_in_check = is_king_threatened (game.get_board (), Color::Black,
                                           game.get_board ().get_king_position (Color::Black));
    bool bishop_sac_or_is_in_check = bishop_sac || is_in_check;
    REQUIRE (bishop_sac_or_is_in_check);
}

TEST_CASE ("Advanced pawn should be captured")
{
    FenParser fen { "rnb1k2r/ppp1qppp/4p3/3pP3/3P4/P1Q5/1PP2PPP/R3KBNR w KQkq d6 0 1" };
    auto game = fen.build ();

    SearchHelper helper;
    IterativeSearch search = helper.build (game.get_board (), 3, 10);

    game.move (move_parse ("e5 d6 ep", Color::White));

    SearchResult result = search.iteratively_deepen (Color::Black);

    REQUIRE (result.move.has_value ());

    game.move (*result.move);
    // assert the pawn at d6 has been taken:
    INFO ("Info:", to_string (*result.move));
    auto board = game.get_board ();
    REQUIRE (board.piece_at (coord_parse ("d6")) != make_piece (Color::White, Piece::Pawn));
}