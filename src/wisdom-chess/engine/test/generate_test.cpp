#include <algorithm>

#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/generate.hpp"
#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/threats.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"
#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/material.hpp"

#include "wisdom-chess-tests.hpp"

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

    auto
    enPassantMovesAmongLegalMoves (const Board& board)
        -> MoveList
    {
        MoveList result;

        for (auto move : generateLegalMoves (board, board.getCurrentTurn()))
        {
            if (move.isEnPassant())
                result.append (move);
        }

        return result;
    }

    // Returns the number of positions in the tree with an en passant capture.
    auto
    checkEnPassantMovesInTree (const Board& board, int depth)
        -> int
    {
        auto who = board.getCurrentTurn();
        auto en_passant_moves = generateLegalEnPassantMoves (board);

        INFO( board.toFenString (who) );
        CHECK( en_passant_moves == enPassantMovesAmongLegalMoves (board) );
        CHECK( board.getLegalEnPassantTarget().has_value() == !en_passant_moves.isEmpty() );

        int found = en_passant_moves.isEmpty() ? 0 : 1;
        if (depth <= 0)
            return found;

        for (auto move : generateLegalMoves (board, who))
            found += checkEnPassantMovesInTree (board.withMove (who, move), depth - 1);
        return found;
    }

    auto
    materialGain (const Board& board, Move move)
        -> int
    {
        if (move.isEnPassant())
            return 0;
        return Material::weight (pieceType (board.pieceAt (move.getDst())))
            - Material::weight (pieceType (board.pieceAt (move.getSrc())));
    }

    // Whether the ordering must put `a` before `b`, by its rules stated one
    // pair at a time. Pairs it does not rank either way may come in any order.
    auto
    mustComeBefore (const Board& board, Color who, const MoveOrdering& ordering, Move a, Move b)
        -> bool
    {
        if (a == ordering.priority_move || b == ordering.priority_move)
            return a == ordering.priority_move && b != ordering.priority_move;

        if (a.isAnyCapturing() != b.isAnyCapturing())
            return a.isAnyCapturing();

        if (a.isAnyCapturing() && materialGain (board, a) != materialGain (board, b))
            return materialGain (board, a) > materialGain (board, b);

        if (a.isPromoting() != b.isPromoting())
            return a.isPromoting();

        if (a.isPromoting())
            return Material::weight (a.getPromotedPiece()) > Material::weight (b.getPromotedPiece());

        if (a.isAnyCapturing())
            return a.getSrc().index() * Num_Squares + a.getDst().index()
                < b.getSrc().index() * Num_Squares + b.getDst().index();

        auto killer_slot = [&ordering] (Move move) {
            auto found = std::find (ordering.killers.begin(), ordering.killers.end(), move);
            return found - ordering.killers.begin();
        };
        if (killer_slot (a) != killer_slot (b))
            return killer_slot (a) < killer_slot (b);

        auto a_score = ordering.history ? ordering.history.value()->getScore (who, a) : 0;
        auto b_score = ordering.history ? ordering.history.value()->getScore (who, b) : 0;
        if (a_score != b_score)
            return a_score > b_score;

        return a.getSrc().index() * Num_Squares + a.getDst().index()
            < b.getSrc().index() * Num_Squares + b.getDst().index();
    }

    void
    checkOrderingInTree (const Board& board, int depth)
    {
        auto who = board.getCurrentTurn();
        auto plain = generateAllPotentialMoves (board, who);

        // Name some quiet moves as the priority move and killers, and give
        // others history scores, so every rule has something to rank.
        vector<Move> quiet;
        for (auto move : plain)
        {
            if (!move.isAnyCapturing() && !move.isPromoting())
                quiet.push_back (move);
        }

        auto history = make_unique<CutoffHistory>();
        MoveOrdering ordering { nullopt, {}, history.get() };

        if (quiet.size() >= 3)
        {
            ordering.priority_move = quiet[quiet.size() - 1];
            ordering.killers = { quiet[quiet.size() - 2], quiet[0] };
        }
        for (size_t i = 0; i < quiet.size(); i += 2)
            history->store (who, quiet[i], narrow_debug<int> (1 + i % 3));

        auto ordered = generateAllPotentialMoves (board, who, ordering);

        INFO( board.toFenString (who) );
        INFO( ordered );
        CHECK( sortedMoves (ordered) == sortedMoves (plain) );

        for (auto first = ordered.begin(); first != ordered.end(); first++)
        {
            for (auto later = first + 1; later != ordered.end(); later++)
                CHECK( !mustComeBefore (board, who, ordering, *later, *first) );
        }

        if (depth <= 0)
            return;

        for (auto move : generateLegalMoves (board, who))
            checkOrderingInTree (board.withMove (who, move), depth - 1);
    }

    void
    checkHasLegalMoveInTree (const Board& board, int depth)
    {
        auto who = board.getCurrentTurn();
        auto legal_moves = generateLegalMoves (board, who);
        bool in_check = isKingThreatened (board, who, board.getKingPosition (who));

        INFO( board.toFenString (who) );
        CHECK( hasLegalMove (board) == !legal_moves.isEmpty() );
        CHECK( hasLegalMove (board, in_check) == !legal_moves.isEmpty() );

        if (depth <= 0)
            return;

        for (auto move : legal_moves)
            checkHasLegalMoveInTree (board.withMove (who, move), depth - 1);
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

TEST_CASE( "generateAllPotentialMoves with a MoveOrdering" )
{
    SUBCASE( "The priority move is first, then the killers in slot order" )
    {
        Board board;
        Move priority = moveParse ("d2 d4", Color::White);
        Move first_killer = moveParse ("g1 f3", Color::White);
        Move second_killer = moveParse ("e2 e4", Color::White);

        auto move_list = generateAllPotentialMoves (
            board, Color::White,
            MoveOrdering { priority, { first_killer, second_killer }, nullptr }
        );

        INFO( move_list );
        REQUIRE( move_list.size() == 20 );
        CHECK( *move_list.begin() == priority );
        CHECK( *(move_list.begin() + 1) == first_killer );
        CHECK( *(move_list.begin() + 2) == second_killer );
        CHECK( sortedMoves (move_list)
               == sortedMoves (generateAllPotentialMoves (board, Color::White)) );
    }

    SUBCASE( "Captures come before a killer" )
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
        Move killer = moveParse ("e1 d1", Color::Black);

        auto move_list = generateAllPotentialMoves (
            board, Color::Black, MoveOrdering { nullopt, { killer, nullopt }, nullptr }
        );

        INFO( move_list );
        REQUIRE( move_list.size() >= 4 );
        CHECK( *move_list.begin() == moveParse ("c4xd3", Color::Black) );
        CHECK( *(move_list.begin() + 1) == moveParse ("c4xb3", Color::Black) );
        CHECK( *(move_list.begin() + 2) == moveParse ("e4xd3", Color::Black) );
        CHECK( *(move_list.begin() + 3) == killer );
    }

    SUBCASE( "Promotions come before a killer" )
    {
        BoardBuilder builder;

        builder.addPiece ("a7", Color::White, Piece::Pawn);
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.addPiece ("e8", Color::Black, Piece::King);

        auto board = Board { builder };
        Move killer = moveParse ("e1 d1", Color::White);

        auto move_list = generateAllPotentialMoves (
            board, Color::White, MoveOrdering { nullopt, { killer, nullopt }, nullptr }
        );

        INFO( move_list );
        REQUIRE( move_list.size() >= 5 );
        CHECK( *move_list.begin() == moveParse ("a7 a8 (Q)", Color::White) );
        CHECK( (move_list.begin() + 3)->isPromoting() );
        CHECK( *(move_list.begin() + 4) == killer );
    }

    SUBCASE( "A killer that is not in the list changes nothing" )
    {
        Board board;
        Move absent = moveParse ("a1 h8", Color::White);

        auto plain = generateAllPotentialMoves (board, Color::White);
        auto ordered = generateAllPotentialMoves (
            board, Color::White, MoveOrdering { nullopt, { absent, absent }, nullptr }
        );

        CHECK( std::vector<Move> (ordered.begin(), ordered.end())
               == std::vector<Move> (plain.begin(), plain.end()) );
    }

    SUBCASE( "Quiet moves that are not killers go by their history score" )
    {
        Board board;
        auto history = make_unique<CutoffHistory>();
        Move killer = moveParse ("d2 d4", Color::White);
        Move often = moveParse ("g1 f3", Color::White);
        Move seldom = moveParse ("h2 h3", Color::White);

        history->store (Color::White, seldom, 2);
        history->store (Color::White, often, 3);
        history->store (Color::White, killer, 1);

        auto move_list = generateAllPotentialMoves (
            board, Color::White, MoveOrdering { nullopt, { killer, nullopt }, history.get() }
        );

        INFO( move_list );
        REQUIRE( move_list.size() == 20 );
        CHECK( *move_list.begin() == killer );
        CHECK( *(move_list.begin() + 1) == often );
        CHECK( *(move_list.begin() + 2) == seldom );
    }

    SUBCASE( "Moves with the same history score keep the square order" )
    {
        Board board;
        auto history = make_unique<CutoffHistory>();
        Move scored = moveParse ("h2 h3", Color::White);

        history->store (Color::White, scored, 2);

        auto plain = generateAllPotentialMoves (board, Color::White);
        auto ordered = generateAllPotentialMoves (
            board, Color::White, MoveOrdering { nullopt, {}, history.get() }
        );

        std::vector<Move> expected { scored };
        for (auto move : plain)
        {
            if (move != scored)
                expected.push_back (move);
        }

        CHECK( std::vector<Move> (ordered.begin(), ordered.end()) == expected );
    }

    SUBCASE( "The other side's history does not reorder the moves" )
    {
        Board board;
        auto history = make_unique<CutoffHistory>();

        history->store (Color::Black, moveParse ("h2 h3", Color::White), 5);

        auto plain = generateAllPotentialMoves (board, Color::White);
        auto ordered = generateAllPotentialMoves (
            board, Color::White, MoveOrdering { nullopt, {}, history.get() }
        );

        CHECK( std::vector<Move> (ordered.begin(), ordered.end())
               == std::vector<Move> (plain.begin(), plain.end()) );
    }

    SUBCASE( "Every pair in a tree of positions follows the ordering rules" )
    {
        const czstring fens[] = {
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
            "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
            "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1",
        };

        for (auto fen : fens)
        {
            auto game = Game::createGameFromFen (fen);
            checkOrderingInTree (game.getBoard(), 1);
        }
    }

    SUBCASE( "An empty ordering gives the plain order" )
    {
        Board board;

        auto plain = generateAllPotentialMoves (board, Color::White);
        auto ordered = generateAllPotentialMoves (board, Color::White, MoveOrdering {});

        CHECK( std::vector<Move> (ordered.begin(), ordered.end())
               == std::vector<Move> (plain.begin(), plain.end()) );
    }
}

