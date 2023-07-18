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

        [[nodiscard]] auto get_code () const& noexcept -> const BoardCode&
        {
            return my_code;
        }
        void get_code () const&& = delete;

        [[nodiscard]] auto get_material () const& noexcept
            -> const Material&
        {
            return my_material;
        }
        void get_material () const&& = delete;

        [[nodiscard]] auto get_position () const& noexcept
            -> const Position&
        {
            return my_position;
        }
        void get_position () const&& = delete;

        [[nodiscard]] auto to_fen_string (Color turn) const -> string;
        [[nodiscard]] auto castled_string (Color color) const -> string;

        // Throws an exception if the move couldn't be applied.
        [[nodiscard]] auto with_move (Color who, Move move) const -> Board;

        [[nodiscard]] auto get_king_position (Color who) const
            -> Coord
        {
            return my_king_pos[color_index (who)];
        }

        [[nodiscard]] auto get_castling_eligibility (Color who) const -> CastlingEligibility
        {
            return my_code.castle_state (who);
        }

        [[nodiscard]] auto able_to_castle (Color who, CastlingEligibility castle_types) const
            -> bool
        {
            auto castle_state = get_castling_eligibility (who);
            auto castle_bits = castle_state.underlying_value ();
            bool neg_not_set = ((~castle_bits) & castle_types.underlying_value ()) != 0;

            return neg_not_set;
        }

        [[nodiscard]] auto is_en_passant_vulnerable (Color who) const noexcept -> bool
        {
            return my_code.en_passant_target (who) != No_En_Passant_Coord;
        }

        [[nodiscard]] auto get_current_turn () const -> Color
        {
            return my_code.current_turn ();
        }

        [[nodiscard]] auto get_en_passant_target (Color who) const noexcept -> Coord
        {
            return my_code.en_passant_target (who);
        }

        [[nodiscard]] auto get_en_passant_target (ColorIndex who) const noexcept -> Coord
        {
            return get_en_passant_target (color_from_color_index (who));
        }

        [[nodiscard]] auto get_en_passant_targets () const noexcept -> EnPassantTargets
        {
            return my_code.en_passant_targets ();
        }

        void randomize_positions ();

        void set_king_position (Color who, Coord pos)
        {
            my_king_pos[color_index (who)] = pos;
        }

        void remove_castling_eligibility (Color who, CastlingEligibility removed_castle_states)
        {
            CastlingEligibility orig_castle_state = get_castling_eligibility (who);
            my_code.set_castle_state (who, orig_castle_state | removed_castle_states);
        }

        void undo_castle_change (Color who, CastlingEligibility castle_state)
        {
            my_code.set_castle_state (who, castle_state);
        }

        void set_castle_state (Color who, CastlingEligibility new_state)
        {
            my_code.set_castle_state (who, new_state);
        }

        void set_en_passant_target (ColorIndex who, Coord target) noexcept
        {
            my_code.set_en_passant_target (color_from_color_index (who), target);
        }

        void set_en_passant_target (Color who, Coord target) noexcept
        {
            set_en_passant_target (color_index (who), target);
        }

        void set_current_turn (Color who)
        {
            my_code.set_current_turn (who);
        }

        [[nodiscard]] auto get_board_code () const& -> const BoardCode&
        {
            return my_code;
        }
        void get_board_code () const&& = delete;

        void update_move_clock (Color who, Piece orig_src_piece_type, Move mv)
        {
            if (mv.is_any_capturing () || orig_src_piece_type == Piece::Pawn)
                my_half_move_clock = 0;
            else
                my_half_move_clock++;

            if (who == Color::Black)
                my_full_move_clock++;
        }

        [[nodiscard]] static auto all_coords () -> CoordIterator
        {
            return CoordIterator {};
        }

        void set_piece (int8_t row, int8_t col, ColoredPiece piece)
        {
            my_squares[coord_index (row, col)] = piece;
        }

        void set_piece (Coord coord, ColoredPiece piece)
        {
            my_squares[coord_index (coord)] = piece;
        }

        [[nodiscard]] auto find_first_coord_with_piece (ColoredPiece piece,
                                                        Coord starting_at = First_Coord) const
            -> optional<Coord>;

    private:
        void make_move (Color who, Move move);
        auto apply_for_en_passant (Color who, Coord src, Coord dst) -> ColoredPiece;
        auto get_castling_rook_move (Move move, Color who) -> Move;
        void apply_for_castling_move (Color who, Move king_move,
                                      [[maybe_unused]] Coord src, [[maybe_unused]] Coord dst);
        void apply_for_king_move (Color who, [[maybe_unused]] Coord src, Coord dst);
        void apply_for_rook_capture (Color opponent, ColoredPiece dst_piece, Coord src, Coord dst);
        void apply_for_rook_move (Color player, ColoredPiece src_piece,
                                  Move move, Coord src, Coord dst);
        void update_en_passant_eligibility (Color who, ColoredPiece src_piece, Move move);

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

    constexpr auto coord_color (Coord coord) -> Color
    {
        int parity = (Row (coord) % 2 + Column (coord) % 2) % 2;
        return color_from_color_index (gsl::narrow_cast<int8_t> (parity));
    }

    // white moves up (-)
    // black moves down (+)
    template <class IntegerType = int8_t>
    constexpr IntegerType pawn_direction (Color color)
    {
        static_assert (std::is_integral_v<IntegerType>);
        assert (color == Color::Black || color == Color::White);
        int8_t color_as_int = to_int8 (color);
        return gsl::narrow_cast<IntegerType>(-1 + 2 * (color_as_int - 1));
    }
}

#endif // WISDOM_CHESS_BOARD_HPP
