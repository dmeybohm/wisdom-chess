#ifndef WISDOM_CHESS_BOARD_H_
#define WISDOM_CHESS_BOARD_H_

#include "global.hpp"
#include "coord.hpp"
#include "coord_iterator.hpp"
#include "move.hpp"
#include "piece.hpp"
#include "material.hpp"
#include "position.hpp"
#include "board_code.hpp"
#include "transposition_table.hpp"
#include "generate.hpp"
#include "variation_glimpse.hpp"

namespace wisdom
{
    struct BoardPositions
    {
        int rank;
        Color piece_color;
        vector<Piece> pieces;
    };

    class BoardBuilder;

    class Board
    {
    private:
        // The representation of the board.
        ColoredPiece my_squares[Num_Rows][Num_Columns];

        // castle state of the board.
        CastlingState my_castled[Num_Players];

        // positions of the kings.
        Coord my_king_pos[Num_Players];

        // Keep track of hashing information.
        BoardCode my_code;

        // Keep track of the material on the board.
        Material my_material;

        // Keep track of the positions on the board.
        Position my_position;

        // The columns which are eligible for en_passant.
        Coord my_en_passant_target[Num_Players];

        // Number of half moves since pawn or capture.
        int my_half_move_clock = 0;

        // Number of full moves, updated after black moves.
        int my_full_move_clock = 1;

    public:
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
            return this->my_squares[coord.row][coord.col];
        }

        void set_piece (int row, int col, ColoredPiece piece)
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
        void take_back (Color who, Move move, UndoMove undo_state);

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
            ColorIndex c_index = color_index (who);

            bool didnt_castle = my_castled[c_index] != Castle_Castled;
            bool neg_not_set = ((~my_castled[c_index]) & castle_type) != 0;

            return didnt_castle && neg_not_set;
        }

        void apply_castle_change (Color who, CastlingState castle_state)
        {
            ColorIndex index = color_index (who);
            my_castled[index] = castle_state;
        }

        void undo_castle_change (Color who, CastlingState castle_state)
        {
            ColorIndex index = color_index (who);
            my_castled[index] = castle_state;
        }

        [[nodiscard]] auto get_castle_state (Color who) const -> CastlingState
        {
            ColorIndex index = color_index (who);
            return my_castled[index];
        }

        void set_castle_state (Color who, CastlingState new_state)
        {
            ColorIndex index = color_index (who);
            my_castled[index] = new_state;
        }

        [[nodiscard]] auto is_en_passant_vulnerable (Color who) const noexcept -> bool
        {
            return my_en_passant_target[color_index (who)] != No_En_Passant_Coord;
        }

        [[nodiscard]] auto get_en_passant_target (Color who) const noexcept -> Coord
        {
            return my_en_passant_target[color_index (who)];
        }

        [[nodiscard]] auto get_en_passant_target (ColorIndex who) const noexcept -> Coord
        {
            return my_en_passant_target[who];
        }

        void set_en_passant_target (Color who, Coord target) noexcept
        {
            my_en_passant_target[color_index (who)] = target;
        }

        void set_en_passant_target (ColorIndex who, Coord target) noexcept
        {
            my_en_passant_target[who] = target;
        }

        friend class BoardBuilder;
    };

    // white moves up (-)
    // black moves down (+)
    constexpr int pawn_direction (Color color)
    {
        switch (color)
        {
            case Color::Black: return 1;
            case Color::White: return -1;
            case Color::None: throw Error { "Invalid color in pawn_direction()" };
        }
        std::terminate ();
    }

}

#endif // WISDOM_CHESS_BOARD_H_
