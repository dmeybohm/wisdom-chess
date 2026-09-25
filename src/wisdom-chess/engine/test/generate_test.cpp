#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/generate.hpp"
#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"
#include "wisdom-chess/engine/game.hpp"

#include "wisdom-chess-tests.hpp"

#include <algorithm>

using namespace wisdom;

namespace
{
    // For comparing what was generated without depending on the order.
    auto
    sortedMoves (const MoveList& list)
        -> std::vector<Move>
    {
        std::vector<Move> result { list.begin(), list.end() };
        std::sort (
            result.begin(),
            result.end(),
            [] (Move a, Move b) { return a.toInt() < b.toInt(); }
        );
        return result;
    }

    auto
    containsMove (const MoveList& list, Move wanted)
        -> bool
    {
        return std::find (list.begin(), list.end(), wanted) != list.end();
    }
}

TEST_CASE( "generate default moves" )
{
    Board board;

    auto move_list = generateAllPotentialMoves (board, Color::White);

    MoveList expected { Color::White, {
        "a2 a4", "a2 a3", "b2 b4", "b2 b3", "c2 c4", "c2 c3", "d2 d4", "d2 d3",
        "e2 e4", "e2 e3", "f2 f4", "f2 f3", "g2 g4", "g2 g3", "h2 h4", "h2 h3",
        "b1 a3", "b1 c3", "g1 f3", "g1 h3",
    } };

    INFO( move_list );
    REQUIRE( sortedMoves (move_list) == sortedMoves (expected) );
}

TEST_CASE( "generate en passant moves" )
{
    Board board;

    board = board.withMove (Color::White, moveParse ("e2 e4", Color::White));
    board = board.withMove (Color::Black, moveParse ("d7 d5", Color::Black));
    board = board.withMove (Color::White, moveParse ("e4 e5", Color::White));
    board = board.withMove (Color::Black, moveParse ("f7 f5", Color::Black));

    auto move_list = generateAllPotentialMoves (board, Color::White);

    INFO( move_list );
    CHECK( containsMove (move_list, moveParse ("e5f6 ep", Color::White)) );

    // Only the pawn that just moved two squares can be taken that way.
    CHECK( !containsMove (move_list, moveParse ("e5 d6 ep", Color::White)) );
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

    INFO( move_list );
    REQUIRE( move_list.size() >= 2 );
    CHECK( *move_list.begin() == moveParse ("c4xd3", Color::Black) );
    CHECK( *(move_list.begin() + 1) == moveParse ("c4xb3", Color::Black) );
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
        CHECK( isCheckmated (board) );
        CHECK( !isStalemated (board) );
        CHECK( evaluate (board, Color::White, 1) == -checkmateScoreInMoves (1) );
        CHECK( evaluate (board, Color::Black, 1) == checkmateScoreInMoves (1) );
    }

    SUBCASE( "A stalemated player has no legal move" )
    {
        auto board = boardFromFen ("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1");

        CHECK( !hasLegalMove (board) );
        CHECK( isStalemated (board) );
        CHECK( !isCheckmated (board) );
    }

    SUBCASE( "A player in check with an evasion has a legal move" )
    {
        auto board = boardFromFen ("4k3/8/8/8/8/8/4r3/4K3 w - - 0 1");

        CHECK( hasLegalMove (board) );
        CHECK( !isCheckmated (board) );
        CHECK( !isStalemated (board) );
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

TEST_CASE( "generateCaptures" )
{
    auto boardFromFen = [](const char* fen_text) {
        FenParser fen { fen_text };
        auto game = fen.build();
        return Board { game.getBoard() };
    };

    SUBCASE( "The starting position has none" )
    {
        Board board;

        CHECK( generateCaptures (board, Color::White).isEmpty() );
        CHECK( generateCaptures (board, Color::Black).isEmpty() );
    }

    SUBCASE( "En passant is a capture" )
    {
        auto board = boardFromFen (
            "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3"
        );
        auto captures = generateCaptures (board, Color::White);

        CHECK( captures.size() == 1 );
        CHECK( containsMove (captures, moveParse ("e5f6 ep", Color::White)) );
    }

    SUBCASE( "Only promotions to a queen are included" )
    {
        auto board = boardFromFen ("1n6/P7/8/8/8/8/8/k6K w - - 0 1");
        auto captures = generateCaptures (board, Color::White);

        CHECK( captures.size() == 2 );
        CHECK( containsMove (captures, moveParse ("a7a8 (Q)", Color::White)) );
        CHECK( containsMove (captures, moveParse ("a7xb8 (Q)", Color::White)) );
    }

    SUBCASE( "Matches the captures and queen promotions of all moves, in order" )
    {
        const char* fens[] = {
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
            "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
            "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
            "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
            "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
            "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3",
        };

        for (auto fen_text : fens)
        {
            auto board = boardFromFen (fen_text);

            for (auto who : { Color::White, Color::Black })
            {
                INFO( fen_text, " ", asString (who) );

                std::vector<Move> expected;
                for (auto move : generateAllPotentialMoves (board, who))
                {
                    if (move.isPromoting()
                            ? move.getPromotedPiece() == Piece::Queen
                            : move.isAnyCapturing())
                    {
                        expected.push_back (move);
                    }
                }

                auto captures = generateCaptures (board, who);
                std::vector<Move> actual { captures.begin(), captures.end() };

                CHECK( actual == expected );
            }
        }
    }
}