TEST_CASE( "hasLegalMove" )
{
    auto board_from_fen = [] (czstring fen_text) {
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
        auto board = board_from_fen (
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
        auto board = board_from_fen ("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1");

        CHECK( !hasLegalMove (board) );
        CHECK( isStalemated (board) );
        CHECK( !isCheckmated (board) );
    }

    SUBCASE( "A player in check with an evasion has a legal move" )
    {
        auto board = board_from_fen ("4k3/8/8/8/8/8/4r3/4K3 w - - 0 1");

        CHECK( hasLegalMove (board) );
        CHECK( !isCheckmated (board) );
        CHECK( !isStalemated (board) );
    }

    SUBCASE( "A player in check whose only evasion is a block has a legal move" )
    {
        auto board = board_from_fen ("6rk/6pp/8/8/8/8/1B6/K6R b - - 0 1");
        auto with_check = board.withMove (Color::Black, moveParse ("g7 g6", Color::Black));
        with_check = with_check.withMove (Color::White, moveParse ("b2 f6", Color::White));

        auto legal_moves = generateLegalMoves (with_check, Color::Black);

        CHECK( legal_moves.size() == 1 );
        CHECK( hasLegalMove (with_check) );
        CHECK( hasLegalMove (with_check, true) );
    }

    SUBCASE( "A pinned piece is the only one that could move" )
    {
        auto board = board_from_fen ("8/8/8/8/2q5/2b5/1R6/K1k5 w - - 0 1");

        CHECK( !hasLegalMove (board) );
        CHECK( !hasLegalMove (board, false) );
        CHECK( isStalemated (board) );
    }

    SUBCASE( "A blocked pawn away from the king's lines is no legal move" )
    {
        auto board = board_from_fen ("k7/2Q5/8/8/7p/7P/8/6K1 b - - 0 1");

        CHECK( !hasLegalMove (board) );
        CHECK( !hasLegalMove (board, false) );
        CHECK( isStalemated (board) );
    }

    SUBCASE( "A capture by a pawn away from the king's lines is a legal move" )
    {
        auto board = board_from_fen ("k7/2Q5/8/8/7p/6NP/8/6K1 b - - 0 1");

        CHECK( generateLegalMoves (board, Color::Black).size() == 1 );
        CHECK( hasLegalMove (board) );
        CHECK( hasLegalMove (board, false) );
    }

    SUBCASE( "An en passant capture that exposes the king is no legal move" )
    {
        // Taking the pawn opens the diagonal from h1 to the king.
        auto board = board_from_fen ("K7/7r/4p3/3pP3/8/8/8/1r4kb w - d6 0 1");

        CHECK( generateLegalMoves (board, Color::White).isEmpty() );
        CHECK( !hasLegalMove (board) );
        CHECK( !hasLegalMove (board, false) );
    }

    SUBCASE( "Agrees with generateLegalMoves" )
    {
        czstring fens[] = {
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
            auto board = board_from_fen (fen_text);
            auto who = board.getCurrentTurn();

            INFO( fen_text );
            CHECK( hasLegalMove (board)
                   == !generateLegalMoves (board, who).isEmpty() );
        }
    }

    SUBCASE( "Agrees with generateLegalMoves in every position of a tree" )
    {
        czstring fens[] = {
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
            "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
            "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
            "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
            "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 b - - 0 10",
            // Endgames with a stalemate inside the tree.
            "8/8/8/8/4k3/8/4p3/4K3 b - - 0 1",
            "8/8/8/4b3/2q5/8/1R6/K1k5 b - - 0 1",
            "7k/8/6K1/8/8/8/8/5Q2 w - - 0 1",
        };

        for (auto fen_text : fens)
            checkHasLegalMoveInTree (board_from_fen (fen_text), 2);
    }
}

TEST_CASE( "generateCaptures" )
{
    auto board_from_fen = [] (czstring fen_text) {
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
        auto board = board_from_fen (
            "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3"
        );
        auto captures = generateCaptures (board, Color::White);

        CHECK( captures.size() == 1 );
        CHECK( containsMove (captures, moveParse ("e5f6 ep", Color::White)) );
    }

    SUBCASE( "Only promotions to a queen are included" )
    {
        auto board = board_from_fen ("1n6/P7/8/8/8/8/8/k6K w - - 0 1");
        auto captures = generateCaptures (board, Color::White);

        CHECK( captures.size() == 2 );
        CHECK( containsMove (captures, moveParse ("a7a8 (Q)", Color::White)) );
        CHECK( containsMove (captures, moveParse ("a7xb8 (Q)", Color::White)) );
    }

    SUBCASE( "Matches the captures and queen promotions of all moves" )
    {
        czstring fens[] = {
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
            "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
            "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
            "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
            "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
            "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3",
            "nr3kr1/Pp1p1P1p/1P6/nPp4p/B7/B2N4/P2P2P1/5RK1 w - - 0 1",
        };

        for (auto fen_text : fens)
        {
            auto board = board_from_fen (fen_text);

            for (auto who : { Color::White, Color::Black })
            {
                INFO( fen_text, " ", asString (who) );

                MoveList expected;
                for (auto move : generateAllPotentialMoves (board, who))
                {
                    if (move.isPromoting()
                            ? move.getPromotedPiece() == Piece::Queen
                            : move.isAnyCapturing())
                    {
                        expected.append (move);
                    }
                }

                CHECK( sortedMoves (generateCaptures (board, who)) == sortedMoves (expected) );
            }
        }
    }
}

TEST_CASE( "generateLegalEnPassantMoves" )
{
    auto board_from_fen = [] (czstring fen_text) {
        FenParser fen { fen_text };
        return fen.buildBoard();
    };

    SUBCASE( "A board without a target has none" )
    {
        Board board;

        CHECK( generateLegalEnPassantMoves (board).isEmpty() );
    }

    SUBCASE( "Both adjacent pawns can capture" )
    {
        auto board = board_from_fen ("4k3/8/8/8/2pPp3/8/8/4K3 b - d3 0 1");

        MoveList expected { Color::Black, { "c4d3 ep" } };
        expected.append (moveParse ("e4d3 ep", Color::Black));

        CHECK( generateLegalEnPassantMoves (board) == expected );
    }

    SUBCASE( "A pawn pinned on its file is left out while the other captures" )
    {
        auto board = board_from_fen ("4k3/8/8/8/2pPp3/8/8/4R2K b - d3 0 1");

        MoveList expected { Color::Black, { "c4d3 ep" } };

        CHECK( generateLegalEnPassantMoves (board) == expected );
    }

    SUBCASE( "A pawn pinned on a diagonal cannot capture" )
    {
        auto board = board_from_fen ("8/1k6/8/8/3Pp3/8/8/K6B b - d3 0 1");

        CHECK( generateLegalEnPassantMoves (board).isEmpty() );
    }

    SUBCASE( "The taken pawn cannot uncover a check on its diagonal" )
    {
        auto board = board_from_fen ("8/k7/8/8/3Pp3/8/8/K5B1 b - d3 0 1");

        CHECK( generateLegalEnPassantMoves (board).isEmpty() );
    }

    SUBCASE( "Both pawns cannot leave a rank that shields the king" )
    {
        auto board = board_from_fen ("8/8/8/8/R2Pp2k/8/8/4K3 b - d3 0 1");

        CHECK( generateLegalEnPassantMoves (board).isEmpty() );
    }

    SUBCASE( "Capturing the pawn that gives check is legal" )
    {
        auto board = board_from_fen ("8/8/8/4k3/3Pp3/8/8/K7 b - d3 0 1");

        MoveList expected { Color::Black, { "e4d3 ep" } };

        CHECK( generateLegalEnPassantMoves (board) == expected );
    }

    SUBCASE( "A capture that leaves the king in check is not legal" )
    {
        auto board = board_from_fen ("8/8/7k/8/3Pp3/8/8/K1B5 b - d3 0 1");

        CHECK( generateLegalEnPassantMoves (board).isEmpty() );
    }

    SUBCASE( "A double push on an edge file can be captured" )
    {
        auto black_captures = board_from_fen ("4k3/8/8/8/Pp6/8/8/4K3 b - a3 0 1");
        auto white_captures = board_from_fen ("4k3/8/8/6Pp/8/8/8/4K3 w - h6 0 1");

        MoveList black_expected { Color::Black, { "b4a3 ep" } };
        MoveList white_expected { Color::White, { "g5h6 ep" } };

        CHECK( generateLegalEnPassantMoves (black_captures) == black_expected );
        CHECK( generateLegalEnPassantMoves (white_captures) == white_expected );
    }

    SUBCASE( "A target without a pawn to take has none" )
    {
        // FenParser rejects these targets, so set them on the builder.
        BoardBuilder builder;
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.addPiece ("e8", Color::Black, Piece::King);
        builder.addPiece ("e4", Color::Black, Piece::Pawn);
        builder.setEnPassantTarget (Color::White, "d3");
        builder.setCurrentTurn (Color::Black);
        auto missing_pawn = Board { builder };

        builder.addPiece ("d4", Color::White, Piece::Pawn);
        builder.addPiece ("d3", Color::White, Piece::Knight);
        auto occupied_target = Board { builder };

        CHECK( generateLegalEnPassantMoves (missing_pawn).isEmpty() );
        CHECK( generateLegalEnPassantMoves (occupied_target).isEmpty() );
    }

    SUBCASE( "A target of the player to move has none" )
    {
        // If White's own target counted for White, c5 would capture the pawn
        // on d5 en passant.
        auto board = board_from_fen ("4k3/8/8/2PP4/3Pp3/8/8/4K3 b - d3 0 1");
        auto white_to_move = board.withCurrentTurn (Color::White);

        CHECK( !generateLegalEnPassantMoves (board).isEmpty() );
        CHECK( generateLegalEnPassantMoves (white_to_move).isEmpty() );
    }

    SUBCASE( "Agrees with generateLegalMoves" )
    {
        // Each tree is as deep as it needs to be to reach en passant
        // targets. The slow suite checks deeper trees.
        struct Position
        {
            czstring fen;
            int depth;
        };

        const Position positions[] = {
            { "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 2 },
            { "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 3 },
            { "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 b - - 0 1", 3 },
            { "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 2 },
            { "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3", 3 },
            { "4k3/pppppppp/8/PPPPPPPP/pppppppp/8/PPPPPPPP/4K3 w - - 0 1", 3 },
        };

        // generateLegalMoves() offers en passant only where this generator
        // found a capture, so a generator that finds none agrees with it.
        // Each tree has to reach a capture.
        for (const auto& position : positions)
        {
            INFO( position.fen );
            CHECK( checkEnPassantMovesInTree (board_from_fen (position.fen), position.depth) > 0 );
        }
    }
}
