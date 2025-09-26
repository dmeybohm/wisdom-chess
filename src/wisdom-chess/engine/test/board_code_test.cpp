#include <algorithm>
#include <iostream>
#include <sstream>

#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/board_code.hpp"
#include "wisdom-chess/engine/coord.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "board code" )
{
    SUBCASE( "Board code is able to be set for an empty board" )
    {
        BoardCode code = BoardCode::fromEmptyBoard();
        BoardCode initial = BoardCode::fromEmptyBoard();

        auto initial_str = code.asString();
        std::size_t num_zeroes = std::count (initial_str.begin(), initial_str.end(), '0');
        REQUIRE( num_zeroes > 0 );
        REQUIRE( num_zeroes < 64 );

        Coord a8 = coordParse ("a8");
        ColoredPiece black_pawn = ColoredPiece::make (Color::Black, Piece::Pawn);
        code.addPiece (a8, black_pawn);

        REQUIRE( code != initial );

        code.removePiece (a8, black_pawn);

        REQUIRE( code == initial );

        Coord h1 = coordParse ("h1");
        ColoredPiece white_king = ColoredPiece::make (Color::White, Piece::King);
        code.addPiece (h1, white_king);

        std::string result = code.asString();
        CHECK( code != initial );

        code.removePiece (h1, white_king);
        CHECK( code == initial );
    }

    SUBCASE( "Board code sets up a default board" )
    {
        BoardCode code = BoardCode::fromDefaultPosition();

        auto num_ones = code.numberOfSetBits();

        CHECK( num_ones < 64 );  // ... some number less than all the bits.
    }

    SUBCASE( "Capturing moves are applied and undone correctly" )
    {
        BoardBuilder builder;

        builder.addPiece ("a8", Color::White, Piece::Bishop);
        builder.addPiece ("b7", Color::Black, Piece::Knight);
        builder.addPiece ("e1", Color::Black, Piece::King);
        builder.addPiece ("e8", Color::White, Piece::King);

        auto brd = Board { builder };
        BoardCode code  = BoardCode::fromBoard (brd);
        BoardCode initial = code;

        REQUIRE( initial.numberOfSetBits() > 0 );

        Move a8xb7 = moveParse ("a8xb7");
        code.applyMove (brd, a8xb7);
        REQUIRE( initial != code );
    }

    SUBCASE( "Promoting moves are applied and undone correctly" )
    {
        BoardBuilder builder;

        builder.addPiece ("a8", Color::White, Piece::Bishop);
        builder.addPiece ("b7", Color::Black, Piece::Pawn);
        builder.addPiece ("e1", Color::Black, Piece::King);
        builder.addPiece ("e8", Color::White, Piece::King);
        builder.setCurrentTurn (Color::Black);

        auto brd = Board { builder };
        BoardCode code = BoardCode::fromBoard (brd);
        BoardCode initial = code;

        REQUIRE( initial.numberOfSetBits() > 0 );

        Move b7b8_Q = moveParse ("b7b8_Q (Q)");
        code.applyMove (brd, b7b8_Q);
        REQUIRE( initial != code );
    }

    SUBCASE( "Castling moves are applied and undone correctly" )
    {
        BoardBuilder builder;

        builder.addPiece ("a8", Color::Black, Piece::Rook);
        builder.addPiece ("b6", Color::Black, Piece::Pawn);
        builder.addPiece ("e8", Color::Black, Piece::King);
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.setCurrentTurn (Color::Black);

        auto brd = Board { builder };
        BoardCode code  = BoardCode::fromBoard (brd);
        BoardCode initial = code;

        REQUIRE( initial.numberOfSetBits() > 0 );

        Move castle_queenside = moveParse ("o-o-o", Color::Black);
        code.applyMove (brd, castle_queenside);
        REQUIRE( initial != code );
    }

    SUBCASE( "Promoting+Capturing moves are applied and undone correctly")
    {
        BoardBuilder builder;

        builder.addPiece ("a8", Color::White, Piece::Rook);
        builder.addPiece ("b7", Color::Black, Piece::Pawn);
        builder.addPiece ("e8", Color::Black, Piece::King);
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.setCurrentTurn (Color::Black);

        auto brd = Board { builder };
        BoardCode code = BoardCode::fromBoard (brd);
        BoardCode initial = code;

        REQUIRE( initial.numberOfSetBits() > 0 );

        Move promote_castle_move = moveParse ("b7xa8 (Q)", Color::Black);
        REQUIRE( promote_castle_move.isPromoting() );
        REQUIRE( promote_castle_move.isNormalCapturing() );

        code.applyMove (brd, promote_castle_move);
        REQUIRE( initial != code );
    }
}

