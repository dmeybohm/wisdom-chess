#pragma once

#include "board_code.hpp"
#include "coord.hpp"
#include "generate.hpp"
#include "global.hpp"
#include "material.hpp"
#include "move.hpp"
#include "piece.hpp"
#include "position.hpp"

namespace wisdom
{
    class BoardBuilder;

    class Board
    {
    public:
        Board();

        Board (const Board& board) = default;

        explicit Board (const BoardBuilder& builder);

        friend auto
        operator== (const Board& a, const Board& b)
            -> bool
        {
            return a.my_code == b.my_code && a.my_squares == b.my_squares;
        }

        [[nodiscard]] constexpr auto
        pieceAt (int row, int col) const
            -> ColoredPiece
        {
            Coord coord = Coord::make (row, col);
            return my_squares[coord.index()];
        }

        [[nodiscard]] constexpr auto
        pieceAt (Coord coord) const
            -> ColoredPiece
        {
            return my_squares[coord.index()];
        }

        friend auto
        operator<< (std::ostream& os, const Board& board)
            -> std::ostream&;

        void dump() const;

        [[nodiscard]] auto
        getHalfMoveClock() const noexcept
            -> int
        {
            return my_half_move_clock;
        }

        [[nodiscard]] auto
        getFullMoveClock() const noexcept
            -> int
        {
            return my_full_move_clock;
        }

        // Convert the board to a string.
        [[nodiscard]] auto
        asString() const
            -> string;

        [[nodiscard]] auto
        getCode() const noexcept
            -> BoardCode
        {
            return my_code;
        }

        [[nodiscard]] auto
        getMaterial() const& noexcept
            -> const Material&
        {
            return my_material;
        }
        void getMaterial() const&& = delete;

        [[nodiscard]] auto
        getPosition() const& noexcept
            -> const Position&
        {
            return my_position;
        }
        void getPosition() const&& = delete;

        [[nodiscard]] auto
        toFenString (Color turn) const
            -> string;
        [[nodiscard]] auto
        castledString (Color color) const
            -> string;

        // Create a new board with the move applied:
        [[nodiscard]] auto
        withMove (Color who, Move move) const
            -> Board;

        // Create a new board with the current turn updated:
        [[nodiscard]] auto
        withCurrentTurn (Color who) const
            -> Board;

        // Randomize and return copy of current board:
        [[nodiscard]] auto
        withRandomPosition() const
            -> Board;

        [[nodiscard]] auto
        getKingPosition (Color who) const noexcept
            -> Coord
        {
            return my_king_pos[colorIndex (who)];
        }

        [[nodiscard]] auto
        getCastlingEligibility (Color who) const noexcept
            -> CastlingEligibility
        {
            return my_code.castleState (who);
        }

        [[nodiscard]] auto
        ableToCastle (Color who, CastlingEligibility castle_types) const noexcept
            -> bool
        {
            // If either/both is passed, check both types.
            auto check_type
                = (castle_types == Either_Side_Eligible || castle_types == Neither_Side_Eligible)
                ? Neither_Side_Eligible
                : castle_types;

            auto castle_state = getCastlingEligibility (who);
            auto castle_bits = toInt (castle_state);
            bool neg_not_set = ((~castle_bits) & toInt (check_type)) != 0;

            return neg_not_set;
        }

        [[nodiscard]] auto isEnPassantVulnerable (Color who) const noexcept -> bool
        {
            auto target = my_code.enPassantTarget();
            return target->vulnerable_color == who;
        }

        [[nodiscard]] auto getCurrentTurn() const -> Color
        {
            return my_code.currentTurn();
        }

        [[nodiscard]] auto
        getEnPassantTarget() const noexcept
            -> optional<EnPassantTarget>
        {
            return my_code.enPassantTarget();
        }

        [[nodiscard]] auto getBoardCode() const -> BoardCode
        {
            return my_code;
        }

        [[nodiscard]] static auto
        allCoords()
            -> CoordIterator
        {
            return CoordIterator {};
        }

        [[nodiscard]] auto
        findFirstCoordWithPiece (
            ColoredPiece piece,
            Coord starting_at = First_Coord
        ) const
            -> optional<Coord>;

    private:
        void makeMove (Color who, Move move);

        auto applyForEnPassant (Color who, Coord src, Coord dst) noexcept -> ColoredPiece;
        void updateEnPassantEligibility (Color who, ColoredPiece src_piece, Move move) noexcept;
        void setEnPassantTarget (Color who, Coord target) noexcept;
        void clearEnPassantTarget() noexcept;

        [[nodiscard]] auto getCastlingRookMove (Move move, Color who) const -> Move;
        void applyForCastlingMove (
            Color who,
            Move king_move,
            [[maybe_unused]] Coord src,
            [[maybe_unused]] Coord dst
        ) noexcept;
        void updateAfterKingMove (Color who, [[maybe_unused]] Coord src, Coord dst);
        void setCastleState (Color who, CastlingEligibility new_state) noexcept;

        void removeCastlingEligibility (
            Color who,
            CastlingEligibility removed_castle_states
        ) noexcept;

        void updateAfterRookCapture (
            Color opponent,
            ColoredPiece dst_piece,
            Coord src,
            Coord dst
        ) noexcept;

        void updateAfterRookMove (
            Color player,
            ColoredPiece src_piece,
            Move move,
            Coord src,
            Coord dst
        ) noexcept;

        void setKingPosition (Color who, Coord pos) noexcept;
        void setPiece (Coord coord, ColoredPiece piece) noexcept;
        void updateMoveClock (Color who, Piece orig_src_piece_type, Move move) noexcept;
        void setCurrentTurn (Color who) noexcept;

    private:
        // The representation of the board.
        array<ColoredPiece, Num_Squares> my_squares;

        // Keep track of hashing information.
        BoardCode my_code;

        // Number of half moves since pawn or capture.
        int my_half_move_clock = 0;

        // Number of full moves, updated after black moves.
        int my_full_move_clock = 1;

        // Keep track of the positions on the board.
        Position my_position;

        // Keep track of the material on the board.
        Material my_material;

        // positions of the kings.
        array<Coord, Num_Players> my_king_pos;
    };

    constexpr auto coordColor (Coord coord) -> Color
    {
        int parity = (coord.row() % 2 + coord.column() % 2) % 2;
        return colorFromColorIndex (gsl::narrow_cast<int8_t> (parity));
    }

    // white moves up (-)
    // black moves down (+)
    template <class IntegerType = int8_t>
    constexpr IntegerType pawnDirection (Color color)
    {
        static_assert (std::is_integral_v<IntegerType>);
        assert (color == Color::Black || color == Color::White);
        int8_t color_as_int = toInt8 (color);
        return gsl::narrow_cast<IntegerType> (-1 + 2 * (color_as_int - 1));
    }
}
