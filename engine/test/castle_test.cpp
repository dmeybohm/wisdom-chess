#include <doctest/doctest.h>
#include "board_builder.hpp"

#include "board.hpp"

using namespace wisdom;

TEST_CASE("Castling state is modified and restored for rooks")
{
    std::vector<Piece> back_rank =
    {
        Piece::Rook,   Piece::None, Piece::Bishop, Piece::Queen, Piece::King,
        Piece::Bishop, Piece::None, Piece::Rook
    };

    std::vector<BoardPositions> positions =
    {
        { 0, Color::Black, back_rank },
        { 7, Color::White, back_rank },
    };

    Board board { positions };
    Move mv = make_move (0, 0, 0, 1);

    CHECK(able_to_castle (board, Color::Black, Castle_Queenside) == 1);
    CHECK(able_to_castle (board, Color::Black, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::Black, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::Black)] == Castle_None);

    UndoMove undo_state = do_move (board, Color::Black, mv);

    CHECK(able_to_castle (board, Color::Black, Castle_Queenside) == 0);
    CHECK(able_to_castle (board, Color::Black, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::Black, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::Black)] == Castle_Queenside);

    undo_move (board, Color::Black, mv, undo_state);

    CHECK(able_to_castle (board, Color::Black, Castle_Queenside) == 1);
    CHECK(able_to_castle (board, Color::Black, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::Black, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::Black)] == Castle_None);
}

TEST_CASE("Castling state is modified and restored for kings")
{
    std::vector<Piece> back_rank =
    {
        Piece::Rook,   Piece::None, Piece::Bishop, Piece::None, Piece::King,
        Piece::Bishop, Piece::None, Piece::Rook
    };

    std::vector<BoardPositions> positions =
    {
        { 0, Color::Black, back_rank },
        { 7, Color::White, back_rank },
    };

    Board board {positions };
    Move mv = make_move (0, 4, 0, 3);

    CHECK(able_to_castle (board, Color::Black, Castle_Queenside) == 1);
    CHECK(able_to_castle (board, Color::Black, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::Black, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::Black)] == Castle_None);

    UndoMove undo_state = do_move (board, Color::Black, mv);;

    CHECK(able_to_castle (board, Color::Black, Castle_Queenside) == 0);
    CHECK(able_to_castle (board, Color::Black, Castle_Kingside) == 0);
    CHECK(able_to_castle (board, Color::Black, (Castle_Kingside | Castle_Kingside)) == 0);
    CHECK( board.castled[color_index(Color::Black)] == (Castle_Queenside | Castle_Kingside));

    undo_move (board, Color::Black, mv, undo_state);

    CHECK(able_to_castle (board, Color::Black, Castle_Queenside) == 1);
    CHECK(able_to_castle (board, Color::Black, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::Black, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::Black)] == Castle_None);
}

TEST_CASE("Castling state is modified and restored for castling queenside")
{
    std::vector<Piece> back_rank = {
        Piece::Rook,   Piece::None, Piece::None, Piece::None, Piece::King,
        Piece::Bishop, Piece::None, Piece::Rook
    };

    std::vector<BoardPositions> positions = {
        { 0, Color::Black, back_rank },
        { 7, Color::White, back_rank },
    };

    Board board {positions };
    Move mv = make_castling_move (0, 4, 0, 2);

    CHECK(able_to_castle (board, Color::Black, Castle_Queenside) == 1);
    CHECK(able_to_castle (board, Color::Black, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::Black, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::Black)] == Castle_None);

    UndoMove undo_state = do_move (board, Color::Black, mv);;

    CHECK(able_to_castle (board, Color::Black, Castle_Queenside) == 0);
    CHECK(able_to_castle (board, Color::Black, Castle_Kingside) == 0);
    CHECK(able_to_castle (board, Color::Black, (Castle_Kingside | Castle_Kingside)) == 0);
    CHECK(board.castled[color_index(Color::Black)] == Castle_Castled);

    // Check rook and king position updated:
    CHECK( ROW(board.king_pos[color_index(Color::Black)]) == 0 );
    CHECK( COLUMN(board.king_pos[color_index(Color::Black)]) == 2 );
    CHECK(piece_type (piece_at (board, 0, 2)) == Piece::King );
    CHECK(piece_color (piece_at (board, 0, 2)) == Color::Black );
    CHECK(piece_type (piece_at (board, 0, 3)) == Piece::Rook );
    CHECK(piece_color (piece_at (board, 0, 3)) == Color::Black );

    undo_move (board, Color::Black, mv, undo_state);

    CHECK(able_to_castle (board, Color::Black, Castle_Queenside) == 1);
    CHECK(able_to_castle (board, Color::Black, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::Black, (Castle_Kingside | Castle_Kingside)) == 1);

    // check rook and king position restored:
    CHECK( ROW(board.king_pos[color_index(Color::Black)]) == 0 );
    CHECK( COLUMN(board.king_pos[color_index(Color::Black)]) == 4 );
    CHECK(board.castled[color_index(Color::Black)] == Castle_None);
    CHECK(piece_type (piece_at (board, 0, 4)) == Piece::King );
    CHECK(piece_color (piece_at (board, 0, 4)) == Color::Black );
    CHECK(piece_type (piece_at (board, 0, 0)) == Piece::Rook );
    CHECK(piece_color (piece_at (board, 0, 0)) == Color::Black );
}

