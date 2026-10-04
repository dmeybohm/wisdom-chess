#include <doctest/doctest.h>

#include <map>
#include <set>
#include <string>

#include "wisdom-chess/ui/wasm/web_game.hpp"

using namespace wisdom;

namespace
{
    auto
    squareName (const WebColoredPiece& piece)
        -> std::string
    {
        return asString (makeCoord (piece.row, piece.col));
    }

    // The id of each piece, keyed by its square.
    auto
    idsBySquare (nonnull<WebGame> game)
        -> std::map<std::string, int>
    {
        std::map<std::string, int> result;
        auto& list = game->getPieceList();
        for (int i = 0; i < list.length; i++)
            result[squareName (list.pieceAt (i))] = list.pieceAt (i).id;
        return result;
    }

    auto
    pieceOn (nonnull<WebGame> game, czstring square)
        -> WebColoredPiece
    {
        auto& list = game->getPieceList();
        for (int i = 0; i < list.length; i++)
        {
            if (squareName (list.pieceAt (i)) == square)
                return list.pieceAt (i);
        }
        return WebColoredPiece {};
    }

    // A web game between two humans, and an engine game fed the same
    // moves to check the piece list against.
    struct Fixture
    {
        WebGame game { WebPlayer::Human, WebPlayer::Human, 1 };
        Game shadow = Game::createGame (Player::Human, Player::Human);

        void play (czstring src, czstring dst, WebPiece promoted = WebPiece::NoPiece)
        {
            CAPTURE( src );
            CAPTURE( dst );
            int packed_move = game.makeHumanMove (src, dst, promoted);
            REQUIRE( packed_move != WebGame::Illegal_Move );
            shadow.move (Move::fromPacked (packed_move));
            checkPieceList();
        }

        void playAll (std::initializer_list<std::pair<czstring, czstring>> moves)
        {
            for (auto [src, dst] : moves)
                play (src, dst);
        }

        // Every id is unique and in order, and the list shows exactly what
        // is on the board.
        void checkPieceList()
        {
            auto& list = game.getPieceList();
            const auto& board = shadow.getBoard();

            std::set<int> ids;
            int occupied = 0;
            for (int i = 0; i < list.length; i++)
            {
                auto piece = list.pieceAt (i);
                ids.insert (piece.id);
                if (i > 0)
                    CHECK( list.pieceAt (i - 1).id < piece.id );
                CHECK( board.pieceAt (piece.row, piece.col) == mapColoredPiece (piece) );
            }
            for (auto coord : Board::allCoords())
            {
                if (board.pieceAt (coord) != Piece_And_Color_None)
                    occupied++;
            }

            CHECK( narrow<int> (ids.size()) == list.length );
            CHECK( list.length == occupied );
        }
    };
}

TEST_CASE( "The starting position lists every piece in square order" )
{
    Fixture fixture;
    auto& list = fixture.game.getPieceList();

    REQUIRE( list.length == 32 );
    for (int i = 0; i < list.length; i++)
        CHECK( list.pieceAt (i).id == i + 1 );
    CHECK( squareName (list.pieceAt (0)) == "a8" );
    CHECK( squareName (list.pieceAt (31)) == "h1" );
    fixture.checkPieceList();
}

TEST_CASE( "A move keeps every piece's id" )
{
    Fixture fixture;
    auto before = idsBySquare (&fixture.game);

    fixture.play ("e2", "e4");

    auto after = idsBySquare (&fixture.game);
    CHECK( after.at ("e4") == before.at ("e2") );
    CHECK( after.count ("e2") == 0 );
    after.erase ("e4");
    before.erase ("e2");
    CHECK( after == before );
}

TEST_CASE( "A capture removes the captured piece's id" )
{
    Fixture fixture;
    fixture.playAll ({ { "e2", "e4" }, { "d7", "d5" } });
    auto mover = pieceOn (&fixture.game, "e4").id;
    auto captured = pieceOn (&fixture.game, "d5").id;

    fixture.play ("e4", "d5");

    CHECK( pieceOn (&fixture.game, "d5").id == mover );
    CHECK( fixture.game.getPieceList().length == 31 );
    for (auto [square, id] : idsBySquare (&fixture.game))
        CHECK( id != captured );
}

