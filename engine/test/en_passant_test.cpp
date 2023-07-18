#include <doctest/doctest.h>
#include "board_builder.hpp"

#include "board.hpp"
#include "generate.hpp"
#include "coord.hpp"

using namespace wisdom;

TEST_CASE( "en passant" )
{
    SUBCASE( "En passant state starts out as negative 1" )
    {
        Board board;

        REQUIRE( !board.is_en_passant_vulnerable ( Color::White) );
        REQUIRE( !board.is_en_passant_vulnerable ( Color::Black) );

        REQUIRE( board.get_en_passant_target (Color::White) == No_En_Passant_Coord );
        REQUIRE( board.get_en_passant_target (Color::Black) == No_En_Passant_Coord );

        BoardBuilder builder;
        const auto& back_rank = BoardBuilder::Default_Piece_Row;
        builder.add_row_of_same_color ("a8", Color::Black, back_rank);
        builder.add_row_of_same_color_and_piece ("a7", Color::Black, Piece::Pawn);
        builder.add_piece ("e5", Color::White, Piece::Pawn);
        builder.add_row_of_same_color ("a1", Color::White, back_rank);

        auto builder_board = Board { builder };

        REQUIRE( !builder_board.is_en_passant_vulnerable ( Color::White) );
        REQUIRE( !builder_board.is_en_passant_vulnerable ( Color::Black) );
    }

    SUBCASE( "En passant moves work on the right" )
    {
        BoardBuilder builder;
        MoveGenerator move_generator;

        const auto& back_rank = BoardBuilder::Default_Piece_Row;

        builder.add_row_of_same_color ("a8", Color::Black, back_rank);
        builder.add_row_of_same_color_and_piece ("a7", Color::Black, Piece::Pawn);

        builder.add_piece ("e5", Color::White, Piece::Pawn);
        builder.add_row_of_same_color ("a1", Color::White, back_rank);

        builder.set_current_turn (Color::Black);

        auto board = Board { builder };

        REQUIRE( !board.is_en_passant_vulnerable (Color::Black) );
        REQUIRE( !board.is_en_passant_vulnerable (Color::White) );

        Move pawn_move = move_parse ("f7f5");
        board = board.with_move (Color::Black, pawn_move);

        MoveList move_list = move_generator.generate_all_potential_moves (board, Color::White);
        auto maybe_en_passant_move = std::find_if (
                move_list.begin (), move_list.end (), std::mem_fn (&Move::is_en_passant));

        REQUIRE( maybe_en_passant_move != move_list.end () );
        auto en_passant_move = *maybe_en_passant_move;

        // Check move types:
        REQUIRE( en_passant_move.is_en_passant () );

        // Check position:
        REQUIRE( Row (en_passant_move.get_src ()) == 3 );
        REQUIRE( Column (en_passant_move.get_src ()) == 4 );
        REQUIRE( Row (en_passant_move.get_dst ()) == 2 );
        REQUIRE( Column (en_passant_move.get_dst ()) == 5 );

        REQUIRE( board.is_en_passant_vulnerable (Color::Black) );
        REQUIRE( !board.is_en_passant_vulnerable (Color::White) );

        board = board.with_move (Color::White, en_passant_move);

        ColoredPiece en_passant_pawn = board.pieceAt (2, 5);
        REQUIRE( piece_type(en_passant_pawn) == Piece::Pawn );
        REQUIRE( piece_color (en_passant_pawn) == Color::White );

        ColoredPiece taken_pawn = board.pieceAt (3, 4);
        REQUIRE( piece_color (taken_pawn) == Color::None );
        REQUIRE( piece_type(taken_pawn) == Piece::None );
    }

    SUBCASE( "En passant moves work on the left" )
    {
        BoardBuilder builder;
        MoveGenerator move_generator;
        const auto& back_rank = BoardBuilder::Default_Piece_Row;

        builder.add_row_of_same_color ("a8", Color::Black, back_rank);
        builder.add_row_of_same_color_and_piece ("a7", Color::Black, Piece::Pawn);
        builder.add_piece ("e5", Color::White, Piece::Pawn);
        builder.add_row_of_same_color ("a1", Color::White, back_rank);
        builder.set_current_turn (Color::Black);

        auto board = Board { builder };
        Move pawn_move = move_parse ("d7d5");
        REQUIRE( !board.is_en_passant_vulnerable (Color::Black) );
        REQUIRE( !board.is_en_passant_vulnerable (Color::White) );

        board = board.with_move (Color::Black, pawn_move);

        MoveList move_list = move_generator.generate_all_potential_moves (board, Color::White);
        auto maybe_en_passant_move = std::find_if (
                move_list.begin(), move_list.end (), std::mem_fn (&Move::is_en_passant));

        REQUIRE( maybe_en_passant_move != move_list.end () );
        auto en_passant_move = *maybe_en_passant_move;

        // Check move types:
        REQUIRE( en_passant_move.is_en_passant() );

        // Check position:
        REQUIRE( Row (en_passant_move.get_src ()) == 3 );
        REQUIRE( Column (en_passant_move.get_src ()) == 4 );
        REQUIRE( Row (en_passant_move.get_dst ()) == 2 );
        REQUIRE( Column (en_passant_move.get_dst ()) == 3 );

        REQUIRE( board.is_en_passant_vulnerable (Color::Black) );
        REQUIRE( !board.is_en_passant_vulnerable (Color::White) );

        board = board.with_move (Color::White, en_passant_move);

        ColoredPiece en_passant_pawn = board.pieceAt (2, 3);
        REQUIRE( piece_type(en_passant_pawn) == Piece::Pawn );
        REQUIRE( piece_color (en_passant_pawn) == Color::White );

        ColoredPiece taken_pawn = board.pieceAt (3, 4);
        REQUIRE( piece_color (taken_pawn) == Color::None );
        REQUIRE( piece_type(taken_pawn) == Piece::None );
    }
}