#include "catch.hpp"
#include "board_builder.hpp"

#include "board.h"


TEST_CASE("Castling state is modified and restored for rooks", "[castling]")
{
    std::vector<Piece> back_rank =
    {
        Piece::Rook,   Piece::None, Piece::Bishop, Piece::Queen, Piece::King,
        Piece::Bishop, Piece::None, Piece::Rook
    };

    struct std::vector<board_positions> positions =
    {
        { 0, Color::Black, back_rank },
        { 7, Color::White, back_rank },
    };

    struct board board { positions };
    move_t mv = make_move (0, 0, 0, 1);

    CHECK( able_to_castle (board, Color::Black, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, Color::Black, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::Black, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::Black)] == CASTLE_NONE );

    undo_move_t undo_state = do_move (board, Color::Black, mv);

    CHECK( able_to_castle (board, Color::Black, CASTLE_QUEENSIDE) == 0 );
    CHECK( able_to_castle (board, Color::Black, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::Black, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::Black)] == CASTLE_QUEENSIDE );

    undo_move (board, Color::Black, mv, undo_state);

    CHECK( able_to_castle (board, Color::Black, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, Color::Black, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::Black, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::Black)] == CASTLE_NONE );
}

TEST_CASE("Castling state is modified and restored for kings", "[castling]")
{
    std::vector<Piece> back_rank =
    {
        Piece::Rook,   Piece::None, Piece::Bishop, Piece::None, Piece::King,
        Piece::Bishop, Piece::None, Piece::Rook
    };

    std::vector<board_positions> positions =
    {
        { 0, Color::Black, back_rank },
        { 7, Color::White, back_rank },
    };

    struct board board { positions };
    move_t mv = make_move (0, 4, 0, 3);

    CHECK( able_to_castle(board, Color::Black, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle(board, Color::Black, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle(board, Color::Black, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::Black)] == CASTLE_NONE );

    undo_move_t undo_state = do_move (board, Color::Black, mv);;

    CHECK( able_to_castle(board, Color::Black, CASTLE_QUEENSIDE) == 0 );
    CHECK( able_to_castle(board, Color::Black, CASTLE_KINGSIDE) == 0 );
    CHECK( able_to_castle(board, Color::Black, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 0 );
    CHECK( board.castled[color_index(Color::Black)] == (CASTLE_QUEENSIDE | CASTLE_KINGSIDE) );

    undo_move (board, Color::Black, mv, undo_state);

    CHECK( able_to_castle(board, Color::Black, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle(board, Color::Black, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle(board, Color::Black, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::Black)] == CASTLE_NONE );
}

TEST_CASE("Castling state is modified and restored for castling queenside", "[castling]")
{
    std::vector<Piece> back_rank =
    {
        Piece::Rook,   Piece::None, Piece::None, Piece::None, Piece::King,
        Piece::Bishop, Piece::None, Piece::Rook
    };

    std::vector<board_positions> positions =
    {
        { 0, Color::Black, back_rank },
        { 7, Color::White, back_rank },
    };

    struct board board { positions };
    move_t mv = make_castling_move (0, 4, 0, 2);

    CHECK( able_to_castle(board, Color::Black, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle(board, Color::Black, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle(board, Color::Black, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::Black)] == CASTLE_NONE );

    undo_move_t undo_state = do_move (board, Color::Black, mv);;

    CHECK( able_to_castle(board, Color::Black, CASTLE_QUEENSIDE) == 0 );
    CHECK( able_to_castle(board, Color::Black, CASTLE_KINGSIDE) == 0 );
    CHECK( able_to_castle(board, Color::Black, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 0 );
    CHECK( board.castled[color_index(Color::Black)] == CASTLE_CASTLED );

    // Check rook and king position updated:
    CHECK( ROW(board.king_pos[color_index(Color::Black)]) == 0 );
    CHECK( COLUMN(board.king_pos[color_index(Color::Black)]) == 2 );
    CHECK(piece_type (piece_at (board, 0, 2)) == Piece::King );
    CHECK(piece_color (piece_at (board, 0, 2)) == Color::Black );
    CHECK(piece_type (piece_at (board, 0, 3)) == Piece::Rook );
    CHECK(piece_color (piece_at (board, 0, 3)) == Color::Black );

    undo_move (board, Color::Black, mv, undo_state);

    CHECK( able_to_castle(board, Color::Black, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle(board, Color::Black, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle(board, Color::Black, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );

    // check rook and king position restored:
    CHECK( ROW(board.king_pos[color_index(Color::Black)]) == 0 );
    CHECK( COLUMN(board.king_pos[color_index(Color::Black)]) == 4 );
    CHECK( board.castled[color_index(Color::Black)] == CASTLE_NONE );
    CHECK(piece_type (piece_at (board, 0, 4)) == Piece::King );
    CHECK(piece_color (piece_at (board, 0, 4)) == Color::Black );
    CHECK(piece_type (piece_at (board, 0, 0)) == Piece::Rook );
    CHECK(piece_color (piece_at (board, 0, 0)) == Color::Black );
}

TEST_CASE("Castling state is modified and restored for castling kingside", "[castling]")
{
    std::vector<Piece> back_rank =
    {
        Piece::Rook,  Piece::None, Piece::None, Piece::None, Piece::King,
        Piece::None, Piece::None, Piece::Rook
    };

    std::vector<board_positions> positions =
    {
        { 0, Color::Black, back_rank },
        { 7, Color::White, back_rank },
    };

    struct board board { positions };
    move_t mv = make_castling_move (7, 4, 7, 6);

    CHECK( able_to_castle(board, Color::White, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle(board, Color::White, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle(board, Color::White, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::White)] == CASTLE_NONE );

    undo_move_t undo_state = do_move (board, Color::White, mv);

    CHECK( able_to_castle(board, Color::White, CASTLE_QUEENSIDE) == 0 );
    CHECK( able_to_castle(board, Color::White, CASTLE_KINGSIDE) == 0 );
    CHECK( able_to_castle(board, Color::White, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 0 );
    CHECK( board.castled[color_index(Color::White)] == CASTLE_CASTLED );

    // Check rook and king position updated:
    CHECK( ROW(board.king_pos[color_index(Color::White)]) == 7 );
    CHECK( COLUMN(board.king_pos[color_index(Color::White)]) == 6 );
    CHECK(piece_type (piece_at (board, 7, 6)) == Piece::King );
    CHECK(piece_color (piece_at (board, 7, 6)) == Color::White );
    CHECK(piece_type (piece_at (board, 7, 5)) == Piece::Rook );
    CHECK(piece_color (piece_at (board, 7, 5)) == Color::White );

    undo_move (board, Color::White, mv, undo_state);

    CHECK( able_to_castle(board, Color::White, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle(board, Color::White, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle(board, Color::White, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );

    // check rook and king position restored:
    CHECK( ROW(board.king_pos[color_index(Color::White)]) == 7 );
    CHECK( COLUMN(board.king_pos[color_index(Color::White)]) == 4 );
    CHECK( board.castled[color_index(Color::White)] == CASTLE_NONE );
    CHECK(piece_type (piece_at (board, 7, 4)) == Piece::King );
    CHECK(piece_color (piece_at (board, 7, 4)) == Color::White );
    CHECK(piece_type (piece_at (board, 7, 7)) == Piece::Rook );
    CHECK(piece_color (piece_at (board, 7, 7)) == Color::White );
}

TEST_CASE("Opponent's castling state is modified when his rook is taken", "[castling]")
{
    board_builder builder;

    builder.add_piece("a8", Color::Black, Piece::Rook);
    builder.add_piece("e8", Color::Black, Piece::King);
    builder.add_piece("h8", Color::Black, Piece::Rook);
    builder.add_piece("a1", Color::White, Piece::Rook);
    builder.add_piece("e1", Color::White, Piece::King);
    builder.add_piece("h1", Color::White, Piece::Rook);

    // add bishop to capture rook:
    builder.add_piece("b7", Color::White, Piece::Bishop);

    struct board board = builder.build();
    
    move_t mv = make_capturing_move (1, 1, 0, 0);

    CHECK( able_to_castle (board, Color::White, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, Color::White, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::White, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::White)] == CASTLE_NONE );

    CHECK( able_to_castle (board, Color::Black, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, Color::Black, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::Black, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::Black)] == CASTLE_NONE );

    undo_move_t undo_state = do_move (board, Color::White, mv);

    CHECK( able_to_castle (board, Color::White, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, Color::White, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::White, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::White)] == CASTLE_NONE );

    CHECK( able_to_castle (board, Color::Black, CASTLE_QUEENSIDE) == 0 );
    CHECK( able_to_castle (board, Color::Black, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::Black, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::Black)] == CASTLE_QUEENSIDE );

    undo_move (board, Color::White, mv, undo_state);

    CHECK( able_to_castle (board, Color::White, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, Color::White, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::White, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::White)] == CASTLE_NONE );

    CHECK( able_to_castle (board, Color::Black, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, Color::Black, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::Black, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::Black)] == CASTLE_NONE );
}

TEST_CASE("Castling state is updated when rook captures a piece", "[castling]")
{
    board_builder builder;

    builder.add_piece("a8", Color::Black, Piece::Rook);
    builder.add_piece("e8", Color::Black, Piece::King);
    builder.add_piece("h8", Color::Black, Piece::Rook);
    builder.add_piece("a1", Color::White, Piece::Rook);
    builder.add_piece("e1", Color::White, Piece::King);
    builder.add_piece("h1", Color::White, Piece::Rook);

    // add bishop for rook to capture:
    builder.add_piece("a7", Color::White, Piece::Bishop);

    struct board board = builder.build();

    move_t mv = make_capturing_move (0, 0, 1, 0);

    CHECK( able_to_castle (board, Color::White, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, Color::White, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::White, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::White)] == CASTLE_NONE );

    CHECK( able_to_castle (board, Color::Black, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, Color::Black, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::Black, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::Black)] == CASTLE_NONE );

    undo_move_t undo_state = do_move (board, Color::Black, mv);;

    CHECK( able_to_castle (board, Color::White, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, Color::White, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::White, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::White)] == CASTLE_NONE );

    CHECK( able_to_castle (board, Color::Black, CASTLE_QUEENSIDE) == 0 );
    CHECK( able_to_castle (board, Color::Black, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::Black, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::Black)] == CASTLE_QUEENSIDE );

    undo_move (board, Color::Black, mv, undo_state);

    CHECK( able_to_castle (board, Color::White, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, Color::White, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::White, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::White)] == CASTLE_NONE );

    CHECK( able_to_castle (board, Color::Black, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, Color::Black, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::Black, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::Black)] == CASTLE_NONE );
}

TEST_CASE("Opponent's castling state is modified when his rook is taken (failure scenario)", "[castling]")
{
    board_builder builder;

    builder.add_row_of_same_color (0, Color::Black, {
        Piece::Rook, Piece::None, Piece::Queen, Piece::None, Piece::King,
        Piece::Bishop, Piece::Knight, Piece::Rook
    });
    builder.add_row_of_same_color (1, Color::Black, {
        Piece::Pawn, Piece::None, Piece::Pawn, Piece::Pawn, Piece::Pawn,
        Piece::Pawn, Piece::Pawn, Piece::Pawn
    });
    builder.add_row_of_same_color (6, Color::White, {
        Piece::Pawn, Piece::Pawn, Piece::Pawn, Piece::None, Piece::None,
        Piece::Pawn, Piece::Pawn, Piece::Pawn
    });
    builder.add_row_of_same_color (7, Color::White, {
        Piece::Rook, Piece::Knight, Piece::Bishop, Piece::None, Piece::King,
        Piece::None, Piece::Knight, Piece::Rook
    });

    builder.add_piece ("a6", Color::Black, Piece::Pawn);
    builder.add_piece ("e5", Color::Black, Piece::Bishop);
    builder.add_piece ("d3", Color::White, Piece::Pawn);
    // add the queen ready for rook to capture:
    builder.add_piece ("b8", Color::White, Piece::Queen);

    struct board board = builder.build();
    move_t mv = make_capturing_move (0, 0, 0, 1);

    CHECK( able_to_castle (board, Color::White, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, Color::White, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::White, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::White)] == CASTLE_NONE );

    CHECK( able_to_castle (board, Color::Black, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, Color::Black, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::Black, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::Black)] == CASTLE_NONE );

    undo_move_t undo_state = do_move (board, Color::Black, mv);;

    CHECK( able_to_castle (board, Color::White, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, Color::White, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::White, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::White)] == CASTLE_NONE );

    CHECK( able_to_castle (board, Color::Black, CASTLE_QUEENSIDE) == 0 );
    CHECK( able_to_castle (board, Color::Black, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::Black, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::Black)] == CASTLE_QUEENSIDE );

    undo_move (board, Color::Black, mv, undo_state);

    CHECK( able_to_castle (board, Color::White, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, Color::White, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::White, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::White)] == CASTLE_NONE );

    CHECK( able_to_castle (board, Color::Black, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, Color::Black, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::Black, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::Black)] == CASTLE_NONE );
}

