#include <doctest/doctest.h>
#include "game.hpp"
#include "board.hpp"
#include "fen_parser.hpp"

using namespace wisdom;

TEST_CASE( "FEN notation for the starting position" )
{
    FenParser parser { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" };

    Game game = parser.build ();
    Board default_board;

    CHECK(game.getBoard () == default_board );

    auto default_black_state = default_board.getCastlingEligibility (Color::Black);
    auto default_white_state = default_board.getCastlingEligibility (Color::White);
    auto fen_white_state = game.getBoard ().getCastlingEligibility (Color::White);
    auto fen_black_state = game.getBoard ().getCastlingEligibility (Color::Black);

    CHECK( default_white_state == fen_white_state );
    CHECK( default_black_state == fen_black_state );
}

TEST_CASE( "FEN notation for non-starting position" )
{
    FenParser parser { "4r2/8/8/8/8/8/k7/4K2R w K - 0 1" };

    Game game  = parser.build();

    BoardBuilder builder;

    builder.add_piece ("e8", Color::Black, Piece::Rook);
    builder.add_piece ("a2", Color::Black, Piece::King);
    builder.add_piece ("e1", Color::White, Piece::King);
    builder.add_piece ("h1", Color::White, Piece::Rook);

    auto expected = Board { builder };
    CHECK(game.getBoard () == expected );

    auto white_state = game.getBoard ().getCastlingEligibility (Color::White);
    auto black_state = game.getBoard ().getCastlingEligibility (Color::Black);
    auto exp_white_state = expected.getCastlingEligibility (Color::White);
    auto exp_black_state = expected.getCastlingEligibility (Color::Black);

    CHECK( white_state == exp_white_state );
    CHECK( black_state == exp_black_state );
}

TEST_CASE( "FEN notation for castling" )
{
    FenParser parser_full { "4r2/8/8/8/8/8/k7/4K2R w KQkq - 0 1" };

    Game game = parser_full.build();

    REQUIRE(game.getBoard ().getCastlingEligibility (Color::White) == CastlingEligible::EitherSideEligible);
    REQUIRE(game.getBoard ().getCastlingEligibility (Color::Black) == CastlingEligible::EitherSideEligible);

    FenParser parser_no_black_king { "4r2/8/8/8/8/8/k7/4K2R w KQq - 0 1" };

    game = parser_no_black_king.build();

    REQUIRE(game.getBoard ().getCastlingEligibility (Color::White) == CastlingEligible::EitherSideEligible);
    REQUIRE(game.getBoard ().getCastlingEligibility (Color::Black) == CastlingEligible::KingsideIneligible);

    FenParser parser_no_black { "4r2/8/8/8/8/8/k7/4K2R w KQq - 0 1" };

    game = parser_no_black.build();

    REQUIRE(game.getBoard ().getCastlingEligibility (Color::White) == CastlingEligible::EitherSideEligible);
    REQUIRE(game.getBoard ().getCastlingEligibility (Color::Black) == CastlingEligible::KingsideIneligible);
    FenParser parser_nothing { "4r2/8/8/8/8/8/k7/4K2R w - - 0 1" };

    game = parser_nothing.build();

    REQUIRE(game.getBoard ().getCastlingEligibility (Color::White) == CastlingEligible::BothSidesIneligible );
    REQUIRE(game.getBoard ().getCastlingEligibility (Color::Black) == CastlingEligible::BothSidesIneligible );
}

TEST_CASE( "FEN notation for en passant" )
{
    FenParser parser_with_black_target { "4r2/8/8/8/8/8/k7/4K2R w KQkq e6 0 1"};

    Game game = parser_with_black_target.build();
    auto &board = game.getBoard ();

    REQUIRE( !board.isEnPassantVulnerable (Color::White) );
    REQUIRE(board.isEnPassantVulnerable (Color::Black) );

    auto black_target = board.getEnPassantTarget (Color::Black);
    REQUIRE( black_target == coord_parse ("e6") );
}

TEST_CASE( "Parsing half and full moves")
{
    SUBCASE( "With castling and en passant square" )
    {
        FenParser parser { "4r2/8/8/8/8/8/k7/4K2R w Kk e6 10 5" };

        Game game = parser.build ();
        const auto& board = game.getBoard ();
        CHECK (board.getHalfMoveClock () == 10);
        CHECK (board.getFullMoveClock () == 5);
    }

    SUBCASE( "Without castling or en passant square" )
    {
        FenParser parser { "4r2/8/8/8/8/8/k7/4K2R w - - 10 5" };

        Game game = parser.build ();
        const auto& board = game.getBoard ();
        CHECK (board.getHalfMoveClock () == 10);
        CHECK (board.getFullMoveClock () == 5);
    }
}