TEST_CASE( "Castling moves the rook's id with the king's" )
{
    struct Case
    {
        czstring name;
        std::initializer_list<std::pair<czstring, czstring>> setup;
        czstring king_src;
        czstring king_dst;
        czstring rook_src;
        czstring rook_dst;
    };

    const Case cases[] = {
        { "white kingside",
          { { "e2", "e4" }, { "e7", "e5" }, { "g1", "f3" }, { "b8", "c6" }, { "f1", "c4" }, { "d7", "d6" } },
          "e1", "g1", "h1", "f1" },
        { "white queenside",
          { { "d2", "d4" }, { "d7", "d5" }, { "b1", "c3" }, { "b8", "c6" }, { "c1", "f4" }, { "c8", "f5" },
            { "d1", "d2" }, { "d8", "d7" } },
          "e1", "c1", "a1", "d1" },
        { "black kingside",
          { { "e2", "e4" }, { "e7", "e5" }, { "g1", "f3" }, { "g8", "f6" }, { "f1", "c4" }, { "f8", "c5" },
            { "a2", "a3" } },
          "e8", "g8", "h8", "f8" },
        { "black queenside",
          { { "d2", "d4" }, { "d7", "d5" }, { "b1", "c3" }, { "b8", "c6" }, { "c1", "f4" }, { "c8", "f5" },
            { "d1", "d2" }, { "d8", "d7" }, { "a2", "a3" } },
          "e8", "c8", "a8", "d8" },
    };

    for (const auto& test : cases)
    {
        CAPTURE( test.name );
        Fixture fixture;
        fixture.playAll (test.setup);
        auto king = pieceOn (&fixture.game, test.king_src).id;
        auto rook = pieceOn (&fixture.game, test.rook_src).id;

        fixture.play (test.king_src, test.king_dst);

        CHECK( pieceOn (&fixture.game, test.king_dst).id == king );
        CHECK( pieceOn (&fixture.game, test.rook_dst).id == rook );
        CHECK( pieceOn (&fixture.game, test.rook_src).id == 0 );
    }
}

TEST_CASE( "En passant removes the pawn beside the destination" )
{
    SUBCASE( "By White" )
    {
        Fixture fixture;
        fixture.playAll ({ { "e2", "e4" }, { "a7", "a6" }, { "e4", "e5" }, { "d7", "d5" } });
        auto mover = pieceOn (&fixture.game, "e5").id;

        fixture.play ("e5", "d6");

        CHECK( pieceOn (&fixture.game, "d6").id == mover );
        CHECK( pieceOn (&fixture.game, "d5").id == 0 );
        CHECK( fixture.game.getPieceList().length == 31 );
    }

    SUBCASE( "By Black" )
    {
        Fixture fixture;
        fixture.playAll ({ { "a2", "a3" }, { "d7", "d5" }, { "a3", "a4" }, { "d5", "d4" }, { "e2", "e4" } });
        auto mover = pieceOn (&fixture.game, "d4").id;

        fixture.play ("d4", "e3");

        CHECK( pieceOn (&fixture.game, "e3").id == mover );
        CHECK( pieceOn (&fixture.game, "e4").id == 0 );
        CHECK( fixture.game.getPieceList().length == 31 );
    }
}

