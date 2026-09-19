#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "FEN notation for the starting position" )
{
    Game game = Game::createGameFromFen ("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    Board default_board;

    CHECK( game.getBoard() == default_board );

    auto default_black_state = default_board.getCastlingEligibility (Color::Black);
    auto default_white_state = default_board.getCastlingEligibility (Color::White);
    auto fen_white_state = game.getBoard().getCastlingEligibility (Color::White);
    auto fen_black_state = game.getBoard().getCastlingEligibility (Color::Black);

    CHECK( default_white_state == fen_white_state );
    CHECK( default_black_state == fen_black_state );
}

TEST_CASE( "FEN notation for non-starting position" )
{
    Game game = Game::createGameFromFen ("4r2/8/8/8/8/8/k7/4K2R w K - 0 1");

    BoardBuilder builder;

    builder.addPiece ("e8", Color::Black, Piece::Rook);
    builder.addPiece ("a2", Color::Black, Piece::King);
    builder.addPiece ("e1", Color::White, Piece::King);
    builder.addPiece ("h1", Color::White, Piece::Rook);

    auto expected = Board { builder };
    CHECK( game.getBoard() == expected );

    auto white_state = game.getBoard().getCastlingEligibility (Color::White);
    auto black_state = game.getBoard().getCastlingEligibility (Color::Black);
    auto exp_white_state = expected.getCastlingEligibility (Color::White);
    auto exp_black_state = expected.getCastlingEligibility (Color::Black);

    CHECK( white_state == exp_white_state );
    CHECK( black_state == exp_black_state );
}

TEST_CASE( "FEN notation for castling" )
{
    Game game = Game::createGameFromFen ("4r2/8/8/8/8/8/k7/4K2R w KQkq - 0 1");

    REQUIRE( game.getBoard().getCastlingEligibility (Color::White) == CastlingEligibility::Either_Side );
    REQUIRE( game.getBoard().getCastlingEligibility (Color::Black) == CastlingEligibility::Either_Side );

    game = Game::createGameFromFen ("4r2/8/8/8/8/8/k7/4K2R w KQq - 0 1");

    REQUIRE( game.getBoard().getCastlingEligibility (Color::White) == CastlingEligibility::Either_Side );
    REQUIRE( game.getBoard().getCastlingEligibility (Color::Black) == CastlingRights::Queenside );

    game = Game::createGameFromFen ("4r2/8/8/8/8/8/k7/4K2R w KQq - 0 1");

    REQUIRE( game.getBoard().getCastlingEligibility (Color::White) == CastlingEligibility::Either_Side );
    REQUIRE( game.getBoard().getCastlingEligibility (Color::Black) == CastlingRights::Queenside );

    game = Game::createGameFromFen ("4r2/8/8/8/8/8/k7/4K2R w - - 0 1");

    REQUIRE( game.getBoard().getCastlingEligibility (Color::White) == CastlingEligibility::Neither_Side );
    REQUIRE( game.getBoard().getCastlingEligibility (Color::Black) == CastlingEligibility::Neither_Side );
}

TEST_CASE( "FEN notation for en passant" )
{
    Game game = Game::createGameFromFen ("4r2/8/8/8/8/8/k7/4K2R w KQkq e6 0 1");
    auto &board = game.getBoard();

    REQUIRE( !board.isEnPassantVulnerable (Color::White) );
    REQUIRE( board.isEnPassantVulnerable (Color::Black) );

    auto black_target = board.getEnPassantTarget();
    REQUIRE( black_target.has_value() );
    CHECK( black_target->vulnerable_color == Color::Black );
    CHECK( black_target->coord == coordParse ("e6") );
}

TEST_CASE( "FEN notation with an invalid en passant square" )
{
    SUBCASE( "Square outside the board" )
    {
        CHECK_THROWS_AS(
            (void)Game::createGameFromFen ("4r2/8/8/8/8/8/k7/4K2R w KQkq z9 0 1"),
            FenParserError
        );
    }

    SUBCASE( "Square with a missing rank" )
    {
        CHECK_THROWS_AS(
            (void)Game::createGameFromFen ("4r2/8/8/8/8/8/k7/4K2R w KQkq e 0 1"),
            FenParserError
        );
    }
}

TEST_CASE( "Parsing half and full moves" )
{
    SUBCASE( "With castling and en passant square" )
    {
        Game game = Game::createGameFromFen ("4r2/8/8/8/8/8/k7/4K2R w Kk e6 10 5");
        const auto& board = game.getBoard();
        CHECK( board.getHalfMoveClock() == 10 );
        CHECK( board.getFullMoveClock() == 5 );
    }

    SUBCASE( "Without castling or en passant square" )
    {
        Game game = Game::createGameFromFen ("4r2/8/8/8/8/8/k7/4K2R w - - 10 5");
        const auto& board = game.getBoard();
        CHECK( board.getHalfMoveClock() == 10 );
        CHECK( board.getFullMoveClock() == 5 );
    }
}

TEST_CASE( "FEN parser rejects move clocks that are out of range" )
{
    SUBCASE( "Negative half move clock" )
    {
        CHECK_THROWS_AS(
            (void)Game::createGameFromFen ("4r3/8/8/8/8/8/k7/4K2R w - - -2 5"),
            FenParserError
        );
    }

    SUBCASE( "Negative full move number" )
    {
        CHECK_THROWS_AS(
            (void)Game::createGameFromFen ("4r3/8/8/8/8/8/k7/4K2R w - - 10 -5"),
            FenParserError
        );
    }

    SUBCASE( "Half move clock above the limit" )
    {
        CHECK_THROWS_AS(
            (void)Game::createGameFromFen ("4r3/8/8/8/8/8/k7/4K2R w - - 10001 5"),
            FenParserError
        );
        CHECK_THROWS_AS(
            (void)Game::createGameFromFen ("4r3/8/8/8/8/8/k7/4K2R w - - 2147483647 5"),
            FenParserError
        );
    }

    SUBCASE( "Full move number above the limit" )
    {
        CHECK_THROWS_AS(
            (void)Game::createGameFromFen ("4r3/8/8/8/8/8/k7/4K2R w - - 10 10001"),
            FenParserError
        );
    }

    SUBCASE( "The limits themselves are accepted" )
    {
        Game game = Game::createGameFromFen ("4r3/8/8/8/8/8/k7/4K2R w - - 10000 10000");

        CHECK( game.getBoard().getHalfMoveClock() == Max_Half_Move_Clock );
        CHECK( game.getBoard().getFullMoveClock() == Max_Full_Move_Number );
    }
}

TEST_CASE( "FEN parser rejects malformed piece and castling fields" )
{
    SUBCASE( "More than eight ranks" )
    {
        CHECK_THROWS_AS(
            (void)Game::createGameFromFen ("4k3/8/8/8/8/8/8/4K3/8 w - - 0 1"),
            FenParserError
        );
    }

    SUBCASE( "Unknown castling letter" )
    {
        CHECK_THROWS_AS(
            (void)Game::createGameFromFen ("4k3/8/8/8/8/8/8/4K2R w Kx - 0 1"),
            FenParserError
        );
    }

    SUBCASE( "Non-letter characters in the castling field" )
    {
        for (const char* castling : { "K1", "K$", "K-", "-K", "--" })
        {
            CAPTURE( castling );
            auto fen = std::string { "4k3/8/8/8/8/8/8/4K2R w " } + castling + " - 0 1";
            CHECK_THROWS_AS( (void)Game::createGameFromFen (fen), FenParserError );
        }
    }

    SUBCASE( "Valid castling letters still parse" )
    {
        CHECK_NOTHROW( (void)Game::createGameFromFen ("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1") );
        CHECK_NOTHROW( (void)Game::createGameFromFen ("4k3/8/8/8/8/8/8/4K3 w - - 0 1") );
    }
}
