#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"
#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/generate.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

namespace
{
    auto
    boardFromFen (const char* fen_text)
        -> Board
    {
        FenParser parser { fen_text };
        return parser.buildBoard();
    }

    constexpr auto Fools_Mate = "rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3";
    constexpr auto Scholars_Mate
        = "r1bqkb1r/pppp1Qpp/2n2n2/4p3/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 0 4";
    constexpr auto White_Stalemated = "7K/5q2/6k1/8/8/8/8/8 w - - 0 1";
    constexpr auto Black_Stalemated = "7k/5Q2/6K1/8/8/8/8/8 b - - 0 1";
}

// Only the player to move can be checkmated or stalemated, so both tests
// read that player from the board.
TEST_CASE( "Checkmate detection" )
{
    SUBCASE( "The starting position is not checkmate" )
    {
        Board board;
        CHECK( !isCheckmated (board) );

        board = board.withMove (Color::White, moveParse ("e2 e4", Color::White));
        CHECK( !isCheckmated (board) );
    }

    SUBCASE( "White is checkmated" )
    {
        CHECK( isCheckmated (boardFromFen (Fools_Mate)) );
    }

    SUBCASE( "Black is checkmated" )
    {
        CHECK( isCheckmated (boardFromFen (Scholars_Mate)) );
    }

    SUBCASE( "A check that can be answered by capturing the checker is not checkmate" )
    {
        // The same queen as in scholar's mate, but without the bishop defending it.
        auto board = boardFromFen (
            "r1bqkb1r/pppp1Qpp/2n2n2/4p3/4P3/8/PPPP1PPP/RNB1KBNR b KQkq - 0 4"
        );

        CHECK( !isCheckmated (board) );
    }

    SUBCASE( "A check that can only be blocked is not checkmate" )
    {
        // The rook can block on g7.
        auto board = boardFromFen ("6rk/7p/8/8/8/8/1B6/K7 b - - 0 1");
        CHECK( !isCheckmated (board) );

        // A light-squared bishop in its place cannot.
        auto without_blocker = boardFromFen ("6bk/7p/8/8/8/8/1B6/K7 b - - 0 1");
        CHECK( isCheckmated (without_blocker) );
    }

    SUBCASE( "Stalemate is not checkmate" )
    {
        CHECK( !isCheckmated (boardFromFen (White_Stalemated)) );
        CHECK( !isCheckmated (boardFromFen (Black_Stalemated)) );
    }
}

TEST_CASE( "Stalemate detection" )
{
    SUBCASE( "White is stalemated" )
    {
        CHECK( isStalemated (boardFromFen (White_Stalemated)) );
    }

    SUBCASE( "Black is stalemated" )
    {
        CHECK( isStalemated (boardFromFen (Black_Stalemated)) );
    }

    SUBCASE( "Checkmate is not stalemate" )
    {
        CHECK( !isStalemated (boardFromFen (Fools_Mate)) );
        CHECK( !isStalemated (boardFromFen (Scholars_Mate)) );
    }

    SUBCASE( "The starting position is not stalemate" )
    {
        Board board;
        CHECK( !isStalemated (board) );

        board = board.withMove (Color::White, moveParse ("e2 e4", Color::White));
        CHECK( !isStalemated (board) );
    }

    SUBCASE( "A spare pawn move prevents stalemate" )
    {
        auto board = boardFromFen ("7k/5Q2/6K1/8/8/p7/P7/8 b - - 0 1");
        CHECK( isStalemated (board) );

        auto with_spare_move = boardFromFen ("7k/5Q2/6K1/8/p7/8/P7/8 b - - 0 1");
        CHECK( !isStalemated (with_spare_move) );
    }

    SUBCASE( "The same position is stalemate only for the player it traps" )
    {
        auto trapped_to_move = boardFromFen ("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1");
        auto other_to_move = boardFromFen ("7k/5Q2/6K1/8/8/8/8/8 w - - 0 1");

        CHECK( isStalemated (trapped_to_move) );
        CHECK( !isStalemated (other_to_move) );
    }
}

