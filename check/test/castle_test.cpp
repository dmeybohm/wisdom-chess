#include "catch.hpp"

extern "C"
{
#include "../src/board.h"
}

TEST_CASE("Castling state is modified and restored for rooks", "[castling]")
{
    struct board *board = board_new();
    enum piece_type back_rank[] =
    {
        PIECE_ROOK,   PIECE_NONE, PIECE_BISHOP, PIECE_QUEEN, PIECE_KING,
        PIECE_BISHOP, PIECE_NONE, PIECE_ROOK, PIECE_LAST
    };

    struct board_positions positions[] =
    {
        { 0, COLOR_BLACK, back_rank },
        { 7, COLOR_WHITE, back_rank },
        { 0, PIECE_NONE, NULL }
    };

    board_init_from_positions(board, positions);
    move_t mv = move_create (0, 0, 0, 1);

    CHECK( able_to_castle(board, COLOR_BLACK, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle(board, COLOR_BLACK, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle(board, COLOR_BLACK, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_BLACK)] == CASTLE_NONE );

    do_move(board, COLOR_BLACK, &mv);

    CHECK( able_to_castle(board, COLOR_BLACK, CASTLE_QUEENSIDE) == 0 );
    CHECK( able_to_castle(board, COLOR_BLACK, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle(board, COLOR_BLACK, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_BLACK)] == CASTLE_QUEENSIDE );

    undo_move(board, COLOR_BLACK, &mv);

    CHECK( able_to_castle(board, COLOR_BLACK, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle(board, COLOR_BLACK, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle(board, COLOR_BLACK, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_BLACK)] == CASTLE_NONE );
}

TEST_CASE("Castling state is modified and restored for kings", "[castling]")
{
    struct board *board = board_new();
    enum piece_type back_rank[] =
    {
        PIECE_ROOK,   PIECE_NONE, PIECE_BISHOP, PIECE_NONE, PIECE_KING,
        PIECE_BISHOP, PIECE_NONE, PIECE_ROOK, PIECE_LAST
    };

    struct board_positions positions[] =
    {
        { 0, COLOR_BLACK, back_rank },
        { 7, COLOR_WHITE, back_rank },
        { 0, PIECE_NONE, NULL }
    };

    board_init_from_positions(board, positions);

    move_t mv = move_create (0, 4, 0, 3);

    CHECK( able_to_castle(board, COLOR_BLACK, CASTLE_QUEENSIDE) == 1 );
    CHECK( able_to_castle(board, COLOR_BLACK, CASTLE_KINGSIDE) == 1 );
    CHECK( able_to_castle(board, COLOR_BLACK, (CASTLE_KINGSIDE|CASTLE_KINGSIDE)) == 1 );
    CHECK( board->castled[color_index(COLOR_BLACK)] == CASTLE_NONE );

    do_move(board, COLOR_BLACK, &mv);

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
    struct board *board = board_new();
    enum piece_type back_rank[] =
    {
        PIECE_ROOK,   PIECE_NONE, PIECE_NONE, PIECE_NONE, PIECE_KING,
        PIECE_BISHOP, PIECE_NONE, PIECE_ROOK, PIECE_LAST
    };

    struct board_positions positions[] =
    {
        { 0, COLOR_BLACK, back_rank },
        { 7, COLOR_WHITE, back_rank },
        { 0, PIECE_NONE, NULL }
    };

    board_init_from_positions(board, positions);

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
    struct board *board = board_new();
    enum piece_type back_rank[] =
    {
        PIECE_ROOK,   PIECE_NONE, PIECE_NONE, PIECE_NONE, PIECE_KING,
        PIECE_BISHOP, PIECE_NONE, PIECE_ROOK, PIECE_LAST
    };

    struct board_positions positions[] =
    {
        { 0, COLOR_BLACK, back_rank },
        { 7, COLOR_WHITE, back_rank },
        { 0, PIECE_NONE, NULL }
    };

    board_init_from_positions(board, positions);

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
    struct board *board = board_new();
    enum piece_type black_back_rank[] =
    {
        PIECE_ROOK,   PIECE_NONE, PIECE_NONE, PIECE_NONE, PIECE_KING,
        PIECE_NONE, PIECE_NONE, PIECE_ROOK, PIECE_LAST
    };
    enum piece_type white_bishop_b7[] =
    {
        PIECE_NONE, PIECE_BISHOP, PIECE_LAST
    };
    enum piece_type white_back_rank[] =
    {
        PIECE_NONE, PIECE_NONE, PIECE_NONE, PIECE_NONE, PIECE_KING,
        PIECE_NONE, PIECE_NONE, PIECE_NONE, PIECE_LAST
    };

    struct board_positions positions[] =
    {
        { 0, COLOR_BLACK, black_back_rank },
        { 1, COLOR_WHITE, white_bishop_b7 },
        { 7, COLOR_WHITE, white_back_rank },
        { 0, PIECE_NONE, NULL }
    };

    board_init_from_positions (board, positions);

    move_t mv = move_create (1, 1, 0, 0);

    // TODO: need to update the castle state on initialization if the white rooks are missing from
    // starting positions:
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
    struct board *board = board_new();
    enum piece_type black_back_rank[] =
    {
        PIECE_ROOK, PIECE_NONE, PIECE_NONE, PIECE_NONE, PIECE_KING,
        PIECE_NONE, PIECE_NONE, PIECE_ROOK, PIECE_LAST
    };
    enum piece_type white_bishop_b8[] =
    {
        PIECE_BISHOP, PIECE_NONE, PIECE_LAST
    };
    enum piece_type white_back_rank[] =
    {
        PIECE_NONE, PIECE_NONE, PIECE_NONE, PIECE_NONE, PIECE_KING,
        PIECE_NONE, PIECE_NONE, PIECE_NONE, PIECE_LAST
    };

    struct board_positions positions[] =
    {
        { 0, COLOR_BLACK, black_back_rank },
        { 1, COLOR_WHITE, white_bishop_b8 },
        { 7, COLOR_WHITE, white_back_rank },
        { 0, PIECE_NONE, NULL }
    };

    board_init_from_positions (board, positions);

    move_t mv = move_create (0, 0, 1, 0);

    // TODO: need to update the castle state on initialization if the white rooks are missing from
    // starting positions:
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