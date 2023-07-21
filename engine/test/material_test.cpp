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

                material.add (ColoredPiece::make (color, piece_type));

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
                material.add (ColoredPiece::make (color, piece_type));
                material.remove (ColoredPiece::make (color, piece_type));

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

    builder.addPiece ("a1", Color::White, Piece::King);
    builder.addPiece ("a8", Color::Black, Piece::King);
    builder.addPiece ("e6", Color::White, Piece::Pawn);
    builder.addPiece ("f7", Color::Black, Piece::Knight);
    builder.addPiece ("h2", Color::White, Piece::Rook);
    builder.addPiece ("e2", Color::Black, Piece::Pawn);
    builder.addPiece ("d1", Color::White, Piece::Bishop);
    builder.addPiece ("f2", Color::Black, Piece::Pawn);

    auto brd = Board { builder };

    SUBCASE( "Is updated from the amount of pieces on the board")
    {
        const auto& material = brd.getMaterial ();
        CHECK( material.piece_count (Color::Black, Piece::Pawn) == 2 );
        CHECK( material.piece_count (Color::White, Piece::Pawn) == 1 );
        CHECK( material.piece_count (Color::Black, Piece::Queen) == 0 );
        CHECK( material.piece_count (Color::Black, Piece::Rook) == 0 );
        CHECK( material.piece_count (Color::White, Piece::Rook) == 1 );
        CHECK( material.piece_count (Color::Black, Piece::Knight) == 1 );

        const auto& default_material = default_board.getMaterial ();
        CHECK( default_material.piece_count (Color::White, Piece::Pawn) == 8 );
        CHECK( default_material.piece_count (Color::Black, Piece::Pawn) == 8 );
        CHECK( default_material.piece_count (Color::White, Piece::Rook) == 2 );
        CHECK( default_material.piece_count (Color::Black, Piece::Rook) == 2 );
    }

    SUBCASE( "Is updated by a pawn doing a capture" )
    {
        auto capture_move = move_parse ("e6xf7", Color::White);

        brd = brd.withMove (Color::White, capture_move);
        const auto& material = brd.getMaterial ();
        CHECK( material.piece_count (Color::Black, Piece::Knight) == 0 );
        CHECK( material.piece_count (Color::Black, Piece::Pawn) == 2 );
        CHECK( material.piece_count (Color::White, Piece::Pawn) == 1 );
    }

    SUBCASE( "Is saved and restored after a pawn being captured" )
    {
        auto capture_move = move_parse ("h2xe2", Color::White);

        brd = brd.withMove (Color::White, capture_move);
        const auto& material = brd.getMaterial ();
        CHECK( material.piece_count (Color::Black, Piece::Knight) == 1 );
        CHECK( material.piece_count (Color::Black, Piece::Pawn) == 1 );
        CHECK( material.piece_count (Color::White, Piece::Pawn) == 1 );
        CHECK( material.piece_count (Color::Black, Piece::Knight) == 1 );
    }

    SUBCASE( "Is saved and restored after a promotion" )
    {
        brd.setCurrentTurn (Color::Black);
        auto promoting_move = move_parse ("e2e1 (Q)", Color::Black);

        brd = brd.withMove (Color::Black, promoting_move);
        const auto& material = brd.getMaterial ();
        CHECK( material.piece_count (Color::Black, Piece::Pawn) == 1 );
        CHECK( material.piece_count (Color::White, Piece::Pawn) == 1 );
        CHECK( material.piece_count (Color::Black, Piece::Queen) == 1 );
        CHECK( material.piece_count (Color::Black, Piece::Knight) == 1 );
    }

    SUBCASE( "Is saved and restored after a promotion with a capture" )
    {
        brd.setCurrentTurn (Color::Black);
        auto promoting_move = move_parse ("e2xd1 (R)", Color::Black);

        brd = brd.withMove (Color::Black, promoting_move);
        const auto& material = brd.getMaterial ();
        CHECK( material.piece_count (Color::Black, Piece::Pawn) == 1 );
        CHECK( material.piece_count (Color::White, Piece::Pawn) == 1 );
        CHECK( material.piece_count (Color::Black, Piece::Rook) == 1 );
        CHECK( material.piece_count (Color::Black, Piece::Knight) == 1 );
    }
}

