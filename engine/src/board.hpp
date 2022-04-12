#ifndef WISDOM_CHESS_BOARD_H_
#define WISDOM_CHESS_BOARD_H_

#include "board_code.hpp"
#include "coord.hpp"
#include "coord_iterator.hpp"
#include "generate.hpp"
#include "global.hpp"
#include "material.hpp"
#include "move.hpp"
#include "piece.hpp"
#include "position.hpp"
#include "transposition_table.hpp"
#include "variation_glimpse.hpp"

namespace wisdom
{
    struct BoardPositions
    {
        int8_t rank;
        Color piece_color;
        vector<Piece> pieces;
    };

    class BoardBuilder;

    class Board
    {
    private:
        // The representation of the board.
        ColoredPiece my_squares[Num_Rows][Num_Columns];

        // positions of the kings.
        Coord my_king_pos[Num_Players];

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

    public:
        int8_t raw_squares[Num_Rows][Num_Columns];

        Board ();

        Board (const Board& board) = default;

        explicit Board (const vector<BoardPositions> &positions);

        friend bool operator== (const Board &a, const Board &b);

        void print () const;

        [[nodiscard]] constexpr auto piece_at (int row, int col) const
            -> ColoredPiece
        {
            return this->my_squares[row][col];
        }

        [[nodiscard]] constexpr auto piece_at (Coord coord) const
            -> ColoredPiece
        {
            return this->my_squares[Row (coord)][Column (coord)];
        }

        void copy_squares (ColoredPiece copy_to_squares[Num_Rows][Num_Columns]) const
        {
            std::copy (&my_squares[0][0], &my_squares[Num_Rows - 1][Num_Columns - 1] + 1,
                       &copy_to_squares[0][0]);
        }

        void set_piece (int8_t row, int8_t col, ColoredPiece piece)
        {
            this->my_squares[row][col] = piece;
        }

        void set_piece (Coord coord, ColoredPiece piece)
        {
            this->my_squares[Row (coord)][Column (coord)] = piece;
        }

        void print_to_file (std::ostream& out) const;

        void dump () const;

        void update_move_clock (Color who, Piece orig_src_piece_type, Move mv, UndoMove& undo_state)
        {
            undo_state.half_move_clock = this->my_half_move_clock;
            if (is_any_capturing_move (mv) || orig_src_piece_type == Piece::Pawn)
                this->my_half_move_clock = 0;
            else
                this->my_half_move_clock++;

            if (who == Color::Black)
            {
                this->my_full_move_clock++;
                undo_state.full_move_clock_updated = true;
            }
        }

        void restore_move_clock (const UndoMove& undo_state)
        {
            this->my_half_move_clock = undo_state.half_move_clock;
            if (undo_state.full_move_clock_updated)
                this->my_full_move_clock--;
        }

        [[nodiscard]] auto get_half_move_clock () const noexcept
            -> int
        {
            return my_half_move_clock;
        }

        [[nodiscard]] auto get_full_move_clock () const noexcept
            -> int
        {
            return my_full_move_clock;
        }

        // Convert the board to a string.
        [[nodiscard]] auto to_string () const -> string;

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

        auto make_move (Color who, Move move) -> UndoMove;
        void take_back (Color who, Move move, const UndoMove& undo_state);

        [[nodiscard]] auto get_king_position (Color who) const
            -> Coord
        {
            return my_king_pos[color_index (who)];
        }

        void set_king_position (Color who, Coord pos)
        {
            my_king_pos[color_index (who)] = pos;
        }

        [[nodiscard]] auto able_to_castle (Color who, CastlingState castle_type) const
            -> bool
        {
            assert (castle_type != Castle_Previously_None);

            auto castle_state = get_castle_state (who);
            bool neg_not_set = ((~castle_state) & castle_type) != 0;

            return neg_not_set;
        }

        void apply_castle_change (Color who, CastlingState castle_state)
        {
            my_code.set_castle_state (who, castle_state);
        }

        void undo_castle_change (Color who, CastlingState castle_state)
        {
            my_code.set_castle_state (who, castle_state);
        }

        [[nodiscard]] auto get_castle_state (Color who) const -> CastlingState
        {
            return my_code.castle_state (who);
        }

        void set_castle_state (Color who, CastlingState new_state)
        {
            my_code.set_castle_state (who, new_state);
        }

        [[nodiscard]] auto is_en_passant_vulnerable (Color who) const noexcept -> bool
        {
            return my_code.en_passant_target (who) != No_En_Passant_Coord;
        }

        [[nodiscard]] auto get_current_turn () const -> Color
        {
            return my_code.current_turn ();
        }

        void set_current_turn (Color who)
        {
            my_code.set_current_turn (who);
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

        void set_en_passant_target (Color who, Coord target) noexcept
        {
            set_en_passant_target (color_index (who), target);
        }

        void set_en_passant_target (ColorIndex who, Coord target) noexcept
        {
            my_code.set_en_passant_target (color_from_color_index (who), target);
        }

        friend class BoardBuilder;

        [[nodiscard]] const ColoredPiece* squares_ptr () const
        {
            return &my_squares[0][0];
        }

        [[nodiscard]] static auto initial_board_position () -> vector<BoardPositions>;

        void randomize_positions ();
    };

    // white moves up (-)
    // black moves down (+)
    template <class T = int8_t>
    constexpr T pawn_direction (Color color)
    {
        assert (color == Color::Black || color == Color::White);
        int8_t color_as_int = to_int8 (color);
        return gsl::narrow_cast<T>(-1 + 2 * (color_as_int - 1));
    }

}

#endif // WISDOM_CHESS_BOARD_H_