TEST_CASE("Castling state is modified and restored for castling kingside")
{
    std::vector<Piece> back_rank = {
        Piece::Rook,  Piece::None, Piece::None, Piece::None, Piece::King,
        Piece::None, Piece::None, Piece::Rook
    };

    std::vector<BoardPositions> positions = {
        { 0, Color::Black, back_rank },
        { 7, Color::White, back_rank },
    };

    Board board {positions };
    Move mv = make_castling_move (7, 4, 7, 6);

    CHECK(able_to_castle (board, Color::White, Castle_Queenside) == 1);
    CHECK(able_to_castle (board, Color::White, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::White, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::White)] == Castle_None);

    UndoMove undo_state = do_move (board, Color::White, mv);

    CHECK(able_to_castle (board, Color::White, Castle_Queenside) == 0);
    CHECK(able_to_castle (board, Color::White, Castle_Kingside) == 0);
    CHECK(able_to_castle (board, Color::White, (Castle_Kingside | Castle_Kingside)) == 0);
    CHECK(board.castled[color_index(Color::White)] == Castle_Castled);

    // Check rook and king position updated:
    CHECK( ROW(board.king_pos[color_index(Color::White)]) == 7 );
    CHECK( COLUMN(board.king_pos[color_index(Color::White)]) == 6 );
    CHECK(piece_type (piece_at (board, 7, 6)) == Piece::King );
    CHECK(piece_color (piece_at (board, 7, 6)) == Color::White );
    CHECK(piece_type (piece_at (board, 7, 5)) == Piece::Rook );
    CHECK(piece_color (piece_at (board, 7, 5)) == Color::White );

    undo_move (board, Color::White, mv, undo_state);

    CHECK(able_to_castle (board, Color::White, Castle_Queenside) == 1);
    CHECK(able_to_castle (board, Color::White, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::White, (Castle_Kingside | Castle_Kingside)) == 1);

    // check rook and king position restored:
    CHECK( ROW(board.king_pos[color_index(Color::White)]) == 7 );
    CHECK( COLUMN(board.king_pos[color_index(Color::White)]) == 4 );
    CHECK(board.castled[color_index(Color::White)] == Castle_None);
    CHECK(piece_type (piece_at (board, 7, 4)) == Piece::King );
    CHECK(piece_color (piece_at (board, 7, 4)) == Color::White );
    CHECK(piece_type (piece_at (board, 7, 7)) == Piece::Rook );
    CHECK(piece_color (piece_at (board, 7, 7)) == Color::White );
}

