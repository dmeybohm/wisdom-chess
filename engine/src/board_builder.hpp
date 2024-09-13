#pragma once

#include "global.hpp"
#include "piece.hpp"
#include "move.hpp"

namespace wisdom
{
    namespace
    {
        [[nodiscard]] consteval auto
        emptySquares()
            -> array<ColoredPiece, Num_Squares>
        {
            array<ColoredPiece, Num_Squares> result {};
            std::fill (std::begin (result), std::end (result), Piece_And_Color_None);
            return result;
        }
    }

    class BoardBuilderError : public Error
    {
    public:
        explicit BoardBuilderError (const string& message)
            : Error (message)
        {
        }
    };

    class BoardBuilder
    {
    public:
        using PieceRow = array<Piece, Num_Columns>;

        struct CoordAndPiece
        {
            czstring coord;
            Piece piece_type;
        };

        constexpr BoardBuilder()
            : my_squares { emptySquares() }
        {
        }

        static constexpr PieceRow Default_Piece_Row = {
            Piece::Rook, Piece::Knight, Piece::Bishop, Piece::Queen,
            Piece::King, Piece::Bishop, Piece::Knight, Piece::Rook,
        };

        static constexpr auto
        fromDefaultPosition()
            -> BoardBuilder
        {
            auto result = BoardBuilder {};
            result.addRowOfSameColor (0, Color::Black, Default_Piece_Row);
            result.addRowOfSameColorAndPiece (1, Color::Black, Piece::Pawn);
            result.addRowOfSameColorAndPiece (6, Color::White, Piece::Pawn);
            result.addRowOfSameColor (7, Color::White, Default_Piece_Row);

            result.setCastling (Color::White, Either_Side_Eligible);
            result.setCastling (Color::Black, Either_Side_Eligible);

            return result;
        }

        constexpr void
        addPiece (string_view coord_str, Color who, Piece piece_type)
        {
            if (coord_str.size() != 2)
                throw BoardBuilderError ("Invalid coordinate string!");

            Coord algebraic = coordParse (coord_str);

            addPiece (algebraic.row(), algebraic.column(), who, piece_type);
        }

        constexpr void
        addPiece (int row, int col, Color who, Piece piece_type)
        {
            if (row < 0 || row >= Num_Rows)
                throw BoardBuilderError ("Invalid row!");

            if (col < 0 || col >= Num_Columns)
                throw BoardBuilderError ("Invalid column!");

            if (piece_type == Piece::None)
                return;

            auto coord = Coord::make (row, col);
            my_squares[coord.index()] = ColoredPiece::make (who, piece_type);

            if (piece_type == Piece::King)
                my_king_positions[colorIndex (who)] = coord;
        }

        constexpr void 
        addPieces (Color who, const std::initializer_list<CoordAndPiece>& pieces)
        {
            for (auto&& it : pieces)
            {
                string_view str_view { it.coord };
                addPiece (str_view, who, it.piece_type);
            }
        }

        constexpr void
        addRowOfSameColorAndPiece (int row, Color who, Piece piece_type)
        {
            for (int col = 0; col < Num_Columns; col++)
                addPiece (row, col, who, piece_type);
        }

        constexpr void
        addRowOfSameColorAndPiece (string_view coord_str, Color who, Piece piece_type)
        {
            Coord coord = coordParse (coord_str);

            for (int col = 0; col < Num_Columns; col++)
                addPiece (coord.row(), col, who, piece_type);
        }

        constexpr void
        addRowOfSameColor (int row, Color who, const PieceRow& piece_types)
        {
            for (auto col = 0; col < Num_Columns; col++)
                addPiece (row, col, who, piece_types[col]);
        }

        constexpr void
        addRowOfSameColor (
            string_view coord_str,
            Color who,
            const PieceRow& piece_types
        ) {
            Coord coord = coordParse (coord_str);

            for (auto col = 0; col < Num_Columns; col++)
                addPiece (coord.row(), col, who, piece_types[col]);
        }

        constexpr void
        setCurrentTurn (Color who)
        {
            my_current_turn = who;
        }

        constexpr void
        setEnPassantTarget (Color vulnerable_color, string_view coord_str)
        {
            my_en_passant_target = {
                .coord = coordParse (coord_str),
                .vulnerable_color = vulnerable_color
            };
        }

