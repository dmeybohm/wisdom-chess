#ifndef WISDOM_CHESS_BOARD_HPP
#define WISDOM_CHESS_BOARD_HPP

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

        friend auto operator== (const Board& a, const Board& b) -> bool
        {
            return a.my_code == b.my_code && a.my_squares == b.my_squares;
        }

        [[nodiscard]] constexpr auto pieceAt (int row, int col) const
            -> ColoredPiece
        {
            return my_squares[coordIndex (row, col)];
        }

        [[nodiscard]] constexpr auto pieceAt (Coord coord) const
            -> ColoredPiece
        {
            return my_squares[coordIndex (coord)];
        }

        friend auto operator<< (std::ostream& os, const Board& board) -> std::ostream&;

        void dump() const;

        [[nodiscard]] auto getHalfMoveClock() const noexcept
            -> int
        {
            return my_half_move_clock;
        }

        [[nodiscard]] auto getFullMoveClock() const noexcept
            -> int
        {
            return my_full_move_clock;
        }

        // Convert the board to a string.
        [[nodiscard]] auto asString() const -> string;

        [[nodiscard]] auto getCode() const noexcept -> BoardCode
        {
            return my_code;
        }

        [[nodiscard]] auto getMaterial() const& noexcept -> const Material&
        {
            return my_material;
        }
        void getMaterial() const&& = delete;

        [[nodiscard]] auto getPosition() const& noexcept -> const Position&
        {
            return my_position;
        }
        void getPosition() const&& = delete;

        [[nodiscard]] auto toFenString (Color turn) const -> string;
        [[nodiscard]] auto castledString (Color color) const -> string;

        // Throws an exception if the move couldn't be applied.
        [[nodiscard]] auto withMove (Color who, Move move) const -> Board;

        [[nodiscard]] auto getKingPosition (Color who) const -> Coord
        {
            return my_king_pos[colorIndex (who)];
        }

        [[nodiscard]] auto getCastlingEligibility (Color who) const -> CastlingEligibility
        {
            return my_code.castleState (who);
        }

        [[nodiscard]] auto ableToCastle (Color who, CastlingEligibility castle_types) const
            -> bool
        {
            auto castle_state = getCastlingEligibility (who);
            auto castle_bits = castle_state.underlying_value();
            bool neg_not_set = ((~castle_bits) & castle_types.underlying_value()) != 0;

            return neg_not_set;
        }

        [[nodiscard]] auto isEnPassantVulnerable (Color who) const noexcept -> bool
        {
            return my_code.enPassantTarget (who) != No_En_Passant_Coord;
        }

        [[nodiscard]] auto getCurrentTurn() const -> Color
        {
            return my_code.currentTurn();
        }

        [[nodiscard]] auto getEnPassantTarget (Color who) const noexcept -> Coord
        {
            return my_code.enPassantTarget (who);
        }

        [[nodiscard]] auto getEnPassantTarget (ColorIndex who) const noexcept -> Coord
        {
            return getEnPassantTarget (colorFromColorIndex (who));
        }

        [[nodiscard]] auto getEnPassantTargets() const noexcept -> EnPassantTargets
        {
            return my_code.enPassantTargets();
        }

        void randomizePositions();

        void setKingPosition (Color who, Coord pos)
        {
            my_king_pos[colorIndex (who)] = pos;
        }

        void removeCastlingEligibility (Color who, CastlingEligibility removed_castle_states)
        {
            CastlingEligibility orig_castle_state = getCastlingEligibility (who);
            my_code.setCastleState (who, orig_castle_state | removed_castle_states);
        }

        void undoCastleChange (Color who, CastlingEligibility castle_state)
        {
            my_code.setCastleState (who, castle_state);
        }

        void setCastleState (Color who, CastlingEligibility new_state)
        {
            my_code.setCastleState (who, new_state);
        }

        void setEnPassantTarget (ColorIndex who, Coord target) noexcept
        {
            my_code.setEnPassantTarget (colorFromColorIndex (who), target);
        }

        void setEnPassantTarget (Color who, Coord target) noexcept
        {
            setEnPassantTarget (colorIndex (who), target);
        }

        void setCurrentTurn (Color who)
        {
            my_code.setCurrentTurn (who);
        }

        [[nodiscard]] auto getBoardCode() const& -> const BoardCode&
        {
            return my_code;
        }
        void getBoardCode() const&& = delete;

        void updateMoveClock (Color who, Piece orig_src_piece_type, Move mv)
        {
            if (mv.isAnyCapturing() || orig_src_piece_type == Piece::Pawn)
                my_half_move_clock = 0;
            else
                my_half_move_clock++;

            if (who == Color::Black)
                my_full_move_clock++;
        }

        [[nodiscard]] static auto allCoords() -> CoordIterator
        {
            return CoordIterator {};
        }

        void setPiece (int8_t row, int8_t col, ColoredPiece piece)
        {
            my_squares[coordIndex (row, col)] = piece;
        }

        void setPiece (Coord coord, ColoredPiece piece)
        {
            my_squares[coordIndex (coord)] = piece;
        }

        [[nodiscard]] auto findFirstCoordWithPiece (ColoredPiece piece,
                                                    Coord starting_at = First_Coord) const
            -> optional<Coord>;

    private:
        void makeMove (Color who, Move move);
        auto applyForEnPassant (Color who, Coord src, Coord dst) -> ColoredPiece;
        auto getCastlingRookMove (Move move, Color who) -> Move;
        void applyForCastlingMove (Color who, Move king_move,
                                   [[maybe_unused]] Coord src, [[maybe_unused]] Coord dst);
        void applyForKingMove (Color who, [[maybe_unused]] Coord src, Coord dst);
        void applyForRookCapture (Color opponent, ColoredPiece dst_piece, Coord src, Coord dst);
        void applyForRookMove (Color player, ColoredPiece src_piece,
                               Move move, Coord src, Coord dst);
        void updateEnPassantEligibility (Color who, ColoredPiece src_piece, Move move);

    private:
        // The representation of the board.
        array<ColoredPiece, Num_Squares> my_squares;

        // Keep track of hashing information.
        BoardCode my_code;

        // Keep track of the material on the board.
        Material my_material;

        // Keep track of the positions on the board.
        Position my_position;

        // Number of half moves since pawn or capture.
        int my_half_move_clock = 0;

        // Number of full moves, updated after black moves.
        int my_full_move_clock = 1;

        // positions of the kings.
        array<Coord, Num_Players> my_king_pos;
    };

    constexpr auto coordColor (Coord coord) -> Color
    {
        int parity = (Row (coord) % 2 + Column (coord) % 2) % 2;
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
        return gsl::narrow_cast<IntegerType>(-1 + 2 * (color_as_int - 1));
    }
}

#endif // WISDOM_CHESS_BOARD_HPP