TEST_CASE( "isLegalPositionAfterMove" )
{
    auto is_legal = [] (const Board& board, Color who, const char* move_text)
    {
        auto move = moveParse (move_text, who);
        auto after = board.withMove (who, move);
        return isLegalPositionAfterMove (after, who, move);
    };

    SUBCASE( "A pinned piece cannot leave the pin" )
    {
        auto board = boardFromFen ("k3r3/8/8/8/8/8/4R3/4K3 w - - 0 1");

        CHECK( !is_legal (board, Color::White, "e2 d2") );
        CHECK( is_legal (board, Color::White, "e2 e5") );
        CHECK( is_legal (board, Color::White, "e2xe8") );
    }

    SUBCASE( "The king cannot move into check" )
    {
        auto board = boardFromFen ("k2r4/8/8/8/8/8/8/4K3 w - - 0 1");

        CHECK( !is_legal (board, Color::White, "e1 d1") );
        CHECK( is_legal (board, Color::White, "e1 f1") );
    }

    SUBCASE( "A check must be answered" )
    {
        auto board = boardFromFen ("k3r3/8/8/8/8/8/8/R3K3 w - - 0 1");

        CHECK( !is_legal (board, Color::White, "a1 a2") );
        CHECK( is_legal (board, Color::White, "e1 d1") );
    }

    SUBCASE( "Castling through an attacked square" )
    {
        auto board = boardFromFen ("5r1k/8/8/8/8/8/8/R3K2R w KQ - 0 1");

        CHECK( !is_legal (board, Color::White, "o-o") );
        CHECK( is_legal (board, Color::White, "o-o-o") );
    }

    SUBCASE( "Castling onto an attacked square" )
    {
        auto board = boardFromFen ("6rk/8/8/8/8/8/8/R3K2R w KQ - 0 1");

        CHECK( !is_legal (board, Color::White, "o-o") );
        CHECK( is_legal (board, Color::White, "o-o-o") );
    }

    SUBCASE( "Castling out of check" )
    {
        auto board = boardFromFen ("4r2k/8/8/8/8/8/8/R3K2R w KQ - 0 1");

        CHECK( !is_legal (board, Color::White, "o-o") );
        CHECK( !is_legal (board, Color::White, "o-o-o") );
    }

    SUBCASE( "Queenside castling through an attacked square" )
    {
        auto board = boardFromFen ("3r3k/8/8/8/8/8/8/R3K2R w KQ - 0 1");

        CHECK( !is_legal (board, Color::White, "o-o-o") );
        CHECK( is_legal (board, Color::White, "o-o") );
    }

    SUBCASE( "An attack on the rook or on b1 does not prevent castling" )
    {
        auto attacked_rooks = boardFromFen ("r3k2r/8/8/8/8/8/8/R3K2R w KQ - 0 1");
        auto attacked_b1 = boardFromFen ("1r5k/8/8/8/8/8/8/R3K2R w KQ - 0 1");

        CHECK( is_legal (attacked_rooks, Color::White, "o-o") );
        CHECK( is_legal (attacked_rooks, Color::White, "o-o-o") );
        CHECK( is_legal (attacked_b1, Color::White, "o-o-o") );
    }

    SUBCASE( "The same rules apply to Black" )
    {
        auto board = boardFromFen ("r3k2r/8/8/8/8/8/8/5R1K b kq - 0 1");

        CHECK( !is_legal (board, Color::Black, "o-o") );
        CHECK( is_legal (board, Color::Black, "o-o-o") );
    }
}

TEST_CASE( "Checkmate scores" )
{
    static_assert (checkmateScoreInMoves (0) == Checkmate_Score);
    static_assert (checkmateScoreInMoves (1) > checkmateScoreInMoves (2));

    static_assert (isCheckmatingOpponentScore (checkmateScoreInMoves (0)));
    static_assert (isCheckmatingOpponentScore (checkmateScoreInMoves (Default_Max_Depth)));
    static_assert (!isCheckmatingOpponentScore (Max_Non_Checkmate_Score));
    static_assert (!isCheckmatingOpponentScore (0));
    static_assert (!isCheckmatingOpponentScore (-1 * checkmateScoreInMoves (0)));
    static_assert (!isCheckmatingOpponentScore (Checkmate_Score + 1));

    SUBCASE( "A checkmated player gets the mate score for the distance" )
    {
        auto board = boardFromFen (Fools_Mate);

        CHECK( evaluate (board, Color::White, 0) == -1 * checkmateScoreInMoves (0) );
        CHECK( evaluate (board, Color::White, 3) == -1 * checkmateScoreInMoves (3) );
        CHECK( evaluate (board, Color::White, 1) < evaluate (board, Color::White, 3) );
    }

    SUBCASE( "The player giving checkmate gets the same score with the opposite sign" )
    {
        auto white_mated = boardFromFen (Fools_Mate);
        auto black_mated = boardFromFen (Scholars_Mate);

        CHECK( evaluate (white_mated, Color::Black, 3) == checkmateScoreInMoves (3) );
        CHECK( evaluate (black_mated, Color::White, 3) == checkmateScoreInMoves (3) );
        CHECK( evaluate (black_mated, Color::Black, 3) == -1 * checkmateScoreInMoves (3) );
        CHECK( isCheckmatingOpponentScore (evaluate (white_mated, Color::Black, 3)) );
    }

    SUBCASE( "Without legal moves, check decides between mate and stalemate" )
    {
        CHECK( evaluateWithoutLegalMoves (boardFromFen (Fools_Mate), Color::White, 2)
               == -1 * checkmateScoreInMoves (2) );
        CHECK( evaluateWithoutLegalMoves (boardFromFen (Scholars_Mate), Color::Black, 4)
               == -1 * checkmateScoreInMoves (4) );
        CHECK( evaluateWithoutLegalMoves (boardFromFen (White_Stalemated), Color::White, 2) == 0 );
        CHECK( evaluateWithoutLegalMoves (boardFromFen (Black_Stalemated), Color::Black, 2) == 0 );
    }
}

