#include <doctest/doctest.h>

#include "material.hpp"
#include "board.hpp"
#include "board_builder.hpp"

using std::vector;
using namespace wisdom;

TEST_CASE( "Adding material works" )
{
    vector piece_types  { Piece::Bishop, Piece::Rook, Piece::Queen, Piece::Pawn };
    vector colors { Color::White, Color::Black };

    for (auto piece_type : piece_types)
    {
        for (auto color : colors)
        {
            Material material;

            material.add (make_piece (color, piece_type));

            CHECK( (color_index(color) == 0 || color_index(color) == 1) );
            CHECK(material.overall_score(color) > 0 );
        }
    }
}

TEST_CASE( "Deleting material works" )
{
    vector piece_types  { Piece::Bishop, Piece::Rook, Piece::Queen, Piece::Pawn };
    vector colors { Color::White, Color::Black };

    for (auto piece_type : piece_types)
    {
        for (auto color : colors)
        {
            Material material;

            material.remove (make_piece (color, piece_type));

            CHECK( (color_index (color) == 0 || color_index (color) == 1) );
            CHECK(material.overall_score(color) < 0 );
        }
    }
}


TEST_CASE( "Piece count" )
{
    Board default_board;
    BoardBuilder builder;

    builder.add_piece ("a1", Color::White, Piece::King);
    builder.add_piece ("a8", Color::Black, Piece::King);
    builder.add_piece ("e6", Color::White, Piece::Pawn);
    builder.add_piece ("f7", Color::Black, Piece::Knight);
    builder.add_piece ("h2", Color::White, Piece::Rook);
    builder.add_piece ("e2", Color::Black, Piece::Pawn);
    builder.add_piece ("d1", Color::White, Piece::Bishop);
    builder.add_piece ("f2", Color::Black, Piece::Pawn);

    auto brd = builder.build();
    const auto& material = brd->get_material ();

    SUBCASE( "Is updated from the amount of pieces on the board")
    {
        CHECK( material.piece_count (Color::Black, Piece::Pawn) == 2 );
        CHECK( material.piece_count (Color::White, Piece::Pawn) == 1 );
        CHECK( material.piece_count (Color::Black, Piece::Queen) == 0 );
        CHECK( material.piece_count (Color::Black, Piece::Rook) == 0 );
        CHECK( material.piece_count (Color::White, Piece::Rook) == 1 );
        CHECK( material.piece_count (Color::Black, Piece::Knight) == 1 );

        const auto& default_material = default_board.get_material ();
        CHECK( default_material.piece_count (Color::White, Piece::Pawn) == 8 );
        CHECK( default_material.piece_count (Color::Black, Piece::Pawn) == 8 );
        CHECK( default_material.piece_count (Color::White, Piece::Rook) == 2 );
        CHECK( default_material.piece_count (Color::Black, Piece::Rook) == 2 );
    }

    SUBCASE( "Is updated by a pawn doing a capture" )
    {
        auto capture_move = move_parse ("e6xf7", Color::White);

        auto undo_state = brd->make_move (Color::White, capture_move);
        CHECK( material.piece_count (Color::Black, Piece::Knight) == 0 );
        CHECK( material.piece_count (Color::Black, Piece::Pawn) == 2 );
        CHECK( material.piece_count (Color::White, Piece::Pawn) == 1 );

        brd->take_back (Color::White, capture_move, undo_state);

        CHECK( material.piece_count (Color::Black, Piece::Knight) == 1 );
        CHECK( material.piece_count (Color::Black, Piece::Pawn) == 2 );
        CHECK( material.piece_count (Color::White, Piece::Pawn) == 1 );
    }

    SUBCASE( "Is saved and restored after a pawn being captured" )
    {
        auto capture_move = move_parse ("h2xe2", Color::White);

        auto undo_state = brd->make_move (Color::White, capture_move);
        CHECK( material.piece_count (Color::Black, Piece::Knight) == 1 );
        CHECK( material.piece_count (Color::Black, Piece::Pawn) == 1 );
        CHECK( material.piece_count (Color::White, Piece::Pawn) == 1 );
        CHECK( material.piece_count (Color::Black, Piece::Knight) == 1 );

        brd->take_back (Color::White, capture_move, undo_state);

        CHECK( material.piece_count (Color::Black, Piece::Pawn) == 2 );
        CHECK( material.piece_count (Color::White, Piece::Pawn) == 1 );
        CHECK( material.piece_count (Color::Black, Piece::Knight) == 1 );
    }

    SUBCASE( "Is saved and restored after a promotion" )
    {
        brd->set_current_turn (Color::Black);
        auto promoting_move = move_parse ("e2e1 (Q)", Color::Black);

        auto undo_state = brd->make_move (Color::Black, promoting_move);
        CHECK( material.piece_count (Color::Black, Piece::Pawn) == 1 );
        CHECK( material.piece_count (Color::White, Piece::Pawn) == 1 );
        CHECK( material.piece_count (Color::Black, Piece::Queen) == 1 );
        CHECK( material.piece_count (Color::Black, Piece::Knight) == 1 );

        brd->take_back (Color::Black, promoting_move, undo_state);

        CHECK( material.piece_count (Color::Black, Piece::Pawn) == 2 );
        CHECK( material.piece_count (Color::White, Piece::Pawn) == 1 );
        CHECK( material.piece_count (Color::Black, Piece::Queen) == 0 );
        CHECK( material.piece_count (Color::Black, Piece::Knight) == 1 );
    }

    SUBCASE( "Is saved and restored after a promotion with a capture" )
    {
        brd->set_current_turn (Color::Black);
        auto promoting_move = move_parse ("e2xd1 (R)", Color::Black);

        auto undo_state = brd->make_move (Color::Black, promoting_move);
        CHECK( material.piece_count (Color::Black, Piece::Pawn) == 1 );
        CHECK( material.piece_count (Color::White, Piece::Pawn) == 1 );
        CHECK( material.piece_count (Color::Black, Piece::Rook) == 1 );
        CHECK( material.piece_count (Color::Black, Piece::Knight) == 1 );

        brd->take_back (Color::Black, promoting_move, undo_state);

        CHECK( material.piece_count (Color::Black, Piece::Pawn) == 2 );
        CHECK( material.piece_count (Color::White, Piece::Pawn) == 1 );
        CHECK( material.piece_count (Color::Black, Piece::Rook) == 0 );
        CHECK( material.piece_count (Color::Black, Piece::Knight) == 1 );
    }
}