#include "catch.hpp"
#include "board_builder.hpp"

extern "C"
{
#include "../src/board.h"
#include "../src/board_positions.h"
}

TEST_CASE("Castling state is modified and restored for rooks", "[castling]")
{
    enum piece_type back_rank[] =
    {
        PIECE_ROOK,   PIECE_NONE, PIECE_BISHOP, PIECE_QUEEN, PIECE_KING,
        PIECE_BISHOP, PIECE_NONE, PIECE_ROOK, PIECE_LAST
    };

    struct board_positions positions[] =
    {
        { 0, COLOR_BLACK, back_rank },
        { 7, COLOR_WHITE, back_rank },
        { 0, COLOR_NONE, nullptr }
    };

    struct board *board = board_from_positions (positions);
    move_t mv = move_create (0, 0, 0, 1);

    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_BLACK, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_BLACK)] == CASTLE_NONE );

    do_move(board, COLOR_BLACK, &mv);

    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_QUEENSIDE) == 0 );
    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_BLACK, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_BLACK)] == CASTLE_QUEENSIDE );

    undo_move(board, COLOR_BLACK, &mv);

    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_BLACK, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_BLACK)] == CASTLE_NONE );
}

TEST_CASE("Castling state is modified and restored for kings", "[castling]")
{
    enum piece_type back_rank[] =
    {
        PIECE_ROOK,   PIECE_NONE, PIECE_BISHOP, PIECE_NONE, PIECE_KING,
        PIECE_BISHOP, PIECE_NONE, PIECE_ROOK, PIECE_LAST
    };

    struct board_positions positions[] =
    {
        { 0, COLOR_BLACK, back_rank },
        { 7, COLOR_WHITE, back_rank },
        { 0, COLOR_NONE, nullptr }
    };

    struct board *board = board_from_positions (positions);

    move_t mv = move_create (0, 4, 0, 3);

    CHECK( able_to_castle(board, COLOR_BLACK, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle(board, COLOR_BLACK, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle(board, COLOR_BLACK, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_BLACK)] == CASTLE_NONE );

    do_move (board, COLOR_BLACK, &mv);

    CHECK( able_to_castle(board, COLOR_BLACK, CASTLE_QUEENSIDE) == 0 );
    CHECK( able_to_castle(board, COLOR_BLACK, CASTLE_KINGSIDE) == 0 );
    CHECK( able_to_castle(board, COLOR_BLACK, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 0 );
    CHECK( board->castled[color_index(COLOR_BLACK)] == (CASTLE_QUEENSIDE | CASTLE_KINGSIDE) );

    undo_move(board, COLOR_BLACK, &mv);

    CHECK( able_to_castle(board, COLOR_BLACK, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle(board, COLOR_BLACK, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle(board, COLOR_BLACK, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_BLACK)] == CASTLE_NONE );
}

TEST_CASE("Castling state is modified and restored for castling queenside", "[castling]")
{
    enum piece_type back_rank[] =
    {
        PIECE_ROOK,   PIECE_NONE, PIECE_NONE, PIECE_NONE, PIECE_KING,
        PIECE_BISHOP, PIECE_NONE, PIECE_ROOK, PIECE_LAST
    };

    struct board_positions positions[] =
    {
        { 0, COLOR_BLACK, back_rank },
        { 7, COLOR_WHITE, back_rank },
        { 0, COLOR_NONE, nullptr }
    };

    struct board *board = board_from_positions (positions);

    move_t mv = move_create (0, 4, 0, 2);
    move_set_castling (&mv);

    CHECK( able_to_castle(board, COLOR_BLACK, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle(board, COLOR_BLACK, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle(board, COLOR_BLACK, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_BLACK)] == CASTLE_NONE );

    do_move(board, COLOR_BLACK, &mv);

    CHECK( able_to_castle(board, COLOR_BLACK, CASTLE_QUEENSIDE) == 0 );
    CHECK( able_to_castle(board, COLOR_BLACK, CASTLE_KINGSIDE) == 0 );
    CHECK( able_to_castle(board, COLOR_BLACK, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 0 );
    CHECK( board->castled[color_index(COLOR_BLACK)] == CASTLE_CASTLED );

    // Check rook and king position updated:
    CHECK( ROW(board->king_pos[color_index(COLOR_BLACK)]) == 0 );
    CHECK( COLUMN(board->king_pos[color_index(COLOR_BLACK)]) == 2 );
    CHECK( PIECE_TYPE(PIECE_AT(board, 0, 2)) == PIECE_KING );
    CHECK( PIECE_COLOR(PIECE_AT(board, 0, 2)) == COLOR_BLACK );
    CHECK( PIECE_TYPE(PIECE_AT(board, 0, 3)) == PIECE_ROOK );
    CHECK( PIECE_COLOR(PIECE_AT(board, 0, 3)) == COLOR_BLACK );

    undo_move(board, COLOR_BLACK, &mv);

    CHECK( able_to_castle(board, COLOR_BLACK, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle(board, COLOR_BLACK, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle(board, COLOR_BLACK, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );

    // check rook and king position restored:
    CHECK( ROW(board->king_pos[color_index(COLOR_BLACK)]) == 0 );
    CHECK( COLUMN(board->king_pos[color_index(COLOR_BLACK)]) == 4 );
    CHECK( board->castled[color_index(COLOR_BLACK)] == CASTLE_NONE );
    CHECK( PIECE_TYPE(PIECE_AT(board, 0, 4)) == PIECE_KING );
    CHECK( PIECE_COLOR(PIECE_AT(board, 0, 4)) == COLOR_BLACK );
    CHECK( PIECE_TYPE(PIECE_AT(board, 0, 0)) == PIECE_ROOK );
    CHECK( PIECE_COLOR(PIECE_AT(board, 0, 0)) == COLOR_BLACK );
}

TEST_CASE("Castling state is modified and restored for castling kingside", "[castling]")
{
    enum piece_type back_rank[] =
    {
        PIECE_ROOK,   PIECE_NONE, PIECE_NONE, PIECE_NONE, PIECE_KING,
        PIECE_BISHOP, PIECE_NONE, PIECE_ROOK, PIECE_LAST
    };

    struct board_positions positions[] =
    {
        { 0, COLOR_BLACK, back_rank },
        { 7, COLOR_WHITE, back_rank },
        { 0, COLOR_NONE, nullptr }
    };

    struct board *board = board_from_positions (positions);

    move_t mv = move_create (7, 4, 7, 6);
    move_set_castling (&mv);

    CHECK( able_to_castle(board, COLOR_WHITE, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle(board, COLOR_WHITE, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle(board, COLOR_WHITE, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_WHITE)] == CASTLE_NONE );

    do_move(board, COLOR_WHITE, &mv);

    CHECK( able_to_castle(board, COLOR_WHITE, CASTLE_QUEENSIDE) == 0 );
    CHECK( able_to_castle(board, COLOR_WHITE, CASTLE_KINGSIDE) == 0 );
    CHECK( able_to_castle(board, COLOR_WHITE, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 0 );
    CHECK( board->castled[color_index(COLOR_WHITE)] == CASTLE_CASTLED );

    // Check rook and king position updated:
    CHECK( ROW(board->king_pos[color_index(COLOR_WHITE)]) == 7 );
    CHECK( COLUMN(board->king_pos[color_index(COLOR_WHITE)]) == 6 );
    CHECK( PIECE_TYPE(PIECE_AT(board, 7, 6)) == PIECE_KING );
    CHECK( PIECE_COLOR(PIECE_AT(board, 7, 6)) == COLOR_WHITE );
    CHECK( PIECE_TYPE(PIECE_AT(board, 7, 5)) == PIECE_ROOK );
    CHECK( PIECE_COLOR(PIECE_AT(board, 7, 5)) == COLOR_WHITE );

    undo_move(board, COLOR_WHITE, &mv);

    CHECK( able_to_castle(board, COLOR_WHITE, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle(board, COLOR_WHITE, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle(board, COLOR_WHITE, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );

    // check rook and king position restored:
    CHECK( ROW(board->king_pos[color_index(COLOR_WHITE)]) == 7 );
    CHECK( COLUMN(board->king_pos[color_index(COLOR_WHITE)]) == 4 );
    CHECK( board->castled[color_index(COLOR_WHITE)] == CASTLE_NONE );
    CHECK( PIECE_TYPE(PIECE_AT(board, 7, 4)) == PIECE_KING );
    CHECK( PIECE_COLOR(PIECE_AT(board, 7, 4)) == COLOR_WHITE );
    CHECK( PIECE_TYPE(PIECE_AT(board, 7, 7)) == PIECE_ROOK );
    CHECK( PIECE_COLOR(PIECE_AT(board, 7, 7)) == COLOR_WHITE );
}

TEST_CASE("Opponent's castling state is modified when his rook is taken", "[castling]")
{
    board_builder builder;

    builder.add_piece("a8", COLOR_BLACK, PIECE_ROOK);
    builder.add_piece("e8", COLOR_BLACK, PIECE_KING);
    builder.add_piece("h8", COLOR_BLACK, PIECE_ROOK);
    builder.add_piece("a1", COLOR_WHITE, PIECE_ROOK);
    builder.add_piece("e1", COLOR_WHITE, PIECE_KING);
    builder.add_piece("h1", COLOR_WHITE, PIECE_ROOK);

    // add bishop to capture rook:
    builder.add_piece("b7", COLOR_WHITE, PIECE_BISHOP);

    struct board *board = builder.build();
    move_t mv = move_create (1, 1, 0, 0);

    CHECK( able_to_castle (board, COLOR_WHITE, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_WHITE, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_WHITE, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_WHITE)] == CASTLE_NONE );

    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_BLACK, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_BLACK)] == CASTLE_NONE );

    do_move (board, COLOR_WHITE, &mv);

    CHECK( able_to_castle (board, COLOR_WHITE, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_WHITE, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_WHITE, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_WHITE)] == CASTLE_NONE );

    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_QUEENSIDE) == 0 );
    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_BLACK, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_BLACK)] == CASTLE_QUEENSIDE );

    undo_move (board, COLOR_WHITE, &mv);

    CHECK( able_to_castle (board, COLOR_WHITE, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_WHITE, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_WHITE, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_WHITE)] == CASTLE_NONE );

    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_BLACK, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_BLACK)] == CASTLE_NONE );
}

