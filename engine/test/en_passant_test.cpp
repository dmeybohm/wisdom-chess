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

        Move pawn_move = move_parse ("f7f5");
        UndoMove first_undo_state = board.make_move (Color::Black, pawn_move);
        REQUIRE( !is_en_passant_vulnerable (first_undo_state, Color::Black) );
        REQUIRE( !is_en_passant_vulnerable (first_undo_state, Color::White) );

        MoveList move_list = move_generator.generate_all_potential_moves (board, Color::White);
        auto maybe_en_passant_move = std::find_if (
                move_list.begin (), move_list.end (), std::mem_fn (&Move::is_en_passant));

        REQUIRE( maybe_en_passant_move != move_list.end () );
        auto en_passant_move = *maybe_en_passant_move;

        // Check move types:
        REQUIRE(en_passant_move.is_en_passant () );

        // Check position:
        REQUIRE( Row (en_passant_move.get_src ()) == 3 );
        REQUIRE( Column (en_passant_move.get_src ()) == 4 );
        REQUIRE( Row (en_passant_move.get_dst ()) == 2 );
        REQUIRE( Column (en_passant_move.get_dst ()) == 5 );

        UndoMove en_passant_undo_state = board.make_move (Color::White, en_passant_move);

        REQUIRE( en_passant_undo_state.category == MoveCategory::EnPassant);
        REQUIRE( is_en_passant_vulnerable (en_passant_undo_state, Color::Black) );
        REQUIRE( !is_en_passant_vulnerable (en_passant_undo_state, Color::White) );

        ColoredPiece en_passant_pawn = board.piece_at (2, 5);
        REQUIRE( piece_type(en_passant_pawn) == Piece::Pawn );
        REQUIRE( piece_color (en_passant_pawn) == Color::White );

        ColoredPiece taken_pawn = board.piece_at (3, 4);
        REQUIRE( piece_color (taken_pawn) == Color::None );
        REQUIRE( piece_type(taken_pawn) == Piece::None );

        board.take_back (Color::White, en_passant_move, en_passant_undo_state);
        REQUIRE( board.is_en_passant_vulnerable ( Color::Black) );

        ColoredPiece en_passant_pawn_space = board.piece_at (2, 5);
        REQUIRE( piece_type(en_passant_pawn_space) == Piece::None );
        REQUIRE( piece_color (en_passant_pawn_space) == Color::None );

        ColoredPiece en_passant_pawn_done = board.piece_at (3, 4);
        REQUIRE( piece_type(en_passant_pawn_done) == Piece::Pawn );
        REQUIRE( piece_color (en_passant_pawn_done) == Color::White );

        ColoredPiece taken_pawn_undone = board.piece_at (3, 5);
        REQUIRE( piece_color (taken_pawn_undone) == Color::Black );
        REQUIRE( piece_type(taken_pawn_undone) == Piece::Pawn );
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
        UndoMove first_undo_state = board.make_move (Color::Black, pawn_move);
        REQUIRE( !is_en_passant_vulnerable (first_undo_state, Color::Black) );
        REQUIRE( !is_en_passant_vulnerable (first_undo_state, Color::White) );

        MoveList move_list = move_generator.generate_all_potential_moves (board, Color::White);
        auto maybe_en_passant_move = std::find_if (
                move_list.begin(), move_list.end (), std::mem_fn (&Move::is_en_passant));

        REQUIRE( maybe_en_passant_move != move_list.end () );
        auto en_passant_move = *maybe_en_passant_move;

        // Check move types:
        REQUIRE(en_passant_move.is_en_passant () );

        // Check position:
        REQUIRE( Row (en_passant_move.get_src ()) == 3 );
        REQUIRE( Column (en_passant_move.get_src ()) == 4 );
        REQUIRE( Row (en_passant_move.get_dst ()) == 2 );
        REQUIRE( Column (en_passant_move.get_dst ()) == 3 );

        UndoMove en_passant_undo_state = board.make_move (Color::White, en_passant_move);

        REQUIRE( en_passant_undo_state.category == MoveCategory::EnPassant);
        REQUIRE( is_en_passant_vulnerable (en_passant_undo_state, Color::Black) );
        REQUIRE( !is_en_passant_vulnerable (en_passant_undo_state, Color::White) );

        ColoredPiece en_passant_pawn = board.piece_at (2, 3);
        REQUIRE( piece_type(en_passant_pawn) == Piece::Pawn );
        REQUIRE( piece_color (en_passant_pawn) == Color::White );

        ColoredPiece taken_pawn = board.piece_at (3, 4);
        REQUIRE( piece_color (taken_pawn) == Color::None );
        REQUIRE( piece_type(taken_pawn) == Piece::None );

        board.take_back (Color::White, en_passant_move, en_passant_undo_state);
        REQUIRE( board.is_en_passant_vulnerable ( Color::Black) );

        ColoredPiece en_passant_pawn_space = board.piece_at (2, 5);
        REQUIRE( piece_type(en_passant_pawn_space) == Piece::None );
        REQUIRE( piece_color (en_passant_pawn_space) == Color::None );

        ColoredPiece en_passant_pawn_done = board.piece_at (3, 4);
        REQUIRE( piece_type(en_passant_pawn_done) == Piece::Pawn );
        REQUIRE( piece_color (en_passant_pawn_done) == Color::White );

        ColoredPiece taken_pawn_undone = board.piece_at (3, 3);
        REQUIRE( piece_color (taken_pawn_undone) == Color::Black );
        REQUIRE( piece_type(taken_pawn_undone) == Piece::Pawn );
    }

    SUBCASE( "En passant state is reset after en passant" )
    {
        BoardBuilder builder;
        const auto& back_rank = BoardBuilder::Default_Piece_Row;
        builder.add_row_of_same_color ("a8", Color::Black, back_rank);
        builder.add_row_of_same_color_and_piece ("a7", Color::Black, Piece::Pawn);
        builder.add_piece ("e5", Color::White, Piece::Pawn);
        builder.add_row_of_same_color ("a1", Color::White, back_rank);
        builder.set_current_turn (Color::Black);

        auto board = Board { builder };
        Move first_move = move_parse ("d7d5");
        Move en_passant = move_parse ("e5 d4 (ep)");

        UndoMove first_undo = board.make_move (Color::Black, first_move);
        REQUIRE( board.is_en_passant_vulnerable ( Color::Black) );
        REQUIRE( !board.is_en_passant_vulnerable ( Color::White) );

        UndoMove second_undo = board.make_move (Color::White, en_passant);
        REQUIRE( !board.is_en_passant_vulnerable ( Color::Black) );
        REQUIRE( !board.is_en_passant_vulnerable ( Color::White) );

        board.take_back (Color::White, en_passant, second_undo);
        REQUIRE( board.is_en_passant_vulnerable ( Color::Black) );
        REQUIRE( !board.is_en_passant_vulnerable ( Color::White) );

        board.take_back (Color::Black, first_move, first_undo);
        REQUIRE( !board.is_en_passant_vulnerable ( Color::Black) );
        REQUIRE( !board.is_en_passant_vulnerable ( Color::White) );
    }
}