TEST_CASE( "A promoted pawn keeps its id and changes its type" )
{
    Fixture fixture;
    fixture.playAll ({
        { "h2", "h4" }, { "g7", "g5" }, { "h4", "g5" }, { "h7", "h6" },
        { "g5", "h6" }, { "f8", "g7" }, { "h6", "g7" }, { "a7", "a6" },
    });
    CHECK( fixture.game.needsPawnPromotion ("g7", "h8") );
    CHECK( !fixture.game.needsPawnPromotion ("e2", "e4") );
    auto white_pawn = pieceOn (&fixture.game, "g7").id;

    fixture.play ("g7", "h8", WebPiece::Queen);

    CHECK( pieceOn (&fixture.game, "h8").id == white_pawn );
    CHECK( pieceOn (&fixture.game, "h8").piece == WebPiece::Queen );

    fixture.playAll ({
        { "a6", "a5" }, { "b2", "b4" }, { "a5", "b4" }, { "c2", "c3" },
        { "b4", "c3" }, { "a2", "a3" }, { "c3", "c2" }, { "a3", "a4" },
    });
    auto black_pawn = pieceOn (&fixture.game, "c2").id;

    fixture.play ("c2", "b1", WebPiece::Knight);

    CHECK( pieceOn (&fixture.game, "b1").id == black_pawn );
    CHECK( pieceOn (&fixture.game, "b1").piece == WebPiece::Knight );
    CHECK( pieceOn (&fixture.game, "b1").color == WebColor::Black );
}

TEST_CASE( "The piece list follows a longer game" )
{
    Fixture fixture;
    fixture.playAll ({
        { "e2", "e4" }, { "e7", "e5" }, { "g1", "f3" }, { "b8", "c6" }, { "f1", "c4" }, { "g8", "f6" },
        { "e1", "g1" }, { "f6", "e4" }, { "d2", "d4" }, { "e5", "d4" }, { "f1", "e1" }, { "d7", "d5" },
        { "c4", "d5" }, { "d8", "d5" }, { "b1", "c3" }, { "d5", "a5" }, { "c3", "e4" }, { "c8", "e6" },
        { "e4", "g5" }, { "e8", "c8" },
    });

    CHECK( fixture.game.getPieceList().length == 27 );
}

TEST_CASE( "A computer move updates the piece list" )
{
    Fixture fixture;
    auto mover = pieceOn (&fixture.game, "g1").id;

    fixture.game.makeComputerMove ("g1 f3");

    CHECK( pieceOn (&fixture.game, "f3").id == mover );
    CHECK( fixture.game.getCurrentTurn() == WebColor::Black );
}

TEST_CASE( "An illegal human move changes nothing" )
{
    Fixture fixture;
    auto before = idsBySquare (&fixture.game);

    SUBCASE( "A move the piece cannot make" )
    {
        CHECK( fixture.game.makeHumanMove ("e2", "e5", WebPiece::NoPiece) == WebGame::Illegal_Move );
        CHECK( std::string { fixture.game.getMoveStatus() } == "Illegal move" );
    }

    SUBCASE( "A square that does not parse" )
    {
        CHECK( fixture.game.makeHumanMove ("z9", "e4", WebPiece::NoPiece) == WebGame::Illegal_Move );
        CHECK( fixture.game.makeHumanMove (nullptr, "e4", WebPiece::NoPiece) == WebGame::Illegal_Move );
    }

    SUBCASE( "Moving the other side's piece" )
    {
        CHECK( fixture.game.makeHumanMove ("e7", "e5", WebPiece::NoPiece) == WebGame::Illegal_Move );
    }

    CHECK( idsBySquare (&fixture.game) == before );
    CHECK( fixture.game.getCurrentTurn() == WebColor::White );
}

TEST_CASE( "A human cannot move for the engine" )
{
    WebGame game { WebPlayer::ChessEngine, WebPlayer::Human, 1 };

    CHECK( game.makeHumanMove ("e2", "e4", WebPiece::NoPiece) == WebGame::Illegal_Move );
}

TEST_CASE( "Checkmate is reported" )
{
    Fixture fixture;
    fixture.playAll ({ { "f2", "f3" }, { "e7", "e5" }, { "g2", "g4" }, { "d8", "h4" } });

    CHECK( fixture.game.getGameStatus() == WebGameStatus::Checkmate );
    CHECK( fixture.game.getInCheck() );
    CHECK( fixture.game.moveNumber == 4 );
}
