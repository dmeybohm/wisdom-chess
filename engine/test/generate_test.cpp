#include "board.hpp"
#include "generate.hpp"
#include "tests.hpp"
#include "board_builder.hpp"

using namespace wisdom;

TEST_CASE("generate default moves")
{
    Board board;
    MoveGenerator move_generator;

    auto move_list = move_generator.generateAllPotentialMoves (board, Color::White);

    std::string expected = "{ [a2 a4] [a2 a3] [b2 b4] [b2 b3] [c2 c4] [c2 c3] "
                           "[d2 d4] [d2 d3] [e2 e4] [e2 e3] [f2 f4] [f2 f3] "
                           "[g2 g4] [g2 g3] [h2 h4] [h2 h3] [b1 a3] [b1 c3] "
                           "[g1 f3] [g1 h3] }";
    REQUIRE( move_list.to_string() == expected );
}

TEST_CASE("generate en passant moves")
{
    Board board;
    MoveGenerator move_generator;

    board = board.withMove (Color::White, move_parse ("e2 e4", Color::White));
    board = board.withMove (Color::Black, move_parse ("d7 d5", Color::Black));
    board = board.withMove (Color::White, move_parse ("e4 e5", Color::White));
    board = board.withMove (Color::Black, move_parse ("f7 f5", Color::Black));

    auto move_list = move_generator.generateAllPotentialMoves (board, Color::White).to_string ();
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

    MoveGenerator move_generator;
    auto move_list = move_generator.generateAllPotentialMoves (board, Color::Black);

    std::string expected = "{ [c4xd3] [c4xb3] ";
    std::string converted = move_list.to_string ().substr (0, expected.size ());

    INFO( move_list );
    REQUIRE( expected == converted );
}
