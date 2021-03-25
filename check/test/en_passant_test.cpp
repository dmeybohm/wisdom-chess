#include "doctest/doctest.h"
#include "board_builder.hpp"

#include "board.hpp"
#include "move_tree.hpp"
#include "generate.hpp"

using namespace wisdom;

TEST_CASE( "En passant state starts out as negative 1" )
{
    Board board;

    REQUIRE( !is_en_passant_vulnerable (board, Color::White) );
    REQUIRE( !is_en_passant_vulnerable (board, Color::Black) );

    BoardBuilder builder;
    std::vector<Piece> back_rank {
            Piece::Rook,   Piece::Knight, Piece::Bishop, Piece::Queen, Piece::King,
            Piece::Bishop, Piece::Knight, Piece::Rook
    };
    builder.add_row_of_same_color ("a8", Color::Black, back_rank);
    builder.add_row_of_same_color_and_piece ("a7", Color::Black, Piece::Pawn);
    builder.add_piece ("e5", Color::White, Piece::Pawn);
    builder.add_row_of_same_color ("a1", Color::White, back_rank);

    Board builder_board = builder.build();

    REQUIRE( !is_en_passant_vulnerable (builder_board, Color::White) );
    REQUIRE( !is_en_passant_vulnerable (builder_board, Color::Black) );
}

TEST_CASE( "En passant moves work on the right" )
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

    Board board = builder.build();

    Move pawn_move = parse_move ("f7f5");
    UndoMove first_undo_state = do_move (board, Color::Black, pawn_move);
    REQUIRE( !is_en_passant_vulnerable (first_undo_state, Color::Black) );
    REQUIRE( !is_en_passant_vulnerable (first_undo_state, Color::White) );

    MoveList move_list = generate_moves (board, Color::White);
    Move en_passant_move = null_move;

    for (auto move : move_list)
    {
        if (is_en_passant_move(move))
            en_passant_move = move;
    }

    REQUIRE( !is_null_move(en_passant_move) );

    // Check move types:
    REQUIRE( is_en_passant_move(en_passant_move) );

    // Check position:
    REQUIRE(ROW(move_src (en_passant_move)) == 3 );
    REQUIRE(COLUMN(move_src (en_passant_move)) == 4 );
    REQUIRE(ROW(move_dst (en_passant_move)) == 2 );
    REQUIRE(COLUMN(move_dst (en_passant_move)) == 5 );

    UndoMove en_passant_undo_state = do_move (board, Color::White, en_passant_move);

    REQUIRE( en_passant_undo_state.category == MoveCategory::EnPassant );
    REQUIRE( is_en_passant_vulnerable (en_passant_undo_state, Color::Black) );
    REQUIRE( !is_en_passant_vulnerable (en_passant_undo_state, Color::White) );

    ColoredPiece en_passant_pawn = piece_at (board, 2, 5);
    REQUIRE(piece_type(en_passant_pawn) == Piece::Pawn );
    REQUIRE(piece_color (en_passant_pawn) == Color::White );
    ColoredPiece taken_pawn = piece_at (board, 3, 4);
    REQUIRE(piece_color (taken_pawn) == Color::None );
    REQUIRE(piece_type(taken_pawn) == Piece::None );

    undo_move (board, Color::White, en_passant_move, en_passant_undo_state);
    REQUIRE( is_en_passant_vulnerable (board, Color::Black) );

    ColoredPiece en_passant_pawn_space = piece_at (board, 2, 5);
    REQUIRE(piece_type(en_passant_pawn_space) == Piece::None );
    REQUIRE(piece_color (en_passant_pawn_space) == Color::None );
    ColoredPiece en_passant_pawn_done = piece_at (board, 3, 4);
    REQUIRE(piece_type(en_passant_pawn_done) == Piece::Pawn );
    REQUIRE(piece_color (en_passant_pawn_done) == Color::White );
    ColoredPiece taken_pawn_undone = piece_at (board, 3, 5);
    REQUIRE(piece_color (taken_pawn_undone) == Color::Black );
    REQUIRE(piece_type(taken_pawn_undone) == Piece::Pawn );
}

