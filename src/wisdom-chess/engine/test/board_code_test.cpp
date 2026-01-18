#include <algorithm>
#include <iostream>
#include <set>
#include <sstream>

#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/board_code.hpp"
#include "wisdom-chess/engine/coord.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "board code" )
{
    SUBCASE( "Board code is able to be set for an empty board" )
    {
        BoardCode code = BoardCode::fromEmptyBoard();
        BoardCode initial = BoardCode::fromEmptyBoard();

        CHECK( code.getHashCode() == 0 );

        Coord a8 = coordParse ("a8");
        ColoredPiece black_pawn = ColoredPiece::make (Color::Black, Piece::Pawn);
        code.addPiece (a8, black_pawn);

        REQUIRE( code != initial );

        code.removePiece (a8, black_pawn);

        REQUIRE( code == initial );

        Coord h1 = coordParse ("h1");
        ColoredPiece white_king = ColoredPiece::make (Color::White, Piece::King);
        code.addPiece (h1, white_king);

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

    SUBCASE( "Capturing moves are applied correctly" )
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

    SUBCASE( "Promoting moves are applied correctly" )
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

    SUBCASE( "Castling moves are applied correctly" )
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

    SUBCASE( "Promoting+Capturing moves are applied correctly" )
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

    SUBCASE( "Reverting board to same position gives the same board code" )
    {
        Board default_board;
        BoardCode code = default_board.getCode();

        Move white_knight_ahead = moveParse ("g1f3", Color::White),
             white_knight_return = moveParse ("f3g1", Color::White);
        Move black_knight_ahead = moveParse ("g8f6", Color::Black),
             black_knight_return = moveParse ("f6g8", Color::Black);

        Board new_board = default_board
            .withMove (Color::White, white_knight_ahead)
            .withMove (Color::Black, black_knight_ahead)
            .withMove (Color::White, white_knight_return)
            .withMove (Color::Black, black_knight_return);

        BoardCode new_code = new_board.getCode();
        REQUIRE( code == new_code );
    }

    SUBCASE( "Position from withMove() and from FEN yields same board code" )
    {
        Board default_board;

        Move white_knight_ahead = moveParse ("g1f3", Color::White);
        Move black_knight_ahead = moveParse ("g8f6", Color::Black);

        Board moved_board = default_board
            .withMove (Color::White, white_knight_ahead)
            .withMove (Color::Black, black_knight_ahead);

        BoardCode moved_code = moved_board.getCode();

        FenParser parser ("rnbqkb1r/pppppppp/5n2/8/8/5N2/PPPPPPPP/RNBQKB1R w KQkq - 2 2");
        Board fen_board = parser.buildBoard();
        BoardCode fen_code = fen_board.getCode();

        REQUIRE( moved_code == fen_code );
    }

    SUBCASE( "Position from withMove() and from BoardBuilder yields same board code" )
    {
        Board default_board;

        Move white_knight_ahead = moveParse ("g1f3", Color::White);
        Move black_knight_ahead = moveParse ("g8f6", Color::Black);

        Board moved_board = default_board
            .withMove (Color::White, white_knight_ahead)
            .withMove (Color::Black, black_knight_ahead);

        BoardCode moved_code = moved_board.getCode();

        BoardBuilder builder;
        builder.addPiece ("a1", Color::White, Piece::Rook);
        builder.addPiece ("b1", Color::White, Piece::Knight);
        builder.addPiece ("c1", Color::White, Piece::Bishop);
        builder.addPiece ("d1", Color::White, Piece::Queen);
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.addPiece ("f1", Color::White, Piece::Bishop);
        builder.addPiece ("f3", Color::White, Piece::Knight);
        builder.addPiece ("h1", Color::White, Piece::Rook);
        builder.addRowOfSameColorAndPiece (6, Color::White, Piece::Pawn);

        builder.addPiece ("a8", Color::Black, Piece::Rook);
        builder.addPiece ("b8", Color::Black, Piece::Knight);
        builder.addPiece ("c8", Color::Black, Piece::Bishop);
        builder.addPiece ("d8", Color::Black, Piece::Queen);
        builder.addPiece ("e8", Color::Black, Piece::King);
        builder.addPiece ("f8", Color::Black, Piece::Bishop);
        builder.addPiece ("f6", Color::Black, Piece::Knight);
        builder.addPiece ("h8", Color::Black, Piece::Rook);
        builder.addRowOfSameColorAndPiece (1, Color::Black, Piece::Pawn);

        builder.setCurrentTurn (Color::White);

        BoardCode builder_code = BoardCode::fromBoardBuilder (builder);

        REQUIRE( moved_code == builder_code );
    }

    SUBCASE( "Promotion hash matches fresh board computation" )
    {
        BoardBuilder builder;

        builder.addPiece ("b7", Color::White, Piece::Pawn);
        builder.addPiece ("e8", Color::Black, Piece::King);
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.setCurrentTurn (Color::White);

        Board before = Board { builder };
        Move promote_move = moveParse ("b7b8 (Q)", Color::White);
        Board after = before.withMove (Color::White, promote_move);

        BoardBuilder expected_builder;
        expected_builder.addPiece ("b8", Color::White, Piece::Queen);
        expected_builder.addPiece ("e8", Color::Black, Piece::King);
        expected_builder.addPiece ("e1", Color::White, Piece::King);
        expected_builder.setCurrentTurn (Color::Black);

        auto expected_code = BoardCode::fromBoardBuilder (expected_builder);
        auto actual_code = after.getCode();

        CHECK( actual_code.getHashCode() == expected_code.getHashCode() );
    }

    SUBCASE( "Capture hash matches fresh board computation" )
    {
        BoardBuilder builder;

        builder.addPiece ("e4", Color::White, Piece::Knight);
        builder.addPiece ("d6", Color::Black, Piece::Bishop);
        builder.addPiece ("e8", Color::Black, Piece::King);
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.setCurrentTurn (Color::White);

        Board before = Board { builder };
        Move capture_move = moveParse ("e4xd6", Color::White);
        Board after = before.withMove (Color::White, capture_move);

        BoardBuilder expected_builder;
        expected_builder.addPiece ("d6", Color::White, Piece::Knight);
        expected_builder.addPiece ("e8", Color::Black, Piece::King);
        expected_builder.addPiece ("e1", Color::White, Piece::King);
        expected_builder.setCurrentTurn (Color::Black);

        auto expected_code = BoardCode::fromBoardBuilder (expected_builder);
        auto actual_code = after.getCode();

        CHECK( actual_code.getHashCode() == expected_code.getHashCode() );
    }

    SUBCASE( "Promotion with capture hash matches fresh board computation" )
    {
        BoardBuilder builder;

        builder.addPiece ("b7", Color::White, Piece::Pawn);
        builder.addPiece ("a8", Color::Black, Piece::Rook);
        builder.addPiece ("e8", Color::Black, Piece::King);
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.setCurrentTurn (Color::White);

        Board before = Board { builder };
        Move promote_capture = moveParse ("b7xa8 (Q)", Color::White);
        Board after = before.withMove (Color::White, promote_capture);

        BoardBuilder expected_builder;
        expected_builder.addPiece ("a8", Color::White, Piece::Queen);
        expected_builder.addPiece ("e8", Color::Black, Piece::King);
        expected_builder.addPiece ("e1", Color::White, Piece::King);
        expected_builder.setCurrentTurn (Color::Black);

        auto expected_code = BoardCode::fromBoardBuilder (expected_builder);
        auto actual_code = after.getCode();

        CHECK( actual_code.getHashCode() == expected_code.getHashCode() );
    }
}

TEST_CASE( "Board code can be converted" )
{
    SUBCASE( "to a string" )
    {
        std::stringstream stream;
        BoardCode code = BoardCode::fromEmptyBoard();

        code.addPiece(
            coordParse ("h1"),
            ColoredPiece::make (Color::White, Piece::King)
        );

        auto result = code.asString();

        std::size_t num_zeroes = std::count (result.begin(), result.end(), '0');
        CHECK( num_zeroes > 0 );
        CHECK( num_zeroes < 64 );
    }

    SUBCASE( "to an ostream" )
    {
        std::stringstream stream;
        BoardCode code = BoardCode::fromEmptyBoard();

        code.addPiece(
            coordParse ("h1"),
            ColoredPiece::make (Color::White, Piece::King)
        );

        stream << code;
        auto result = stream.str();

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
        code.setCastleState (Color::Black, CastlingRights::Queenside);
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

        auto en_passant_target = with_state_code.getEnPassantTarget();
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

        board_code.setCastleState (Color::White, CastlingRights::Kingside);
        board_code.setCastleState (Color::Black, CastlingRights::Queenside);
        white_state = board_code.getCastleState (Color::White);
        black_state = board_code.getCastleState (Color::Black);

        CHECK( white_state == CastlingRights::Kingside );
        CHECK( black_state == CastlingRights::Queenside );
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

TEST_CASE( "Zobrist piece index mapping" )
{
    SUBCASE( "All piece-color combinations have unique indices" )
    {
        std::set<int> seen_indices;

        for (auto color : { Color::White, Color::Black })
        {
            for (auto piece : { Piece::Pawn, Piece::Knight, Piece::Bishop,
                                Piece::Rook, Piece::Queen, Piece::King })
            {
                auto index = zobristPieceIndex (color, piece);
                auto [it, inserted] = seen_indices.insert (index);
                CHECK_MESSAGE(
                    inserted,
                    "Duplicate index " << index << " for "
                        << (color == Color::White ? "White" : "Black") << " "
                        << pieceToChar (ColoredPiece::make (color, piece))
                );
            }
        }

        CHECK( seen_indices.size() == 12 );
    }

    SUBCASE( "White and black pieces have different indices for same piece type" )
    {
        for (auto piece : { Piece::Pawn, Piece::Knight, Piece::Bishop,
                            Piece::Rook, Piece::Queen, Piece::King })
        {
            auto white_index = zobristPieceIndex (Color::White, piece);
            auto black_index = zobristPieceIndex (Color::Black, piece);
            CHECK( white_index != black_index );
        }
    }

    SUBCASE( "Indices are within valid table bounds" )
    {
        constexpr int max_valid_index = Num_Players * Num_Piece_Types - 1;

        for (auto color : { Color::White, Color::Black })
        {
            for (auto piece : { Piece::Pawn, Piece::Knight, Piece::Bishop,
                                Piece::Rook, Piece::Queen, Piece::King })
            {
                auto index = zobristPieceIndex (color, piece);
                CHECK( index >= 0 );
                CHECK( index <= max_valid_index );
            }
        }
    }

    SUBCASE( "Different colored pieces at same square have different hashes" )
    {
        Coord h3 = coordParse ("h3");

        ColoredPiece white_knight = ColoredPiece::make (Color::White, Piece::Knight);
        ColoredPiece black_bishop = ColoredPiece::make (Color::Black, Piece::Bishop);

        auto white_hash = boardCodeHash (h3, white_knight);
        auto black_hash = boardCodeHash (h3, black_bishop);

        CHECK( white_hash != black_hash );
    }

    SUBCASE( "initializeBoardCodes produces values indexed by zobristPieceIndex" )
    {
        for (auto color : { Color::White, Color::Black })
        {
            for (auto piece : { Piece::Pawn, Piece::Knight, Piece::Bishop,
                                Piece::Rook, Piece::Queen, Piece::King })
            {
                auto piece_index = zobristPieceIndex (color, piece);
                for (auto square = 0; square < Num_Squares; square++)
                {
                    auto hash_value = Hash_Code_Table[piece_index * Num_Squares + square];
                    CHECK( hash_value != 0 );
                }
            }
        }
    }
}