TEST_CASE( "Board code can be converted" )
{
    SUBCASE( "to a string" )
    {
        std::stringstream stream;
        BoardCode code = BoardCode::fromEmptyBoard();

        code.addPiece(
            coordParse("h1"),
            ColoredPiece::make (Color::White, Piece::King)
        );

        auto result = code.asString().substr (0, 4);

        std::size_t num_zeroes = std::count (result.begin(), result.end(), '0');
        CHECK( num_zeroes > 0 );
        CHECK( num_zeroes < 64 );
    }

    SUBCASE( "to an ostream" )
    {
        std::stringstream stream;
        BoardCode code = BoardCode::fromEmptyBoard();

        code.addPiece(
            coordParse("h1"),
            ColoredPiece::make (Color::White, Piece::King)
        );

        stream << code;
        auto result = stream.str().substr(0, 4);

        std::size_t num_zeroes = std::count (result.begin(), result.end(), '0');
        CHECK( num_zeroes > 0 );
        CHECK( num_zeroes < 64 );
    }
}

TEST_CASE( "Board code stores metadata" )
{
    SUBCASE( "setMetadataBits preserves high bits of hash code" )
    {
        BoardCode code = BoardCode::fromEmptyBoard();

        code.addPiece (
            coordParse ("a1"),
            ColoredPiece::make (Color::White, Piece::King)
        );
        code.addPiece (
            coordParse ("h8"),
            ColoredPiece::make (Color::Black, Piece::King)
        );

        code.setCastleState (Color::White, CastlingEligibility::Either_Side);
        code.setCastleState (Color::Black, CastlingEligibility::Either_Side);

        auto initial_hash = code.getHashCode();
        auto high_48_bits = initial_hash & 0xfffffffFFFF0000ULL;
        auto low_16_bits = initial_hash & 0xffffULL;

        code.setCurrentTurn (Color::Black);
        code.setCastleState (Color::White, CastlingEligibility::Neither_Side);
        code.setCastleState (Color::Black, CastlingIneligible::Queenside);
        code.setEnPassantTarget (Color::White, coordParse ("e3"));

        auto modified_hash = code.getHashCode();
        auto modified_high_48_bits = modified_hash & 0xfffffffFFFF0000ULL;
        auto modified_low_16_bits = modified_hash & 0xffffULL;

        CHECK( modified_high_48_bits == high_48_bits );

        CHECK( modified_low_16_bits != low_16_bits );

        code.clearEnPassantTarget();
        code.setCastleState (Color::White, CastlingEligibility::Either_Side);
        code.setCastleState (Color::Black, CastlingEligibility::Either_Side);
        code.setCurrentTurn (Color::White);

        auto restored_hash = code.getHashCode();
        CHECK( restored_hash == initial_hash );
    }

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

        auto with_state_code = board_with_state.getCode();
        auto without_state_code = board_without_state.getCode();
        CHECK( with_state_code != without_state_code );

        auto en_passant_target = with_state_code.enPassantTarget ();
        auto expected_coord = coordParse ("d6");
        auto metadata = with_state_code.getMetadataBits();
        REQUIRE( en_passant_target.has_value() );
        CHECK( en_passant_target->vulnerable_color == Color::Black );
        CHECK( en_passant_target->coord == expected_coord );
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

        auto with_state_code = board_with_state.getCode();
        auto without_state_code = board_without_state.getCode();
        CHECK( with_state_code != without_state_code );

        auto en_passant_target = with_state_code.getEnPassantTarget();

        auto expected_coord = coordParse ("e3");
        REQUIRE( en_passant_target.has_value() );
        CHECK( en_passant_target->vulnerable_color == Color::White );
        CHECK( en_passant_target->coord == expected_coord );
    }

    SUBCASE( "Board code stores castle state" )
    {
        BoardCode board_code = BoardCode::fromDefaultPosition();

        auto initial_white_state = board_code.getCastleState (Color::White);
        auto initial_black_state = board_code.getCastleState (Color::Black);

        CHECK( initial_white_state == CastlingEligibility::Either_Side );
        CHECK( initial_black_state == CastlingEligibility::Either_Side );

        board_code.setCastleState (Color::White, CastlingEligibility::Neither_Side);
        auto white_state = board_code.getCastleState (Color::White);
        auto black_state = board_code.getCastleState (Color::Black);

        CHECK( white_state == CastlingEligibility::Neither_Side );
        CHECK( black_state == CastlingEligibility::Either_Side );

        board_code.setCastleState (Color::White, CastlingIneligible::Kingside);
        board_code.setCastleState (Color::Black, CastlingIneligible::Queenside);
        white_state = board_code.getCastleState (Color::White);
        black_state = board_code.getCastleState (Color::Black);

        CHECK( white_state == CastlingIneligible::Kingside );
        CHECK( black_state == CastlingIneligible::Queenside );
    }

    SUBCASE( "Board code stores whose turn it is" )
    {
        BoardCode board_code = BoardCode::fromDefaultPosition();

        auto current_turn = board_code.getCurrentTurn();
        CHECK( current_turn == Color::White );

        board_code.setCurrentTurn (Color::Black);
        auto next_turn = board_code.getCurrentTurn();
        CHECK( next_turn == Color::Black );
    }
}