TEST_CASE("Castling state is modified when rook takes a piece on same column (scenario 2)", "[castling]")
{
    board_builder builder;

    builder.add_row_of_same_color (0, Color::Black, {
            Piece::None, Piece::None, Piece::Bishop, Piece::Queen, Piece::King,
            Piece::Bishop, Piece::Knight, Piece::Rook
    });
    builder.add_row_of_same_color (1, Color::Black, {
            Piece::Pawn, Piece::None, Piece::Pawn, Piece::Pawn, Piece::Pawn,
            Piece::Pawn, Piece::Pawn, Piece::Pawn
    });
    builder.add_row_of_same_color (6, Color::White, {
            Piece::None, Piece::None, Piece::Pawn, Piece::Pawn, Piece::None,
            Piece::Pawn, Piece::Pawn, Piece::Pawn
    });
    builder.add_row_of_same_color (7, Color::White, {
            Piece::Rook, Piece::Knight, Piece::Bishop, Piece::Queen, Piece::King,
            Piece::None, Piece::Knight, Piece::Rook
    });

    builder.add_piece ("e6", Color::White, Piece::Pawn);

    // Rook white will capture:
    builder.add_piece ("a2", Color::Black, Piece::Rook);

    struct board board = builder.build();

    move_t mv = make_capturing_move (7, 0, 6, 0);

    CHECK( able_to_castle (board, Color::White, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, Color::White, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::White, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::White)] == CASTLE_NONE );

    CHECK( able_to_castle (board, Color::Black, CASTLE_QUEENSIDE) == 0 );
    CHECK( able_to_castle (board, Color::Black, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::Black, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::Black)] == CASTLE_QUEENSIDE );

    undo_move_t undo_state = do_move (board, Color::White, mv);

    CHECK( able_to_castle (board, Color::White, CASTLE_QUEENSIDE) == 0 );
    CHECK( able_to_castle (board, Color::White, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::White, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::White)] == CASTLE_QUEENSIDE );

    CHECK( able_to_castle (board, Color::Black, CASTLE_QUEENSIDE) == 0 );
    CHECK( able_to_castle (board, Color::Black, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::Black, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::Black)] == CASTLE_QUEENSIDE );

    undo_move (board, Color::White, mv, undo_state);

    CHECK( able_to_castle (board, Color::White, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, Color::White, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::White, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::White)] == CASTLE_NONE );

    CHECK( able_to_castle (board, Color::Black, CASTLE_QUEENSIDE) == 0 );
    CHECK( able_to_castle (board, Color::Black, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, Color::Black, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board.castled[color_index(Color::Black)] == CASTLE_QUEENSIDE );
}