        constexpr void
        setCastling (Color who, CastlingEligibility state)
        {
            my_castle_states[colorIndex (who)] = state;
        }

        constexpr void
        setHalfMovesClock (int new_half_moves_clock)
        {
            my_half_moves_clock = new_half_moves_clock;
        }

        constexpr void
        setFullMoves (int new_full_moves)
        {
            my_full_moves = new_full_moves;
        }

        [[nodiscard]] constexpr auto
        getSquares() const&
                -> const array<ColoredPiece, Num_Squares>&
        {
            return my_squares;
        }
        void getSquares() const&& = delete;

        [[nodiscard]] constexpr auto
        pieceAt (Coord coord) const
            -> ColoredPiece
        {
            assert (coord.index() < Num_Squares);
            return my_squares[coord.index()];
        }

        [[nodiscard]] constexpr auto
        getCurrentTurn() const
            -> Color
        {
            return my_current_turn;
        }

        [[nodiscard]] constexpr auto
        getEnPassantTarget() const
            -> optional<EnPassantTarget>
        {
            return my_en_passant_target;
        }

        [[nodiscard]] constexpr auto
        getCastleState (Color who) const
            -> CastlingEligibility
        {
            if (my_castle_states[colorIndex (who)])
                return *my_castle_states[colorIndex (who)];
            return calculateCastleStateFromPosition (who);
        }

        [[nodiscard]] constexpr auto
        getKingPosition (Color who)
            -> Coord
        {
            auto index = colorIndex (who);
            if (!my_king_positions[index])
                throw BoardBuilderError { "Missing king position in constructing board." };
            return *my_king_positions[index];
        }

        [[nodiscard]] constexpr auto
        getKingPositions() const
            -> array<Coord, Num_Players>
        {
            if (!my_king_positions[Color_Index_White] || !my_king_positions[Color_Index_Black])
                throw BoardBuilderError { "Missing king position in constructing board." };
            return { *my_king_positions[Color_Index_White], *my_king_positions[Color_Index_Black] };
        }

        [[nodiscard]] constexpr auto
        getHalfMoveClock() const
            -> int
        {
            return my_half_moves_clock;
        }

        [[nodiscard]] constexpr auto
        getFullMoveClock() const
            -> int
        {
            return my_full_moves;
        }

    private:
        [[nodiscard]] constexpr auto
        calculateCastleStateFromPosition (Color who) const
            -> CastlingEligibility
        {
            auto row = castlingRowForColor (who);
            int8_t king_col = King_Column;

            CastlingEligibility state = Either_Side_Eligible;
            ColoredPiece prospective_king = pieceAt (makeCoord (row, king_col));
            ColoredPiece prospective_queen_rook = pieceAt (makeCoord (row, 0));
            ColoredPiece prospective_king_rook = pieceAt (makeCoord (row, 7));

            if (pieceType (prospective_king) != Piece::King || pieceColor (prospective_king) != who
                || pieceType (prospective_queen_rook) != Piece::Rook
                || pieceColor (prospective_queen_rook) != who)
            {
                state |= CastlingIneligible::Queenside;
            }
            if (pieceType (prospective_king) != Piece::King || pieceColor (prospective_king) != who
                || pieceType (prospective_king_rook) != Piece::Rook
                || pieceColor (prospective_king_rook) != who)
            {
                state |= CastlingIneligible::Kingside;
            }

            return state;
        }

    private:
        array<ColoredPiece, Num_Squares> my_squares;

        int my_half_moves_clock = 0;
        int my_full_moves = 0;
        Color my_current_turn = Color::White;

        array<optional<CastlingEligibility>, Num_Players> my_castle_states {
            nullopt, 
            nullopt 
        };

        array<optional<Coord>, Num_Players> my_king_positions {
            nullopt, 
            nullopt 
        };

        optional<EnPassantTarget> my_en_passant_target { nullopt };
    };

    namespace
    {
        consteval auto
        initDefaultBoardBuilder()
            -> BoardBuilder
        {
            return BoardBuilder::fromDefaultPosition();
        }
    }

    inline constexpr BoardBuilder default_board_builder =
        initDefaultBoardBuilder();
}