TEST_CASE("Opponent's castling state is modified when his rook is taken")
{
    BoardBuilder builder;

    builder.add_piece("a8", Color::Black, Piece::Rook);
    builder.add_piece("e8", Color::Black, Piece::King);
    builder.add_piece("h8", Color::Black, Piece::Rook);
    builder.add_piece("a1", Color::White, Piece::Rook);
    builder.add_piece("e1", Color::White, Piece::King);
    builder.add_piece("h1", Color::White, Piece::Rook);

    // add bishop to capture rook:
    builder.add_piece("b7", Color::White, Piece::Bishop);

    Board board = builder.build();
    
    Move mv = make_capturing_move (1, 1, 0, 0);

    CHECK(able_to_castle (board, Color::White, Castle_Queenside) == 1);
    CHECK(able_to_castle (board, Color::White, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::White, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::White)] == Castle_None);

    CHECK(able_to_castle (board, Color::Black, Castle_Queenside) == 1);
    CHECK(able_to_castle (board, Color::Black, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::Black, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::Black)] == Castle_None);

    UndoMove undo_state = do_move (board, Color::White, mv);

    CHECK(able_to_castle (board, Color::White, Castle_Queenside) == 1);
    CHECK(able_to_castle (board, Color::White, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::White, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::White)] == Castle_None);

    CHECK(able_to_castle (board, Color::Black, Castle_Queenside) == 0);
    CHECK(able_to_castle (board, Color::Black, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::Black, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::Black)] == Castle_Queenside);

    undo_move (board, Color::White, mv, undo_state);

    CHECK(able_to_castle (board, Color::White, Castle_Queenside) == 1);
    CHECK(able_to_castle (board, Color::White, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::White, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::White)] == Castle_None);

    CHECK(able_to_castle (board, Color::Black, Castle_Queenside) == 1);
    CHECK(able_to_castle (board, Color::Black, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::Black, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::Black)] == Castle_None);
}

TEST_CASE("Castling state is updated when rook captures a piece")
{
    BoardBuilder builder;

    builder.add_piece("a8", Color::Black, Piece::Rook);
    builder.add_piece("e8", Color::Black, Piece::King);
    builder.add_piece("h8", Color::Black, Piece::Rook);
    builder.add_piece("a1", Color::White, Piece::Rook);
    builder.add_piece("e1", Color::White, Piece::King);
    builder.add_piece("h1", Color::White, Piece::Rook);

    // add bishop for rook to capture:
    builder.add_piece("a7", Color::White, Piece::Bishop);

    Board board = builder.build();

    Move mv = make_capturing_move (0, 0, 1, 0);

    CHECK(able_to_castle (board, Color::White, Castle_Queenside) == 1);
    CHECK(able_to_castle (board, Color::White, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::White, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::White)] == Castle_None);

    CHECK(able_to_castle (board, Color::Black, Castle_Queenside) == 1);
    CHECK(able_to_castle (board, Color::Black, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::Black, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::Black)] == Castle_None);

    UndoMove undo_state = do_move (board, Color::Black, mv);;

    CHECK(able_to_castle (board, Color::White, Castle_Queenside) == 1);
    CHECK(able_to_castle (board, Color::White, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::White, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::White)] == Castle_None);

    CHECK(able_to_castle (board, Color::Black, Castle_Queenside) == 0);
    CHECK(able_to_castle (board, Color::Black, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::Black, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::Black)] == Castle_Queenside);

    undo_move (board, Color::Black, mv, undo_state);

    CHECK(able_to_castle (board, Color::White, Castle_Queenside) == 1);
    CHECK(able_to_castle (board, Color::White, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::White, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::White)] == Castle_None);

    CHECK(able_to_castle (board, Color::Black, Castle_Queenside) == 1);
    CHECK(able_to_castle (board, Color::Black, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::Black, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::Black)] == Castle_None);
}

TEST_CASE("Opponent's castling state is modified when his rook is taken (failure scenario)")
{
    BoardBuilder builder;

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

    Board board = builder.build();
    Move mv = make_capturing_move (0, 0, 0, 1);

    CHECK(able_to_castle (board, Color::White, Castle_Queenside) == 1);
    CHECK(able_to_castle (board, Color::White, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::White, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::White)] == Castle_None);

    CHECK(able_to_castle (board, Color::Black, Castle_Queenside) == 1);
    CHECK(able_to_castle (board, Color::Black, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::Black, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::Black)] == Castle_None);

    UndoMove undo_state = do_move (board, Color::Black, mv);;

    CHECK(able_to_castle (board, Color::White, Castle_Queenside) == 1);
    CHECK(able_to_castle (board, Color::White, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::White, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::White)] == Castle_None);

    CHECK(able_to_castle (board, Color::Black, Castle_Queenside) == 0);
    CHECK(able_to_castle (board, Color::Black, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::Black, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::Black)] == Castle_Queenside);

    undo_move (board, Color::Black, mv, undo_state);

    CHECK(able_to_castle (board, Color::White, Castle_Queenside) == 1);
    CHECK(able_to_castle (board, Color::White, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::White, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::White)] == Castle_None);

    CHECK(able_to_castle (board, Color::Black, Castle_Queenside) == 1);
    CHECK(able_to_castle (board, Color::Black, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::Black, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::Black)] == Castle_None);
}