TEST_CASE("Castling state is updated when rook captures a piece", "[castling]")
{
    board_builder builder;

    builder.add_piece("a8", COLOR_BLACK, PIECE_ROOK);
    builder.add_piece("e8", COLOR_BLACK, PIECE_KING);
    builder.add_piece("h8", COLOR_BLACK, PIECE_ROOK);
    builder.add_piece("a1", COLOR_WHITE, PIECE_ROOK);
    builder.add_piece("e1", COLOR_WHITE, PIECE_KING);
    builder.add_piece("h1", COLOR_WHITE, PIECE_ROOK);

    // add bishop for rook to capture:
    builder.add_piece("a7", COLOR_WHITE, PIECE_BISHOP);

    struct board *board = builder.build();
    move_t mv = move_create (0, 0, 1, 0);

    CHECK( able_to_castle (board, COLOR_WHITE, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_WHITE, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_WHITE, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_WHITE)] == CASTLE_NONE );

    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_BLACK, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_BLACK)] == CASTLE_NONE );

    do_move (board, COLOR_BLACK, &mv);

    CHECK( able_to_castle (board, COLOR_WHITE, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_WHITE, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_WHITE, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_WHITE)] == CASTLE_NONE );

    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_QUEENSIDE) == 0 );
    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_BLACK, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_BLACK)] == CASTLE_QUEENSIDE );

    undo_move (board, COLOR_BLACK, &mv);

    CHECK( able_to_castle (board, COLOR_WHITE, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_WHITE, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_WHITE, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_WHITE)] == CASTLE_NONE );

    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_BLACK, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_BLACK)] == CASTLE_NONE );
}