TEST_CASE( "En passant moves work on the left" )
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

    Board board = builder.build();
    Move pawn_move = parse_move ("d7d5");
    UndoMove first_undo_state = do_move (board, Color::Black, pawn_move);
    REQUIRE( !is_en_passant_vulnerable (first_undo_state, Color::Black) );
    REQUIRE( !is_en_passant_vulnerable (first_undo_state, Color::White) );

    MoveList move_list = generate_moves (board, Color::White);
    Move en_passant_move = null_move;

    for (auto move : move_list)
    {
        if (is_en_passant_move(move))
            en_passant_move = move;
    }

    REQUIRE( !is_null_move(en_passant_move) );

    // Check move types:
    REQUIRE( is_en_passant_move(en_passant_move) );

    // Check position:
    REQUIRE(ROW(move_src (en_passant_move)) == 3 );
    REQUIRE(COLUMN(move_src (en_passant_move)) == 4 );
    REQUIRE(ROW(move_dst (en_passant_move)) == 2 );
    REQUIRE(COLUMN(move_dst (en_passant_move)) == 3 );

    UndoMove en_passant_undo_state = do_move (board, Color::White, en_passant_move);

    REQUIRE( en_passant_undo_state.category == MoveCategory::EnPassant );
    REQUIRE( is_en_passant_vulnerable (en_passant_undo_state, Color::Black) );
    REQUIRE( !is_en_passant_vulnerable (en_passant_undo_state, Color::White) );

    ColoredPiece en_passant_pawn = piece_at (board, 2, 3);
    REQUIRE(piece_type(en_passant_pawn) == Piece::Pawn );
    REQUIRE(piece_color (en_passant_pawn) == Color::White );
    ColoredPiece taken_pawn = piece_at (board, 3, 4);
    REQUIRE(piece_color (taken_pawn) == Color::None );
    REQUIRE(piece_type(taken_pawn) == Piece::None );

    undo_move (board, Color::White, en_passant_move, en_passant_undo_state);
    REQUIRE( is_en_passant_vulnerable (board, Color::Black) );

    ColoredPiece en_passant_pawn_space = piece_at (board, 2, 5);
    REQUIRE(piece_type(en_passant_pawn_space) == Piece::None );
    REQUIRE(piece_color (en_passant_pawn_space) == Color::None );
    ColoredPiece en_passant_pawn_done = piece_at (board, 3, 4);
    REQUIRE(piece_type(en_passant_pawn_done) == Piece::Pawn );
    REQUIRE(piece_color (en_passant_pawn_done) == Color::White );
    ColoredPiece taken_pawn_undone = piece_at (board, 3, 3);
    REQUIRE(piece_color (taken_pawn_undone) == Color::Black );
    REQUIRE(piece_type(taken_pawn_undone) == Piece::Pawn );
}

TEST_CASE( "En passant state is reset after en passant" )
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

    Board board = builder.build();
    Move first_move = parse_move ("d7d5");
    Move en_passant = parse_move ("e5 d4 (ep)");

    UndoMove first_undo = do_move (board, Color::Black, first_move);
    REQUIRE( is_en_passant_vulnerable (board, Color::Black) );
    REQUIRE( !is_en_passant_vulnerable (board, Color::White) );

    UndoMove second_undo = do_move (board, Color::White, en_passant);
    REQUIRE( !is_en_passant_vulnerable (board, Color::Black) );
    REQUIRE( !is_en_passant_vulnerable (board, Color::White) );

    undo_move (board, Color::White, en_passant, second_undo);
    REQUIRE( is_en_passant_vulnerable (board, Color::Black) );
    REQUIRE( !is_en_passant_vulnerable (board, Color::White) );

    undo_move (board, Color::Black, first_move, first_undo);
    REQUIRE( !is_en_passant_vulnerable (board, Color::Black) );
    REQUIRE( !is_en_passant_vulnerable (board, Color::White) );
}
