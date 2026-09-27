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

TEST_CASE( "FEN parser sets the side to move on a built board" )
{
    SUBCASE( "White to move" )
    {
        FenParser parser { "4k3/8/8/8/8/8/8/4K3 w - - 0 1" };
        CHECK( parser.buildBoard().getCurrentTurn() == Color::White );
    }

    SUBCASE( "Black to move" )
    {
        FenParser parser { "4k3/8/8/8/8/8/8/4K3 b - - 0 1" };
        CHECK( parser.buildBoard().getCurrentTurn() == Color::Black );
    }
}

TEST_CASE( "FEN notation for castling" )
{
    Game game = Game::createGameFromFen ("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");

    REQUIRE( game.getBoard().getCastlingEligibility (Color::White) == CastlingEligibility::Both_Sides );
    REQUIRE( game.getBoard().getCastlingEligibility (Color::Black) == CastlingEligibility::Both_Sides );

    game = Game::createGameFromFen ("r3k2r/8/8/8/8/8/8/R3K2R w KQq - 0 1");

    REQUIRE( game.getBoard().getCastlingEligibility (Color::White) == CastlingEligibility::Both_Sides );
    REQUIRE( game.getBoard().getCastlingEligibility (Color::Black) == CastlingRights::Queenside );

    game = Game::createGameFromFen ("r3k2r/8/8/8/8/8/8/R3K2R w KQq - 0 1");

    REQUIRE( game.getBoard().getCastlingEligibility (Color::White) == CastlingEligibility::Both_Sides );
    REQUIRE( game.getBoard().getCastlingEligibility (Color::Black) == CastlingRights::Queenside );

    game = Game::createGameFromFen ("r3k2r/8/8/8/8/8/8/R3K2R w - - 0 1");

    REQUIRE( game.getBoard().getCastlingEligibility (Color::White) == CastlingEligibility::Neither_Side );
    REQUIRE( game.getBoard().getCastlingEligibility (Color::Black) == CastlingEligibility::Neither_Side );
}

TEST_CASE( "FEN parser rejects castling rights without the rook present" )
{
    czstring missing_rook = "Castling rights require a rook on its home square!";

    SUBCASE( "Queenside right claimed with no rook on the queenside square" )
    {
        CHECK_THROWS_WITH_AS(
            (void)Game::createGameFromFen ("4k3/8/8/8/8/8/8/4K2R w Q - 0 1"),
            missing_rook,
            FenParserError
        );
    }

    SUBCASE( "Kingside right claimed with no rook on the kingside square" )
    {
        CHECK_THROWS_WITH_AS(
            (void)Game::createGameFromFen ("r3k3/8/8/8/8/8/8/R3K3 w Kk - 0 1"),
            missing_rook,
            FenParserError
        );
    }

    SUBCASE( "Black's rights are checked when White claims none" )
    {
        CHECK_THROWS_WITH_AS(
            (void)Game::createGameFromFen ("r3k3/8/8/8/8/8/8/R3K2R w k - 0 1"),
            missing_rook,
            FenParserError
        );
        CHECK_THROWS_WITH_AS(
            (void)Game::createGameFromFen ("4k2r/8/8/8/8/8/8/R3K2R w q - 0 1"),
            missing_rook,
            FenParserError
        );
    }

    SUBCASE( "A rook of the other color does not count" )
    {
        CHECK_THROWS_WITH_AS(
            (void)Game::createGameFromFen ("r3k2r/8/8/8/8/8/8/r3K2R w Q - 0 1"),
            missing_rook,
            FenParserError
        );
    }

    SUBCASE( "Another piece on the rook's square does not count" )
    {
        CHECK_THROWS_WITH_AS(
            (void)Game::createGameFromFen ("r3k2r/8/8/8/8/8/8/R3K2N w K - 0 1"),
            missing_rook,
            FenParserError
        );
    }
}

TEST_CASE( "FEN parser rejects castling rights without the king on its home square" )
{
    czstring missing_king = "Castling rights require the king on its home square!";

    SUBCASE( "King on its home column but another row" )
    {
        CHECK_THROWS_WITH_AS(
            (void)Game::createGameFromFen ("4k3/8/8/3pP3/8/4K3/8/7R w K d6 0 1"),
            missing_king,
            FenParserError
        );
    }

    SUBCASE( "King on its home row but another column" )
    {
        CHECK_THROWS_WITH_AS(
            (void)Game::createGameFromFen ("r2k3r/8/8/8/8/8/8/R3K2R w q - 0 1"),
            missing_king,
            FenParserError
        );
    }

    SUBCASE( "The other color's king on the home square does not count" )
    {
        CHECK_THROWS_WITH_AS(
            (void)Game::createGameFromFen ("4K2r/8/8/8/8/8/8/4k3 w k - 0 1"),
            missing_king,
            FenParserError
        );
    }
}

TEST_CASE( "FEN notation for en passant" )
{
    Game game = Game::createGameFromFen ("4r3/8/8/4p3/8/8/k7/4K2R w - e6 0 1");
    auto &board = game.getBoard();

    REQUIRE( !board.isEnPassantVulnerable (Color::White) );
    REQUIRE( board.isEnPassantVulnerable (Color::Black) );

    auto black_target = board.getEnPassantTarget();
    REQUIRE( black_target.has_value() );
    CHECK( black_target->vulnerable_color == Color::Black );
    CHECK( black_target->coord == coordParse ("e6") );
}

