#include "board.hpp"
#include "generate.hpp"
#include "tests.hpp"
#include "board_builder.hpp"

using namespace wisdom;

TEST_CASE("generate default moves")
{
    Board board;
    MoveGenerator move_generator;

    auto move_list = move_generator.generate_all_potential_moves (board, Color::White);

    std::string expected = "{ [a2 a3] [a2 a4] [b2 b3] [b2 b4] [c2 c3] [c2 c4] "
                           "[d2 d3] [d2 d4] [e2 e3] [e2 e4] [f2 f3] [f2 f4] "
                           "[g2 g3] [g2 g4] [h2 h3] [h2 h4] [b1 a3] [b1 c3] "
                           "[g1 f3] [g1 h3] }";
    REQUIRE( move_list.to_string() == expected );
}

TEST_CASE("generate en passant moves")
{
    Board board;
    MoveGenerator move_generator;

    board.make_move (Color::White, move_parse ("e2 e4", Color::White));
    board.make_move (Color::Black, move_parse ("d7 d5", Color::Black));
    board.make_move (Color::White, move_parse ("e4 e5", Color::White));
    board.make_move (Color::Black, move_parse ("f7 f5", Color::Black));

    auto move_list = move_generator.generate_all_potential_moves (board, Color::White).to_string ();
    auto pos = move_list.find ("[e5 f6 ep]");

    INFO( move_list );
    REQUIRE( pos != std::string::npos );
}

TEST_CASE( "Generated moves are sorted by capturing difference of pieces" )
{
    BoardBuilder builder;

    builder.add_piece ("c4", Color::Black, Piece::Pawn);
    builder.add_piece ("e4", Color::Black, Piece::Queen);
    builder.add_piece ("d3", Color::White, Piece::Queen);
    builder.add_piece ("b3", Color::White, Piece::Bishop);
    builder.add_piece ("a1", Color::White, Piece::King);
    builder.add_piece ("e1", Color::Black, Piece::King);
    builder.set_current_turn (Color::Black);

    auto board = builder.build();

    MoveGenerator move_generator;
    auto move_list = move_generator.generate_all_potential_moves (*board, Color::Black);

    std::string expected = "{ [c4xd3] [c4xb3] ";
    std::string converted = move_list.to_string ().substr (0, expected.size ());

    INFO( move_list );
    REQUIRE( expected == converted );
}