TEST_CASE("Opponent's castling state is modified when his rook is taken (failure scenario)", "[castling]")
{
    board_builder builder;

    builder.add_row_of_same_color (0, COLOR_BLACK, {
        PIECE_ROOK, PIECE_NONE, PIECE_QUEEN, PIECE_NONE, PIECE_KING,
        PIECE_BISHOP, PIECE_KNIGHT, PIECE_ROOK
    });
    builder.add_row_of_same_color (1, COLOR_BLACK, {
        PIECE_PAWN, PIECE_NONE, PIECE_PAWN, PIECE_PAWN, PIECE_PAWN,
        PIECE_PAWN, PIECE_PAWN, PIECE_PAWN
    });
    builder.add_row_of_same_color (6, COLOR_WHITE, {
        PIECE_PAWN, PIECE_PAWN, PIECE_PAWN, PIECE_NONE, PIECE_NONE,
        PIECE_PAWN, PIECE_PAWN, PIECE_PAWN
    });
    builder.add_row_of_same_color (7, COLOR_WHITE, {
        PIECE_ROOK, PIECE_KNIGHT, PIECE_BISHOP, PIECE_NONE, PIECE_KING,
        PIECE_NONE, PIECE_KNIGHT, PIECE_ROOK
    });

    builder.add_piece ("a6", COLOR_BLACK, PIECE_PAWN);
    builder.add_piece ("e5", COLOR_BLACK, PIECE_BISHOP);
    builder.add_piece ("d3", COLOR_WHITE, PIECE_PAWN);
    // add the queen ready for rook to capture:
    builder.add_piece ("b8", COLOR_WHITE, PIECE_QUEEN);

    struct board *board = builder.build();

    move_t mv = move_create (0, 0, 0, 1);

    CHECK( able_to_castle (board, COLOR_WHITE, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_WHITE, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_WHITE, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_WHITE)] == CASTLE_NONE );

    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_BLACK, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_BLACK)] == CASTLE_NONE );

    do_move (board, COLOR_BLACK, &mv);

    CHECK( able_to_castle (board, COLOR_WHITE, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_WHITE, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_WHITE, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_WHITE)] == CASTLE_NONE );

    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_QUEENSIDE) == 0 );
    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_BLACK, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_BLACK)] == CASTLE_QUEENSIDE );

    undo_move (board, COLOR_BLACK, &mv);

    CHECK( able_to_castle (board, COLOR_WHITE, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_WHITE, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_WHITE, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_WHITE)] == CASTLE_NONE );

    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_BLACK, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_BLACK)] == CASTLE_NONE );
}