TEST_CASE( "FEN records a double pawn push without an adjacent enemy pawn" )
{
    auto game = Game::createGameFromFen (
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    );
    Board board { game.getBoard() };

    board = board.withMove (Color::White, moveParse ("e2 e4"));
    CHECK( board.toFenString (Color::Black)
           == "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1" );

    board = board.withMove (Color::Black, moveParse ("e7 e5"));
    CHECK( board.toFenString (Color::White)
           == "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2" );
}

TEST_CASE( "FEN notation with an invalid en passant square" )
{
    SUBCASE( "Square outside the board" )
    {
        CHECK_THROWS_WITH_AS(
            (void)Game::createGameFromFen ("4r3/8/8/8/8/8/k7/4K2R w - z9 0 1"),
            "Error parsing en passant coordinate: Invalid coordinate!",
            FenParserError
        );
    }

    SUBCASE( "Square with a missing rank" )
    {
        CHECK_THROWS_WITH_AS(
            (void)Game::createGameFromFen ("4r3/8/8/8/8/8/k7/4K2R w - e 0 1"),
            "Error parsing en passant coordinate: Invalid coordinate!",
            FenParserError
        );
    }
}

TEST_CASE( "FEN parser rejects an en passant target no double pawn push could have left" )
{
    czstring missing_pawn = "En passant target requires a pawn that just moved two squares!";
    czstring occupied = "En passant target requires empty squares behind the pawn!";
    czstring wrong_rank = "En passant target is on the wrong rank for the side to move!";

    SUBCASE( "No pawn in front of the target" )
    {
        CHECK_THROWS_WITH_AS(
            (void)Game::createGameFromFen ("4k3/8/8/3P4/8/8/8/4K3 w - e6 0 1"),
            missing_pawn,
            FenParserError
        );
    }

    SUBCASE( "A pawn of the side to move in front of the target" )
    {
        CHECK_THROWS_WITH_AS(
            (void)Game::createGameFromFen ("4k3/8/8/3PP3/8/8/8/4K3 w - e6 0 1"),
            missing_pawn,
            FenParserError
        );
    }

    SUBCASE( "Another piece in front of the target" )
    {
        CHECK_THROWS_WITH_AS(
            (void)Game::createGameFromFen ("4k3/8/8/3Pn3/8/8/8/4K3 w - e6 0 1"),
            missing_pawn,
            FenParserError
        );
    }

    SUBCASE( "Target square occupied" )
    {
        CHECK_THROWS_WITH_AS(
            (void)Game::createGameFromFen ("4k3/8/4n3/3Pp3/8/8/8/4K3 w - e6 0 1"),
            occupied,
            FenParserError
        );
    }

    SUBCASE( "Pawn's starting square occupied" )
    {
        CHECK_THROWS_WITH_AS(
            (void)Game::createGameFromFen ("4k3/4p3/8/3Pp3/8/8/8/4K3 w - e6 0 1"),
            occupied,
            FenParserError
        );
    }

    SUBCASE( "Target on the rank of the side to move" )
    {
        CHECK_THROWS_WITH_AS(
            (void)Game::createGameFromFen ("4k3/8/8/8/3pP3/8/8/4K3 w - e3 0 1"),
            wrong_rank,
            FenParserError
        );
        CHECK_THROWS_WITH_AS(
            (void)Game::createGameFromFen ("4k3/8/8/3pP3/8/8/8/4K3 b - d6 0 1"),
            wrong_rank,
            FenParserError
        );
    }

    SUBCASE( "Target on a rank no double push crosses" )
    {
        CHECK_THROWS_WITH_AS(
            (void)Game::createGameFromFen ("4k3/8/8/3pP3/8/8/8/4K3 w - d5 0 1"),
            wrong_rank,
            FenParserError
        );
    }

    SUBCASE( "A target left by a double push is accepted for either side" )
    {
        CHECK_NOTHROW( (void)Game::createGameFromFen ("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1") );
        CHECK_NOTHROW( (void)Game::createGameFromFen ("4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 1") );
    }
}

TEST_CASE( "FEN full move number starts at 1" )
{
    SUBCASE( "A default board" )
    {
        Board board;

        CHECK( board.getFullMoveClock() == 1 );
        CHECK( board.toFenString (Color::White)
               == "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" );

        board = board.withMove (Color::White, moveParse ("e2 e4"));
        CHECK( board.toFenString (Color::Black)
               == "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1" );

        board = board.withMove (Color::Black, moveParse ("e7 e5"));
        CHECK( board.toFenString (Color::White)
               == "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2" );
    }

    SUBCASE( "A standard game" )
    {
        auto game = Game::createStandardGame();

        CHECK( game.getBoard().getFullMoveClock() == 1 );
    }

    SUBCASE( "A board from a builder that was not given one" )
    {
        BoardBuilder builder;
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.addPiece ("e8", Color::Black, Piece::King);

        CHECK( Board { builder }.getFullMoveClock() == 1 );
    }
}

TEST_CASE( "Parsing half and full moves" )
{
    SUBCASE( "With castling and en passant square" )
    {
        Game game = Game::createGameFromFen ("r3k2r/8/8/4p3/8/8/8/R3K2R w Kk e6 10 5");
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

    SUBCASE( "A full move number of 0 is read as 1" )
    {
        Game game = Game::createGameFromFen ("4r3/8/8/8/8/8/k7/4K2R w - - 10 0");
        const auto& board = game.getBoard();
        CHECK( board.getFullMoveClock() == 1 );
        CHECK( board.toFenString (Color::White) == "4r3/8/8/8/8/8/k7/4K2R w - - 10 1" );
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
        for (czstring castling : { "K1", "K$", "K-", "-K", "--" })
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
