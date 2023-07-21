#include <doctest/doctest.h>
#include "board.hpp"
#include "board_builder.hpp"

#include <algorithm>
#include <iostream>

#include "board_code.hpp"
#include "coord.hpp"

using namespace wisdom;

TEST_CASE( "board code")
{
    SUBCASE( "Board code is able to be set" )
    {
        BoardCode code = BoardCode::from_empty_board ();
        BoardCode initial = BoardCode::from_empty_board ();

        auto initial_str = code.to_string ();
        std::size_t num_zeroes = std::count (initial_str.begin (), initial_str.end (), '0');
        REQUIRE( num_zeroes == initial_str.size () );

        Coord a8 = coord_parse ("a8");
        ColoredPiece black_pawn = ColoredPiece::make (Color::Black, Piece::Pawn);
        code.add_piece (a8, black_pawn);

        REQUIRE( code != initial );

        code.remove_piece (a8);

        REQUIRE( code == initial );

        Coord h1 = coord_parse ("h1");
        ColoredPiece white_king = ColoredPiece::make (Color::White, Piece::King);
        code.add_piece (h1, white_king);

        std::string result = code.to_string ();
        result = result.substr (0, 4);
        CHECK( result == "0110" );

        code.remove_piece (h1);
        result = code.to_string ().substr (0, 4);
        CHECK( result == "0000" );
    }

    SUBCASE("Capturing moves are applied and undone correctly")
    {
        BoardBuilder builder;

        builder.addPiece ("a8", Color::White, Piece::Bishop);
        builder.addPiece ("b7", Color::Black, Piece::Knight);
        builder.addPiece ("e1", Color::Black, Piece::King);
        builder.addPiece ("e8", Color::White, Piece::King);

        auto brd = Board { builder };
        BoardCode code  = BoardCode::from_board (brd);
        BoardCode initial = code;

        REQUIRE( initial.count_ones () > 0 );

        Move a8xb7 = move_parse ("a8xb7");
        code.apply_move (brd, a8xb7);
        REQUIRE( initial != code );
    }

    SUBCASE("Promoting moves are applied and undone correctly")
    {
        BoardBuilder builder;

        builder.addPiece ("a8", Color::White, Piece::Bishop);
        builder.addPiece ("b7", Color::Black, Piece::Pawn);
        builder.addPiece ("e1", Color::Black, Piece::King);
        builder.addPiece ("e8", Color::White, Piece::King);

        auto brd = Board { builder };
        brd.setCurrentTurn (Color::Black);
        BoardCode code = BoardCode::from_board (brd);
        BoardCode initial = code;

        REQUIRE( initial.count_ones () > 0 );

        Move b7b8_Q = move_parse ("b7b8_Q (Q)");
        code.apply_move (brd, b7b8_Q);
        REQUIRE( initial != code );
    }

    SUBCASE("Castling moves are applied and undone correctly")
    {
        BoardBuilder builder;

        builder.addPiece ("a8", Color::Black, Piece::Rook);
        builder.addPiece ("b6", Color::Black, Piece::Pawn);
        builder.addPiece ("e8", Color::Black, Piece::King);
        builder.addPiece ("e1", Color::White, Piece::King);

        auto brd = Board { builder };
        brd.setCurrentTurn (Color::Black);
        BoardCode code  = BoardCode::from_board (brd);
        BoardCode initial = code;

        REQUIRE( initial.count_ones () > 0 );

        Move castle_queenside = move_parse ("o-o-o", Color::Black);
        code.apply_move (brd, castle_queenside);
        REQUIRE( initial != code );
    }

    SUBCASE("Promoting+Capturing moves are applied and undone correctly")
    {
        BoardBuilder builder;

        builder.addPiece ("a8", Color::White, Piece::Rook);
        builder.addPiece ("b7", Color::Black, Piece::Pawn);
        builder.addPiece ("e8", Color::Black, Piece::King);
        builder.addPiece ("e1", Color::White, Piece::King);

        auto brd = Board { builder };
        brd.setCurrentTurn (Color::Black);
        BoardCode code = BoardCode::from_board (brd);
        BoardCode initial = code;

        REQUIRE (initial.count_ones () > 0);

        Move promote_castle_move = move_parse ("b7xa8 (Q)", Color::Black);
        REQUIRE(promote_castle_move.is_promoting () );
        REQUIRE( promote_castle_move.is_normal_capturing () );

        code.apply_move (brd, promote_castle_move);
        REQUIRE( initial != code);
    }
}


TEST_CASE( "Board code stores ancilliary state" )
{
    SUBCASE( "Board code stores en passant state for Black" )
    {
        BoardBuilder builder;

        builder.addPiece ("e5", Color::White, Piece::Pawn);
        builder.addPiece ("d5", Color::Black, Piece::Pawn);
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.addPiece ("e8", Color::Black, Piece::King);

        auto board_without_state = Board { builder };
        builder.setEnPassantTarget (Color::Black, "d6");
        auto board_with_state = Board { builder };

        auto with_state_code = board_with_state.getCode ();
        auto without_state_code = board_without_state.getCode ();
        CHECK( with_state_code != without_state_code );

        auto en_passant_target = with_state_code.en_passant_target (Color::Black);
        auto expected_coord = coord_parse("d6");
        auto ancilliary = with_state_code.get_ancillary_bits ();
        INFO(ancilliary);
        INFO( wisdom::to_string (en_passant_target) );
        CHECK( en_passant_target == expected_coord );

        auto opponent_target = with_state_code.en_passant_target (Color::White);
        CHECK( opponent_target == No_En_Passant_Coord );
    }

    SUBCASE( "Board code stores en passant state for White" )
    {
        BoardBuilder builder;

        builder.addPiece ("e4", Color::White, Piece::Pawn);
        builder.addPiece ("d4", Color::Black, Piece::Pawn);
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.addPiece ("e8", Color::Black, Piece::King);

        auto board_without_state = Board { builder };
        builder.setEnPassantTarget (Color::White, "e3");
        auto board_with_state = Board { builder };

        auto with_state_code = board_with_state.getCode ();
        auto without_state_code = board_without_state.getCode ();
        CHECK( with_state_code != without_state_code );

        auto en_passant_target = with_state_code.en_passant_target (Color::White);

        auto expected_coord = coord_parse ("e3");
        auto ancilliary = with_state_code.get_ancillary_bits ();
        CHECK( en_passant_target == expected_coord );

        auto opponent_target = with_state_code.en_passant_target (Color::Black);
        INFO(ancilliary);
        INFO( wisdom::to_string (opponent_target) );

        CHECK( opponent_target == No_En_Passant_Coord );
    }

    SUBCASE( "Board code stores castle state" )
    {

    }

    SUBCASE( "Board code stores whose turn it is" )
    {

    }
}
