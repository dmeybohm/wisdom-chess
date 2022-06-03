#include <doctest/doctest.h>

#include "material.hpp"
#include "board.hpp"
#include "board_builder.hpp"

using std::vector;
using namespace wisdom;

TEST_CASE( "Overall score" )
{
    vector piece_types  { Piece::Bishop, Piece::Rook, Piece::Queen, Piece::Pawn };
    vector colors { Color::White, Color::Black };

    SUBCASE( "Adding material works" )
    {
        for (auto piece_type : piece_types)
        {
            for (auto color : colors)
            {
                Material material;

                material.add (make_piece (color, piece_type));

                CHECK( material.overall_score(color) > 0 );
            }
        }
    }

    SUBCASE( "Deleting material works" )
    {
        Material material;

        for (auto piece_type : piece_types)
        {
            for (auto color : colors)
            {
                material.add (make_piece (color, piece_type));
                material.remove (make_piece (color, piece_type));

                CHECK( material.overall_score(color) == 0 );
            }
        }

        CHECK( material.overall_score (Color::White) == 0 );
        CHECK( material.overall_score (Color::Black) == 0 );
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

TEST_CASE( "has_sufficient_material()" )
{
    Board default_board;
    BoardBuilder builder;
    const auto& default_material = default_board.get_material ();

    builder.add_piece ("a1", Color::White, Piece::King);
    builder.add_piece ("a8", Color::Black, Piece::King);

    SUBCASE( "Returns true for default board" )
    {
        CHECK( default_material.has_sufficient_material (default_board) );
    }

    SUBCASE( "Returns true if there is a king and two knights vs a king" )
    {
        builder.add_piece ("a7", Color::Black, Piece::Knight);
        builder.add_piece ("b7", Color::Black, Piece::Knight);

        auto brd = builder.build ();
        const auto& material = brd->get_material ();
        CHECK( material.has_sufficient_material (default_board) );
    }

    SUBCASE( "Returns true if there is a king and knight vs a king and knight" )
    {
        builder.add_piece ("a7", Color::Black, Piece::Knight);
        builder.add_piece ("c7", Color::White, Piece::Knight);

        auto brd = builder.build ();
        const auto& material = brd->get_material ();
        CHECK( material.has_sufficient_material (default_board) );
    }

    SUBCASE( "Returns true if there is a king and two knights vs a king and knight" )
    {
        builder.add_piece ("a7", Color::Black, Piece::Knight);
        builder.add_piece ("b7", Color::Black, Piece::Knight);
        builder.add_piece ("c7", Color::White, Piece::Knight);

        auto brd = builder.build ();
        const auto& material = brd->get_material ();
        CHECK( material.has_sufficient_material (default_board) );
    }

    SUBCASE( "Returns true if there is a pawn" )
    {
        builder.add_piece ("a7", Color::Black, Piece::Pawn);
        auto brd = builder.build ();
        const auto& material = brd->get_material ();

        CHECK( material.has_sufficient_material (*brd) );
    }

    SUBCASE( "Returns false if there are only two kings" )
    {
        auto brd = builder.build ();
        const auto& material = brd->get_material ();

        CHECK( !material.has_sufficient_material (*brd) );
    }

    SUBCASE( "Returns false if there is a king and knight vs. a king" )
    {
        builder.add_piece ("a7", Color::Black, Piece::Knight);

        auto brd = builder.build ();
        const auto& material = brd->get_material ();

        CHECK( !material.has_sufficient_material (*brd) );
    }

    SUBCASE( "Returns false if there is a king and bishop vs. a king" )
    {
        builder.add_piece ("a7", Color::Black, Piece::Bishop);

        auto brd = builder.build ();
        const auto& material = brd->get_material ();

        CHECK( !material.has_sufficient_material (*brd) );
    }

    SUBCASE( "Returns true if there is a king and bishop vs. a king and bishop of opposite colors" )
    {
        builder.add_piece ("a8", Color::White, Piece::Bishop);
        builder.add_piece ("a7", Color::Black, Piece::Bishop);

        auto brd = builder.build ();
        const auto& material = brd->get_material ();

        CHECK( material.has_sufficient_material (*brd) );
    }

    SUBCASE( "Returns false if there is a king and bishop vs. a king and bishop of the same color" )
    {
        builder.add_piece ("a8", Color::White, Piece::Bishop);
        builder.add_piece ("b7", Color::Black, Piece::Bishop);

        auto brd = builder.build ();
        const auto& material = brd->get_material ();

        CHECK( !material.has_sufficient_material (*brd) );
    }

    SUBCASE( "Returns true if there is a king two bishops of opposite color vs. a king" )
    {
        builder.add_piece ("a8", Color::White, Piece::Bishop);
        builder.add_piece ("b8", Color::White, Piece::Bishop);

        auto brd = builder.build ();
        const auto& material = brd->get_material ();

        CHECK( material.has_sufficient_material (*brd) );
    }

    SUBCASE( "Returns false if there is a king two bishops of the same color vs. a king" )
    {
        builder.add_piece ("a8", Color::White, Piece::Bishop);
        builder.add_piece ("b7", Color::White, Piece::Bishop);

        auto brd = builder.build ();
        const auto& material = brd->get_material ();

        CHECK( !material.has_sufficient_material (*brd) );
    }
}