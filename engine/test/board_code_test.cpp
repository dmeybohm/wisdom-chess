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
    SUBCASE("Board code is able to be set")
    {
        BoardCode code, initial;

        auto initial_str = code.to_string ();
        std::size_t num_zeroes = std::count (initial_str.begin (), initial_str.end (), '0');
        REQUIRE( num_zeroes == initial_str.size () );

        Coord a8 = coord_parse ("a8");
        ColoredPiece black_pawn = make_piece (Color::Black, Piece::Pawn);
        code.add_piece (a8, black_pawn);

        REQUIRE( code != initial );

        code.remove_piece (a8);

        REQUIRE( code == initial );

        Coord h1 = coord_parse ("h1");
        ColoredPiece white_king = make_piece (Color::White, Piece::King);
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

        builder.add_piece ("a8", Color::White, Piece::Bishop);
        builder.add_piece ("b7", Color::Black, Piece::Knight);
        builder.add_piece ("e1", Color::Black, Piece::King);
        builder.add_piece ("e8", Color::White, Piece::King);

        auto brd = builder.build ();
        BoardCode code { *brd };
        BoardCode initial = code;

        REQUIRE( initial.count_ones () > 0 );

        Move a8xb7 = move_parse ("a8xb7");
        code.apply_move (*brd, a8xb7);
        REQUIRE( initial != code );

        UndoMove undo_state = brd->make_move (Color::White, a8xb7);

        code.unapply_move (*brd, a8xb7, undo_state);

        REQUIRE( initial == code );
    }

    SUBCASE("Promoting moves are applied and undone correctly")
    {
        BoardBuilder builder;

        builder.add_piece ("a8", Color::White, Piece::Bishop);
        builder.add_piece ("b7", Color::Black, Piece::Pawn);
        builder.add_piece ("e1", Color::Black, Piece::King);
        builder.add_piece ("e8", Color::White, Piece::King);

        auto brd = builder.build ();
        BoardCode code { *brd };
        BoardCode initial = code;

        REQUIRE( initial.count_ones () > 0 );

        Move b7b8_Q = move_parse ("b7b8_Q (Q)");
        code.apply_move (*brd, b7b8_Q);
        REQUIRE( initial != code );

        UndoMove undo_state = brd->make_move (Color::Black, b7b8_Q);
        code.unapply_move (*brd, b7b8_Q, undo_state);
        REQUIRE( initial == code );
    }

    SUBCASE("Castling moves are applied and undone correctly")
    {
        BoardBuilder builder;

        builder.add_piece ("a8", Color::Black, Piece::Rook);
        builder.add_piece ("b6", Color::Black, Piece::Pawn);
        builder.add_piece ("e8", Color::Black, Piece::King);
        builder.add_piece ("e1", Color::White, Piece::King);

        auto brd = builder.build ();
        BoardCode code { *brd };
        BoardCode initial = code;

        REQUIRE( initial.count_ones () > 0 );

        Move castle_queenside = move_parse ("o-o-o", Color::Black);
        code.apply_move (*brd, castle_queenside);
        REQUIRE( initial != code );

        UndoMove undo_state = brd->make_move (Color::Black, castle_queenside);
        code.unapply_move (*brd, castle_queenside, undo_state);
        REQUIRE( initial == code );
    }

    SUBCASE("Promoting+Capturing moves are applied and undone correctly")
    {
        BoardBuilder builder;

        builder.add_piece ("a8", Color::White, Piece::Rook);
        builder.add_piece ("b7", Color::Black, Piece::Pawn);
        builder.add_piece ("e8", Color::Black, Piece::King);
        builder.add_piece ("e1", Color::White, Piece::King);

        auto brd = builder.build ();
        BoardCode code { *brd };
        BoardCode initial = code;

        REQUIRE (initial.count_ones () > 0);

        Move promote_castle_move = move_parse ("b7xa8 (Q)", Color::Black);
        REQUIRE( is_promoting_move (promote_castle_move) );
        REQUIRE( is_normal_capture_move (promote_castle_move) );

        code.apply_move (*brd, promote_castle_move);
        REQUIRE( initial != code);

        UndoMove undo_state = brd->make_move (Color::Black, promote_castle_move);
        code.unapply_move (*brd, promote_castle_move, undo_state);
        REQUIRE( initial == code);
    }
}


TEST_CASE( "Board code stores ancilliary state" )
{
    SUBCASE( "Board code stores en passant state for Black" )
    {
        BoardBuilder builder;

        builder.add_piece ("e5", Color::White, Piece::Pawn);
        builder.add_piece ("d5", Color::Black, Piece::Pawn);
        builder.add_piece ("e1", Color::White, Piece::King);
        builder.add_piece ("e8", Color::Black, Piece::King);

        auto board_without_state = builder.build ();
        builder.set_en_passant_target (Color::Black ,"d6");
        auto board_with_state = builder.build ();

        auto with_state_code = board_with_state->get_code ();
        auto without_state_code = board_without_state->get_code ();
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

        builder.add_piece ("e4", Color::White, Piece::Pawn);
        builder.add_piece ("d4", Color::Black, Piece::Pawn);
        builder.add_piece ("e1", Color::White, Piece::King);
        builder.add_piece ("e8", Color::Black, Piece::King);

        auto board_without_state = builder.build ();
        builder.set_en_passant_target (Color::White ,"e3");
        auto board_with_state = builder.build ();

        auto with_state_code = board_with_state->get_code ();
        auto without_state_code = board_without_state->get_code ();
        CHECK( with_state_code != without_state_code );

        auto en_passant_target = with_state_code.en_passant_target (Color::White);
        auto expected_coord = coord_parse("e3");
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