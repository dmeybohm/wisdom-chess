#include <doctest/doctest.h>
#include "board_builder.hpp"

#include "board.hpp"
#include "generate.hpp"

using namespace wisdom;

TEST_CASE( "en passant" )
{
    SUBCASE( "En passant state starts out as negative 1" )
    {
        Board board;

        REQUIRE( !board.is_en_passant_vulnerable ( Color::White) );
        REQUIRE( !board.is_en_passant_vulnerable ( Color::Black) );

        BoardBuilder builder;
        std::vector<Piece> back_rank {
                Piece::Rook,   Piece::Knight, Piece::Bishop, Piece::Queen, Piece::King,
                Piece::Bishop, Piece::Knight, Piece::Rook
        };
        builder.add_row_of_same_color ("a8", Color::Black, back_rank);
        builder.add_row_of_same_color_and_piece ("a7", Color::Black, Piece::Pawn);
        builder.add_piece ("e5", Color::White, Piece::Pawn);
        builder.add_row_of_same_color ("a1", Color::White, back_rank);

        auto builder_board = builder.build();

        REQUIRE( !builder_board->is_en_passant_vulnerable ( Color::White) );
        REQUIRE( !builder_board->is_en_passant_vulnerable ( Color::Black) );
    }

    SUBCASE( "En passant moves work on the right" )
    {
        BoardBuilder builder;
        std::vector<Piece> back_rank {
                Piece::Rook,   Piece::Knight, Piece::Bishop, Piece::Queen, Piece::King,
                Piece::Bishop, Piece::Knight, Piece::Rook
        };
        builder.add_row_of_same_color ("a8", Color::Black, back_rank);
        builder.add_row_of_same_color_and_piece ("a7", Color::Black, Piece::Pawn);
        builder.add_piece ("e5", Color::White, Piece::Pawn);
        builder.add_row_of_same_color ("a1", Color::White, back_rank);

        auto board = builder.build();

        Move pawn_move = move_parse ("f7f5");
        UndoMove first_undo_state = board->make_move (Color::Black, pawn_move);
        REQUIRE( !is_en_passant_vulnerable (first_undo_state, Color::Black) );
        REQUIRE( !is_en_passant_vulnerable (first_undo_state, Color::White) );

        MoveList move_list = generate_moves (*board, Color::White);
        std::optional<Move> optional_en_passant_move = std::nullopt;

        for (auto move : move_list)
        {
            if (is_en_passant_move (move))
                optional_en_passant_move = move;
        }

        REQUIRE( optional_en_passant_move.has_value() );
        auto en_passant_move = *optional_en_passant_move;

        // Check move types:
        REQUIRE( is_en_passant_move (en_passant_move) );

        // Check position:
        REQUIRE( Row (move_src (en_passant_move)) == 3 );
        REQUIRE( Column (move_src (en_passant_move)) == 4 );
        REQUIRE( Row (move_dst (en_passant_move)) == 2 );
        REQUIRE( Column (move_dst (en_passant_move)) == 5 );

        UndoMove en_passant_undo_state = board->make_move (Color::White, en_passant_move);

        REQUIRE( en_passant_undo_state.category == MoveCategory::EnPassant );
        REQUIRE( is_en_passant_vulnerable (en_passant_undo_state, Color::Black) );
        REQUIRE( !is_en_passant_vulnerable (en_passant_undo_state, Color::White) );

        ColoredPiece en_passant_pawn = board->piece_at (2, 5);
        REQUIRE( piece_type(en_passant_pawn) == Piece::Pawn );
        REQUIRE( piece_color (en_passant_pawn) == Color::White );

        ColoredPiece taken_pawn = board->piece_at (3, 4);
        REQUIRE( piece_color (taken_pawn) == Color::None );
        REQUIRE( piece_type(taken_pawn) == Piece::None );

        board->take_back (Color::White, en_passant_move, en_passant_undo_state);
        REQUIRE( board->is_en_passant_vulnerable ( Color::Black) );

        ColoredPiece en_passant_pawn_space = board->piece_at (2, 5);
        REQUIRE( piece_type(en_passant_pawn_space) == Piece::None );
        REQUIRE( piece_color (en_passant_pawn_space) == Color::None );

        ColoredPiece en_passant_pawn_done = board->piece_at (3, 4);
        REQUIRE( piece_type(en_passant_pawn_done) == Piece::Pawn );
        REQUIRE( piece_color (en_passant_pawn_done) == Color::White );

        ColoredPiece taken_pawn_undone = board->piece_at (3, 5);
        REQUIRE( piece_color (taken_pawn_undone) == Color::Black );
        REQUIRE( piece_type(taken_pawn_undone) == Piece::Pawn );
    }

    SUBCASE( "En passant moves work on the left" )
    {
        BoardBuilder builder;
        std::vector<Piece> back_rank {
                Piece::Rook,   Piece::Knight, Piece::Bishop, Piece::Queen, Piece::King,
                Piece::Bishop, Piece::Knight, Piece::Rook
        };
        builder.add_row_of_same_color ("a8", Color::Black, back_rank);
        builder.add_row_of_same_color_and_piece ("a7", Color::Black, Piece::Pawn);
        builder.add_piece ("e5", Color::White, Piece::Pawn);
        builder.add_row_of_same_color ("a1", Color::White, back_rank);

        auto board = builder.build();
        Move pawn_move = move_parse ("d7d5");
        UndoMove first_undo_state = board->make_move (Color::Black, pawn_move);
        REQUIRE( !is_en_passant_vulnerable (first_undo_state, Color::Black) );
        REQUIRE( !is_en_passant_vulnerable (first_undo_state, Color::White) );

        MoveList move_list = generate_moves (*board, Color::White);
        std::optional<Move> optional_en_passant_move = std::nullopt;

        for (auto move : move_list)
        {
            if (is_en_passant_move (move))
                optional_en_passant_move = move;
        }

        REQUIRE( optional_en_passant_move.has_value () );
        auto en_passant_move = *optional_en_passant_move;

        // Check move types:
        REQUIRE( is_en_passant_move(en_passant_move) );

        // Check position:
        REQUIRE( Row (move_src (en_passant_move)) == 3 );
        REQUIRE( Column (move_src (en_passant_move)) == 4 );
        REQUIRE( Row (move_dst (en_passant_move)) == 2 );
        REQUIRE( Column (move_dst (en_passant_move)) == 3 );

        UndoMove en_passant_undo_state = board->make_move (Color::White, en_passant_move);

        REQUIRE( en_passant_undo_state.category == MoveCategory::EnPassant );
        REQUIRE( is_en_passant_vulnerable (en_passant_undo_state, Color::Black) );
        REQUIRE( !is_en_passant_vulnerable (en_passant_undo_state, Color::White) );

        ColoredPiece en_passant_pawn = board->piece_at (2, 3);
        REQUIRE( piece_type(en_passant_pawn) == Piece::Pawn );
        REQUIRE( piece_color (en_passant_pawn) == Color::White );

        ColoredPiece taken_pawn = board->piece_at (3, 4);
        REQUIRE( piece_color (taken_pawn) == Color::None );
        REQUIRE( piece_type(taken_pawn) == Piece::None );

        board->take_back (Color::White, en_passant_move, en_passant_undo_state);
        REQUIRE( board->is_en_passant_vulnerable ( Color::Black) );

        ColoredPiece en_passant_pawn_space = board->piece_at (2, 5);
        REQUIRE( piece_type(en_passant_pawn_space) == Piece::None );
        REQUIRE( piece_color (en_passant_pawn_space) == Color::None );

        ColoredPiece en_passant_pawn_done = board->piece_at (3, 4);
        REQUIRE( piece_type(en_passant_pawn_done) == Piece::Pawn );
        REQUIRE( piece_color (en_passant_pawn_done) == Color::White );

        ColoredPiece taken_pawn_undone = board->piece_at (3, 3);
        REQUIRE( piece_color (taken_pawn_undone) == Color::Black );
        REQUIRE( piece_type(taken_pawn_undone) == Piece::Pawn );
    }

    SUBCASE( "En passant state is reset after en passant" )
    {
        BoardBuilder builder;
        std::vector<Piece> back_rank {
                Piece::Rook,   Piece::Knight, Piece::Bishop, Piece::Queen, Piece::King,
                Piece::Bishop, Piece::Knight, Piece::Rook
        };
        builder.add_row_of_same_color ("a8", Color::Black, back_rank);
        builder.add_row_of_same_color_and_piece ("a7", Color::Black, Piece::Pawn);
        builder.add_piece ("e5", Color::White, Piece::Pawn);
        builder.add_row_of_same_color ("a1", Color::White, back_rank);

        auto board = builder.build();
        Move first_move = move_parse ("d7d5");
        Move en_passant = move_parse ("e5 d4 (ep)");

        UndoMove first_undo = board->make_move (Color::Black, first_move);
        REQUIRE( board->is_en_passant_vulnerable ( Color::Black) );
        REQUIRE( !board->is_en_passant_vulnerable ( Color::White) );

        UndoMove second_undo = board->make_move (Color::White, en_passant);
        REQUIRE( !board->is_en_passant_vulnerable ( Color::Black) );
        REQUIRE( !board->is_en_passant_vulnerable ( Color::White) );

        board->take_back (Color::White, en_passant, second_undo);
        REQUIRE( board->is_en_passant_vulnerable ( Color::Black) );
        REQUIRE( !board->is_en_passant_vulnerable ( Color::White) );

        board->take_back (Color::Black, first_move, first_undo);
        REQUIRE( !board->is_en_passant_vulnerable ( Color::Black) );
        REQUIRE( !board->is_en_passant_vulnerable ( Color::White) );
    }

}