TEST_CASE( "Evaluating a position" )
{
    SUBCASE( "The starting position is level" )
    {
        Board board;

        CHECK( evaluate (board, Color::White, 0) == 0 );
        CHECK( evaluate (board, Color::Black, 0) == 0 );
    }

    SUBCASE( "One player's score is the negation of the other's" )
    {
        const char* fens[] = {
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
            "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
            "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
            "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
        };

        for (auto fen_text : fens)
        {
            INFO( fen_text );
            auto board = boardFromFen (fen_text);

            CHECK( evaluate (board, Color::White, 0) == -1 * evaluate (board, Color::Black, 0) );
        }
    }

    SUBCASE( "A position and its mirror image score the same for the opposite colors" )
    {
        auto board = boardFromFen (
            "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
        );
        auto mirrored = boardFromFen (
            "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1"
        );

        CHECK( evaluate (board, Color::White, 0) == evaluate (mirrored, Color::Black, 0) );
        CHECK( evaluate (board, Color::Black, 0) == evaluate (mirrored, Color::White, 0) );
    }

    SUBCASE( "Extra material is worth more than where it stands" )
    {
        auto board = boardFromFen ("4k3/8/8/8/8/8/8/Q3K3 w - - 0 1");

        CHECK( evaluate (board, Color::White, 0) > 0 );
        CHECK( evaluate (board, Color::Black, 0) < 0 );
    }

    SUBCASE( "Skipping the mate test changes only a checkmated position's score" )
    {
        auto board = boardFromFen (
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
        );
        auto mated = boardFromFen (Fools_Mate);

        CHECK( evaluateWithoutMateTest (board, Color::White) == evaluate (board, Color::White, 0) );
        CHECK( evaluateWithoutMateTest (board, Color::Black) == evaluate (board, Color::Black, 0) );
        CHECK( !isCheckmatingOpponentScore (-evaluateWithoutMateTest (mated, Color::White)) );
    }
}

TEST_CASE( "Castling penalty" )
{
    // What evaluate() adds on top of the material and position scores.
    auto castling_term = [] (const Board& board, Color who)
    {
        return evaluate (board, who, 0)
            - board.getMaterial().overallScore (who)
            - board.getPosition().overallScore (who);
    };

    // In each position Black has lost both castling rights without castling.
    constexpr int Black_Penalty = 100;

    SUBCASE( "No penalty with both rights" )
    {
        auto board = boardFromFen ("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1");

        CHECK( castling_term (board, Color::White) == Black_Penalty );
        CHECK( castling_term (board, Color::Black) == -Black_Penalty );
    }

    SUBCASE( "Half the penalty for one lost right" )
    {
        auto kingside_only = boardFromFen ("4k3/8/8/8/8/8/8/R3K2R w K - 0 1");
        auto queenside_only = boardFromFen ("4k3/8/8/8/8/8/8/R3K2R w Q - 0 1");

        CHECK( castling_term (kingside_only, Color::White) == Black_Penalty - 50 );
        CHECK( castling_term (queenside_only, Color::White) == Black_Penalty - 50 );
    }

    SUBCASE( "The full penalty for losing both rights" )
    {
        auto board = boardFromFen ("4k3/8/8/8/8/8/8/R3K2R w - - 0 1");

        CHECK( castling_term (board, Color::White) == 0 );
        CHECK( castling_term (board, Color::Black) == 0 );
    }

    SUBCASE( "No penalty for a king that looks castled" )
    {
        auto kingside = boardFromFen ("4k3/8/8/8/8/8/8/5RK1 w - - 0 1");
        auto queenside = boardFromFen ("4k3/8/8/8/8/8/8/2KR4 w - - 0 1");

        CHECK( castling_term (kingside, Color::White) == Black_Penalty );
        CHECK( castling_term (queenside, Color::White) == Black_Penalty );
    }

    SUBCASE( "A king on the castled square without the rook beside it is penalized" )
    {
        auto board = boardFromFen ("4k3/8/8/8/8/8/8/6KR w - - 0 1");

        CHECK( castling_term (board, Color::White) == 0 );
    }

    SUBCASE( "Black's castled king is recognized too" )
    {
        auto board = boardFromFen ("5rk1/8/8/8/8/8/8/4K3 w - - 0 1");

        CHECK( castling_term (board, Color::Black) == 100 );
        CHECK( castling_term (board, Color::White) == -100 );
    }
}