TEST_CASE("Castling state is modified when rook takes a piece on same column (scenario 2)")
{
    BoardBuilder builder;

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

    Board board = builder.build();

    Move mv = make_capturing_move (7, 0, 6, 0);

    CHECK(able_to_castle (board, Color::White, Castle_Queenside) == 1);
    CHECK(able_to_castle (board, Color::White, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::White, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::White)] == Castle_None);

    CHECK(able_to_castle (board, Color::Black, Castle_Queenside) == 0);
    CHECK(able_to_castle (board, Color::Black, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::Black, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::Black)] == Castle_Queenside);

    UndoMove undo_state = do_move (board, Color::White, mv);

    CHECK(able_to_castle (board, Color::White, Castle_Queenside) == 0);
    CHECK(able_to_castle (board, Color::White, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::White, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::White)] == Castle_Queenside);

    CHECK(able_to_castle (board, Color::Black, Castle_Queenside) == 0);
    CHECK(able_to_castle (board, Color::Black, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::Black, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::Black)] == Castle_Queenside);

    undo_move (board, Color::White, mv, undo_state);

    CHECK(able_to_castle (board, Color::White, Castle_Queenside) == 1);
    CHECK(able_to_castle (board, Color::White, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::White, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::White)] == Castle_None);

    CHECK(able_to_castle (board, Color::Black, Castle_Queenside) == 0);
    CHECK(able_to_castle (board, Color::Black, Castle_Kingside) == 1);
    CHECK(able_to_castle (board, Color::Black, (Castle_Kingside | Castle_Kingside)) == 1);
    CHECK(board.castled[color_index(Color::Black)] == Castle_Queenside);
}

TEST_CASE( "Test can castle" )
{
    MoveList moves { Color::White, {"e2 e4",
            "d7 d5","e4 e5",
            "c8 d7","d2 d4","e7 e6","c2 c3","d7 c6","b1 a3","f8xa3","b2xa3","c6 a4",
            "d1 d2","g8 e7","a1 b1","e7 c6","b1xb7","b8 d7","g1 f3","a8 b8","d2 b2",
            "b8 a8","f1 e2","f7 f6","e5xf6","d7xf6"}
    };

    Board board;
    Color color = Color::White;
    REQUIRE(able_to_castle (board, Color::White, Castle_Kingside));

    int i = 0;
    for (auto move : moves)
    {
        INFO("Move : ");
        CAPTURE(i);
        i++;
        do_move (board, color, move);
        CHECK( able_to_castle (board, Color::White, Castle_Kingside) );
        color = color_invert (color);
    }
    auto castling = move_parse ("o-o", Color::White);
    do_move (board, Color::White, castling);
}

TEST_CASE( "Test can't castle scenario 1" )
{
//    MoveList moves { Color::White, {
//            "e2 e4","b8 c6",
//            "g1 f3","a8 b8",
//            "h1 g2","b8 a8"
//            "e7 e6","c2 c3","d7 c6","b1 a3","f8xa3","b2xa3","c6 a4",
//            "d1 d2","g8 e7","a1 b1","e7 c6","b1xb7","b8 d7","g1 f3","a8 b8","d2 b2",
//            "b8 a8","f1 e2","f7 f6","e5xf6","d7xf6"}
//    };
//
//    Board board;
//    Color color = Color::White;
//    REQUIRE(able_to_castle (board, Color::White, Castle_Kingside));
//
//    int i = 0;
//    for (auto move : moves)
//    {
//        INFO("Move : ");
//        CAPTURE(i);
//        i++;
//        do_move (board, color, move);
//        CHECK( able_to_castle (board, Color::White, Castle_Kingside) );
//        color = color_invert (color);
//    }
//    auto castling = move_parse ("o-o", Color::White);
//    do_move (board, Color::White, castling);
}