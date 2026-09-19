#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/generate.hpp"
#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"
#include "wisdom-chess/engine/game.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "generate default moves" )
{
    Board board;

    auto move_list = generateAllPotentialMoves (board, Color::White);

    std::string expected = "{ [a2 a4] [a2 a3] [b2 b4] [b2 b3] [c2 c4] [c2 c3] "
                           "[d2 d4] [d2 d3] [e2 e4] [e2 e3] [f2 f4] [f2 f3] "
                           "[g2 g4] [g2 g3] [h2 h4] [h2 h3] [b1 a3] [b1 c3] "
                           "[g1 f3] [g1 h3] }";
    REQUIRE( move_list.asString() == expected );
}

TEST_CASE( "generate en passant moves" )
{
    Board board;

    board = board.withMove (Color::White, moveParse ("e2 e4", Color::White));
    board = board.withMove (Color::Black, moveParse ("d7 d5", Color::Black));
    board = board.withMove (Color::White, moveParse ("e4 e5", Color::White));
    board = board.withMove (Color::Black, moveParse ("f7 f5", Color::Black));

    auto move_list = generateAllPotentialMoves (board, Color::White).asString();
    auto pos = move_list.find ("[e5 f6 ep]");

    INFO( move_list );
    REQUIRE( pos != std::string::npos );
}

TEST_CASE( "Generated moves are sorted by capturing difference of pieces" )
{
    BoardBuilder builder;

    builder.addPiece ("c4", Color::Black, Piece::Pawn);
    builder.addPiece ("e4", Color::Black, Piece::Queen);
    builder.addPiece ("d3", Color::White, Piece::Queen);
    builder.addPiece ("b3", Color::White, Piece::Bishop);
    builder.addPiece ("a1", Color::White, Piece::King);
    builder.addPiece ("e1", Color::Black, Piece::King);
    builder.setCurrentTurn (Color::Black);

    auto board = Board { builder };

    auto move_list = generateAllPotentialMoves (board, Color::Black);

    std::string expected = "{ [c4xd3] [c4xb3] ";
    std::string converted = move_list.asString().substr (0, expected.size());

    INFO( move_list );
    REQUIRE( expected == converted );
}

TEST_CASE( "hasLegalMove" )
{
    auto boardFromFen = [](const char* fen_text) {
        FenParser fen { fen_text };
        auto game = fen.build();
        return Board { game.getBoard() };
    };

    SUBCASE( "The starting position has legal moves" )
    {
        Board board;

        CHECK( hasLegalMove (board) );

        board = board.withMove (Color::White, moveParse ("e2 e4", Color::White));

        CHECK( hasLegalMove (board) );
    }

    SUBCASE( "A checkmated player has no legal move" )
    {
        auto board = boardFromFen (
            "rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3"
        );

        CHECK( !hasLegalMove (board) );
        CHECK( isPlayerCheckmated (board, Color::White) );
        CHECK( !isPlayerCheckmated (board, Color::Black) );
        CHECK( !isStalemated (board, Color::White) );
    }

    SUBCASE( "A stalemated player has no legal move" )
    {
        auto board = boardFromFen ("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1");

        CHECK( !hasLegalMove (board) );
        CHECK( isStalemated (board, Color::Black) );
        CHECK( !isPlayerCheckmated (board, Color::Black) );
    }

    SUBCASE( "A player in check with an evasion has a legal move" )
    {
        auto board = boardFromFen ("4k3/8/8/8/8/8/4r3/4K3 w - - 0 1");

        CHECK( hasLegalMove (board) );
        CHECK( !isPlayerCheckmated (board, Color::White) );
        CHECK( !isStalemated (board, Color::White) );
    }

    SUBCASE( "A player in check whose only evasion is a block has a legal move" )
    {
        auto board = boardFromFen ("6rk/6pp/8/8/8/8/1B6/K6R b - - 0 1");
        auto with_check = board.withMove (Color::Black, moveParse ("g7 g6", Color::Black));
        with_check = with_check.withMove (Color::White, moveParse ("b2 f6", Color::White));

        auto legal_moves = generateLegalMoves (with_check, Color::Black);

        CHECK( legal_moves.size() == 1 );
        CHECK( hasLegalMove (with_check) );
    }

    SUBCASE( "Agrees with generateLegalMoves" )
    {
        const char* fens[] = {
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
            "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
            "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
            "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R b KQkq - 0 1",
            "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 b - - 0 1",
            "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 b - - 0 10",
        };

        for (auto fen_text : fens)
        {
            auto board = boardFromFen (fen_text);
            auto who = board.getCurrentTurn();

            INFO( fen_text );
            CHECK( hasLegalMove (board)
                   == !generateLegalMoves (board, who).isEmpty() );
        }
    }
}