TEST_CASE("Castling state is modified when rook takes a piece on same column (scenario 2)", "[castling]")
{
    board_builder builder;

    builder.add_row_of_same_color (0, COLOR_BLACK, {
            PIECE_NONE, PIECE_NONE, PIECE_BISHOP, PIECE_QUEEN, PIECE_KING,
            PIECE_BISHOP, PIECE_KNIGHT, PIECE_ROOK
    });
    builder.add_row_of_same_color (1, COLOR_BLACK, {
            PIECE_PAWN, PIECE_NONE, PIECE_PAWN, PIECE_PAWN, PIECE_PAWN,
            PIECE_PAWN, PIECE_PAWN, PIECE_PAWN
    });
    builder.add_row_of_same_color (6, COLOR_WHITE, {
            PIECE_NONE, PIECE_NONE, PIECE_PAWN, PIECE_PAWN, PIECE_NONE,
            PIECE_PAWN, PIECE_PAWN, PIECE_PAWN
    });
    builder.add_row_of_same_color (7, COLOR_WHITE, {
            PIECE_ROOK, PIECE_KNIGHT, PIECE_BISHOP, PIECE_QUEEN, PIECE_KING,
            PIECE_NONE, PIECE_KNIGHT, PIECE_ROOK
    });

    builder.add_piece ("e6", COLOR_WHITE, PIECE_PAWN);

    // Rook white will capture:
    builder.add_piece ("a2", COLOR_BLACK, PIECE_ROOK);

    struct board *board = builder.build();
    move_t mv = move_create (7, 0, 6, 0);

    CHECK( able_to_castle (board, COLOR_WHITE, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_WHITE, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_WHITE, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_WHITE)] == CASTLE_NONE );

    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_QUEENSIDE) == 0 );
    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_BLACK, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_BLACK)] == CASTLE_QUEENSIDE );

    do_move (board, COLOR_WHITE, &mv);

    CHECK( able_to_castle (board, COLOR_WHITE, CASTLE_QUEENSIDE) == 0 );
    CHECK( able_to_castle (board, COLOR_WHITE, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_WHITE, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_WHITE)] == CASTLE_QUEENSIDE );

    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_QUEENSIDE) == 0 );
    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_BLACK, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_BLACK)] == CASTLE_QUEENSIDE );

    undo_move (board, COLOR_WHITE, &mv);

    CHECK( able_to_castle (board, COLOR_WHITE, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_WHITE, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_WHITE, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_WHITE)] == CASTLE_NONE );

    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_QUEENSIDE) == 0 );
    CHECK( able_to_castle (board, COLOR_BLACK, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle (board, COLOR_BLACK, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_BLACK)] == CASTLE_QUEENSIDE );
}
