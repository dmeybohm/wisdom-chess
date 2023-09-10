#ifndef WISDOM_CHESS_BOARD_BUILDER_HPP
#define WISDOM_CHESS_BOARD_BUILDER_HPP

#include "global.hpp"
#include "piece.hpp"
#include "move.hpp"

namespace wisdom
{
    class BoardBuilderError : public Error
    {
    public:
        explicit BoardBuilderError (const string &message) :
                Error (message)
        {}
    };

    class BoardBuilder final
    {
    public:
        using PieceRow = array<Piece, Num_Columns>;

        struct CoordAndPiece
        {
            czstring coord;
            Piece piece_type;
        };

        constexpr BoardBuilder() : my_squares { emptySquares() }
        {}

        static auto fromDefaultPosition() -> BoardBuilder;

        static auto fromRandomPosition() -> BoardBuilder;

        [[nodiscard]] static constexpr auto emptySquares() -> array<ColoredPiece, Num_Squares>
        {
            array<ColoredPiece, Num_Squares> result {};
            for (auto i = 0; i < Num_Squares; i++)
                result[i] = Piece_And_Color_None;
            return result;
        }

        static constexpr PieceRow Default_Piece_Row = {
            Piece::Rook, Piece::Knight, Piece::Bishop, Piece::Queen, Piece::King,
            Piece::Bishop, Piece::Knight, Piece::Rook,
        };

        void addPiece (const string& coord_str, Color who, Piece piece_type);

        void addPiece (int row, int col, Color who, Piece piece_type);

        void addPieces (Color who, const vector<CoordAndPiece>& pieces);

        void addRowOfSameColor (int row, Color who, const PieceRow& piece_types);

        void addRowOfSameColor (const string& coord_str, Color who, const PieceRow& piece_types);

        void addRowOfSameColorAndPiece (int row, Color who, Piece piece_type);

        void addRowOfSameColorAndPiece (const string& coord_str, Color who, Piece piece_type);

        void setEnPassantTarget (Color vulnerable_color, const string& coord_str);

        void setCastling (Color who, CastlingEligibility state);

        void setCurrentTurn (Color who);

        void setHalfMovesClock (int new_half_moves_clock);

        void setFullMoves (int new_full_moves);

        [[nodiscard]] auto getSquares() const& -> const array<ColoredPiece, Num_Squares>&
        {
            return my_squares;
        }
        void getSquares() const&& = delete;

        [[nodiscard]] auto pieceAt (Coord coord) const -> ColoredPiece
        {
            assert (coord.index() < Num_Squares);
            return my_squares[coord.index()];
        }

        [[nodiscard]] auto getCurrentTurn() const -> Color
        {
            return my_current_turn;
        }

        [[nodiscard]] auto getEnPassantTargets() const -> array<Coord, Num_Players>
        {
            return my_en_passant_targets;
        }

        [[nodiscard]] auto getCastleState (Color who) const -> CastlingEligibility
        {
            if (my_castle_states[colorIndex (who)])
                return *my_castle_states[colorIndex (who)];
            return calculateCastleStateFromPosition (who);
        }

        [[nodiscard]] auto getKingPosition (Color who) -> Coord
        {
            auto index = colorIndex (who);
            if (!my_king_positions[index])
                throw BoardBuilderError { "Missing king position in constructing board." };
            return *my_king_positions[index];
        }

        [[nodiscard]] auto getKingPositions() const -> array<Coord, Num_Players>
        {
            if (!my_king_positions[Color_Index_White] || !my_king_positions[Color_Index_Black])
                throw BoardBuilderError { "Missing king position in constructing board." };
            return { *my_king_positions[Color_Index_White], *my_king_positions[Color_Index_Black] };
        }

        [[nodiscard]] auto getHalfMoveClock() const -> int
        {
            return my_half_moves_clock;
        }

        [[nodiscard]] auto getFullMoveClock() const -> int
        {
            return my_full_moves;
        }

    private:
        [[nodiscard]] auto calculateCastleStateFromPosition (Color who) const -> CastlingEligibility;

        array<ColoredPiece, Num_Squares> my_squares;
        array<Coord, Num_Players> my_en_passant_targets {
            No_En_Passant_Coord, No_En_Passant_Coord
        };

        array<optional<CastlingEligibility>, Num_Players> my_castle_states {
            nullopt, nullopt
        };

        Color my_current_turn = Color::White;
        int my_half_moves_clock = 0;
        int my_full_moves = 0;
        array<optional<Coord>, Num_Players> my_king_positions {
            nullopt, nullopt
        };
    };

}

#endif //WISDOM_CHESS_BOARD_BUILDER_HPP