TEST_CASE( "DrawCategory" )
{
    CHECK( !static_cast<bool> (DrawCategory { DrawCategory::NoDraw }) );
    CHECK( static_cast<bool> (DrawCategory { DrawCategory::InsufficientMaterial }) );
    CHECK( static_cast<bool> (DrawCategory { DrawCategory::ByRepetition }) );
    CHECK( static_cast<bool> (DrawCategory { DrawCategory::ByNoProgress }) );

    CHECK( DrawCategory { DrawCategory::ByRepetition } == DrawCategory::ByRepetition );
    CHECK( DrawCategory { DrawCategory::ByRepetition } != DrawCategory::ByNoProgress );
}

TEST_CASE( "isProbablyDrawingMove" )
{
    auto shuffle_knights = [] (Game& game, int times)
    {
        for (int i = 0; i < times; i++)
        {
            game.move (moveParse ("g1 f3", Color::White));
            game.move (moveParse ("g8 f6", Color::Black));
            game.move (moveParse ("f3 g1", Color::White));
            game.move (moveParse ("f6 g8", Color::Black));
        }
    };

    SUBCASE( "The starting position is not a draw" )
    {
        auto game = Game::createStandardGame();

        CHECK( isProbablyDrawingMove (game.getBoard(), game.getHistory()) == DrawCategory::NoDraw );
    }

    SUBCASE( "Bare kings are a draw by insufficient material" )
    {
        auto game = Game::createGameFromFen ("4k3/8/8/8/8/8/8/4K3 w - - 0 1");

        CHECK( isProbablyDrawingMove (game.getBoard(), game.getHistory())
               == DrawCategory::InsufficientMaterial );
    }

    SUBCASE( "The third occurrence of a position is a draw by repetition" )
    {
        auto game = Game::createStandardGame();

        shuffle_knights (game, 1);
        CHECK( isProbablyDrawingMove (game.getBoard(), game.getHistory()) == DrawCategory::NoDraw );

        shuffle_knights (game, 1);
        CHECK( isProbablyDrawingMove (game.getBoard(), game.getHistory())
               == DrawCategory::ByRepetition );
    }

    SUBCASE( "After a declined threefold draw, only the fifth occurrence counts" )
    {
        auto game = Game::createStandardGame();
        game.getHistory().setThreefoldRepetitionStatus (DrawStatus::Declined);

        shuffle_knights (game, 3);
        CHECK( isProbablyDrawingMove (game.getBoard(), game.getHistory()) == DrawCategory::NoDraw );

        shuffle_knights (game, 1);
        CHECK( isProbablyDrawingMove (game.getBoard(), game.getHistory())
               == DrawCategory::ByRepetition );
    }

    SUBCASE( "Fifty moves without progress" )
    {
        auto before = Game::createGameFromFen ("4k3/8/8/8/8/8/8/R3K3 w - - 99 80");
        auto reached = Game::createGameFromFen ("4k3/8/8/8/8/8/8/R3K3 w - - 100 80");

        CHECK( isProbablyDrawingMove (before.getBoard(), before.getHistory())
               == DrawCategory::NoDraw );
        CHECK( isProbablyDrawingMove (reached.getBoard(), reached.getHistory())
               == DrawCategory::ByNoProgress );
    }

    SUBCASE( "After a declined fifty-move draw, only seventy-five moves count" )
    {
        auto declined = Game::createGameFromFen ("4k3/8/8/8/8/8/8/R3K3 w - - 149 80");
        declined.getHistory().setFiftyMovesWithoutProgressStatus (DrawStatus::Declined);

        auto reached = Game::createGameFromFen ("4k3/8/8/8/8/8/8/R3K3 w - - 150 80");
        reached.getHistory().setFiftyMovesWithoutProgressStatus (DrawStatus::Declined);

        CHECK( isProbablyDrawingMove (declined.getBoard(), declined.getHistory())
               == DrawCategory::NoDraw );
        CHECK( isProbablyDrawingMove (reached.getBoard(), reached.getHistory())
               == DrawCategory::ByNoProgress );
    }
}