TEST_CASE( "checkmate_is_possible()" )
{
    using CheckmateIsPossible = Material::CheckmateIsPossible;

    Board default_board;
    BoardBuilder builder;
    const auto& default_material = default_board.getMaterial ();

    builder.addPiece ("a1", Color::White, Piece::King);
    builder.addPiece ("a8", Color::Black, Piece::King);

    SUBCASE( "Returns yes for default board" )
    {
        CHECK( default_material.checkmate_is_possible(default_board) );
    }

    SUBCASE( "Returns no if there is a rook and king" )
    {
        builder.addPiece ("a7", Color::Black, Piece::Rook);

        auto brd = Board { builder };
        const auto& material = brd.getMaterial ();
        CHECK( material.checkmate_is_possible(default_board) );
    }

    SUBCASE( "Returns no if there is a rook and king vs a rook and king" )
    {
        builder.addPiece ("a7", Color::Black, Piece::Rook);
        builder.addPiece ("c5", Color::White, Piece::Rook);

        auto brd = Board { builder };
        const auto& material = brd.getMaterial ();
        CHECK( material.checkmate_is_possible(default_board) );
    }

    SUBCASE( "Returns yes if there is a king and two knights vs a king" )
    {
        builder.addPiece ("a7", Color::Black, Piece::Knight);
        builder.addPiece ("b7", Color::Black, Piece::Knight);

        auto brd = Board { builder };
        const auto& material = brd.getMaterial ();
        CHECK( material.checkmate_is_possible(default_board) );
    }

    SUBCASE( "Returns yes if there is a king and knight vs a king and knight" )
    {
        builder.addPiece ("a7", Color::Black, Piece::Knight);
        builder.addPiece ("c7", Color::White, Piece::Knight);

        auto brd = Board { builder };
        const auto& material = brd.getMaterial ();
        CHECK( material.checkmate_is_possible(default_board) );
    }

    SUBCASE( "Returns yes if there is a king and two knights vs a king and knight" )
    {
        builder.addPiece ("a7", Color::Black, Piece::Knight);
        builder.addPiece ("b7", Color::Black, Piece::Knight);
        builder.addPiece ("c7", Color::White, Piece::Knight);

        auto brd = Board { builder };
        const auto& material = brd.getMaterial ();
        CHECK( material.checkmate_is_possible(default_board) );
    }

    SUBCASE( "Returns yes if there is a pawn" )
    {
        builder.addPiece ("a7", Color::Black, Piece::Pawn);
        auto brd = Board { builder };
        const auto& material = brd.getMaterial ();

        CHECK( material.checkmate_is_possible (brd) );
    }

    SUBCASE( "Returns no if there are only two kings" )
    {
        auto brd = Board { builder };
        const auto& material = brd.getMaterial ();

        CHECK( material.checkmate_is_possible (brd) == CheckmateIsPossible::No );
    }

    SUBCASE( "Returns no if there is a king and knight vs. a king" )
    {
        builder.addPiece ("a7", Color::Black, Piece::Knight);

        auto brd = Board { builder };
        const auto& material = brd.getMaterial ();

        CHECK( material.checkmate_is_possible (brd) == CheckmateIsPossible::No );
    }

    SUBCASE( "Returns no if there is a king and bishop vs. a king" )
    {
        builder.addPiece ("a7", Color::Black, Piece::Bishop);

        auto brd = Board { builder };
        const auto& material = brd.getMaterial ();

        CHECK( material.checkmate_is_possible (brd) == CheckmateIsPossible::No );
    }

    SUBCASE( "Returns yes if there is a king and bishop vs. a king and bishop of opposite colors" )
    {
        builder.addPiece ("a8", Color::White, Piece::Bishop);
        builder.addPiece ("a7", Color::Black, Piece::Bishop);

        auto brd = Board { builder };
        const auto& material = brd.getMaterial ();

        CHECK( material.checkmate_is_possible (brd) == CheckmateIsPossible::Yes );
    }

    SUBCASE( "Returns no if there is a king and bishop vs. a king and bishop of the same color" )
    {
        builder.addPiece ("a8", Color::White, Piece::Bishop);
        builder.addPiece ("b7", Color::Black, Piece::Bishop);

        auto brd = Board { builder };
        const auto& material = brd.getMaterial ();

        CHECK( material.checkmate_is_possible (brd) == CheckmateIsPossible::No);
    }

    SUBCASE( "Returns yes if there is a king two bishops of opposite color vs. a king" )
    {
        builder.addPiece ("a8", Color::White, Piece::Bishop);
        builder.addPiece ("b8", Color::White, Piece::Bishop);

        auto brd = Board { builder };
        const auto& material = brd.getMaterial ();

        CHECK( material.checkmate_is_possible (brd) == CheckmateIsPossible::Yes);
    }

    SUBCASE( "Returns no if there is a king two bishops of the same color vs. a king" )
    {
        builder.addPiece ("a8", Color::White, Piece::Bishop);
        builder.addPiece ("b7", Color::White, Piece::Bishop);

        auto brd = Board { builder };
        const auto& material = brd.getMaterial ();

        CHECK( material.checkmate_is_possible(brd) == CheckmateIsPossible::No );
    }
}