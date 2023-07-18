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
        Board ();

        Board (const Board& board) = default;

        explicit Board (const BoardBuilder& builder);

        friend bool operator== (const Board& a, const Board& b);

        void print () const;

        [[nodiscard]] constexpr auto pieceAt (int row, int col) const
            -> ColoredPiece
        {
            return my_squares[coord_index (row, col)];
        }

        [[nodiscard]] constexpr auto pieceAt (Coord coord) const
            -> ColoredPiece
        {
            return my_squares[coord_index (coord)];
        }

        void printToFile (std::ostream& out) const;

        void dump () const;

        [[nodiscard]] auto getHalfMoveClock () const noexcept
            -> int
        {
            return my_half_move_clock;
        }

        [[nodiscard]] auto getFullMoveClock () const noexcept
            -> int
        {
            return my_full_move_clock;
        }

        // Convert the board to a string.
        [[nodiscard]] auto toString () const -> string;

        [[nodiscard]] auto getCode () const& noexcept -> const BoardCode&
        {
            return my_code;
        }
        void get_code () const&& = delete;

        [[nodiscard]] auto getMaterial () const& noexcept
            -> const Material&
        {
            return my_material;
        }
        void getMaterial () const&& = delete;

        [[nodiscard]] auto getPosition () const& noexcept
            -> const Position&
        {
            return my_position;
        }
        void getPosition () const&& = delete;

        [[nodiscard]] auto toFenString (Color turn) const -> string;
        [[nodiscard]] auto castledString (Color color) const -> string;

        // Throws an exception if the move couldn't be applied.
        [[nodiscard]] auto withMove (Color who, Move move) const -> Board;

        [[nodiscard]] auto getKingPosition (Color who) const
            -> Coord
        {
            return my_king_pos[color_index (who)];
        }

        [[nodiscard]] auto getCastlingEligibility (Color who) const -> CastlingEligibility
        {
            return my_code.castle_state (who);
        }

        [[nodiscard]] auto ableToCastle (Color who, CastlingEligibility castle_types) const
            -> bool
        {
            auto castle_state = getCastlingEligibility (who);
            auto castle_bits = castle_state.underlying_value ();
            bool neg_not_set = ((~castle_bits) & castle_types.underlying_value ()) != 0;

            return neg_not_set;
        }

        [[nodiscard]] auto isEnPassantVulnerable (Color who) const noexcept -> bool
        {
            return my_code.en_passant_target (who) != No_En_Passant_Coord;
        }

        [[nodiscard]] auto getCurrentTurn () const -> Color
        {
            return my_code.current_turn ();
        }

        [[nodiscard]] auto getEnPassantTarget (Color who) const noexcept -> Coord
        {
            return my_code.en_passant_target (who);
        }

        [[nodiscard]] auto getEnPassantTarget (ColorIndex who) const noexcept -> Coord
        {
            return getEnPassantTarget (color_from_color_index (who));
        }

        [[nodiscard]] auto getEnPassantTargets () const noexcept -> EnPassantTargets
        {
            return my_code.en_passant_targets ();
        }

        void randomizePositions ();

        void setKingPosition (Color who, Coord pos)
        {
            my_king_pos[color_index (who)] = pos;
        }

        void removeCastlingEligibility (Color who, CastlingEligibility removed_castle_states)
        {
            CastlingEligibility orig_castle_state = getCastlingEligibility (who);
            my_code.set_castle_state (who, orig_castle_state | removed_castle_states);
        }

        void undoCastleChange (Color who, CastlingEligibility castle_state)
        {
            my_code.set_castle_state (who, castle_state);
        }

        void setCastleState (Color who, CastlingEligibility new_state)
        {
            my_code.set_castle_state (who, new_state);
        }

        void setEnPassantTarget (ColorIndex who, Coord target) noexcept
        {
            my_code.set_en_passant_target (color_from_color_index (who), target);
        }

        void setEnPassantTarget (Color who, Coord target) noexcept
        {
            setEnPassantTarget (color_index (who), target);
        }

        void setCurrentTurn (Color who)
        {
            my_code.set_current_turn (who);
        }

        [[nodiscard]] auto getBoardCode () const& -> const BoardCode&
        {
            return my_code;
        }
        void getBoardCode () const&& = delete;

        void updateMoveClock (Color who, Piece orig_src_piece_type, Move mv)
        {
            if (mv.is_any_capturing () || orig_src_piece_type == Piece::Pawn)
                my_half_move_clock = 0;
            else
                my_half_move_clock++;

            if (who == Color::Black)
                my_full_move_clock++;
        }

        [[nodiscard]] static auto allCoords () -> CoordIterator
        {
            return CoordIterator {};
        }

        void setPiece (int8_t row, int8_t col, ColoredPiece piece)
        {
            my_squares[coord_index (row, col)] = piece;
        }

        void setPiece (Coord coord, ColoredPiece piece)
        {
            my_squares[coord_index (coord)] = piece;
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

        // positions of the kings.
        array<Coord, Num_Players> my_king_pos;

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
    };

    constexpr auto coordColor (Coord coord) -> Color
    {
        int parity = (Row (coord) % 2 + Column (coord) % 2) % 2;
        return color_from_color_index (gsl::narrow_cast<int8_t> (parity));
    }

    // white moves up (-)
    // black moves down (+)
    template <class IntegerType = int8_t>
    constexpr IntegerType pawnDirection (Color color)
    {
        static_assert (std::is_integral_v<IntegerType>);
        assert (color == Color::Black || color == Color::White);
        int8_t color_as_int = to_int8 (color);
        return gsl::narrow_cast<IntegerType>(-1 + 2 * (color_as_int - 1));
    }
}

#endif // WISDOM_CHESS_BOARD_HPP
