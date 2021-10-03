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

///////////////////////////////////////////////

namespace wisdom
{
    struct BoardPositions
    {
        int rank;
        Color piece_color;
        vector<Piece> pieces;
    };

    class Board
    {
    private:
        // The representation of the board.
        ColoredPiece my_squares[Num_Rows][Num_Columns];

        // castle state of the board.
        CastlingState castled[Num_Players];

    public:
        // positions of the kings.
        Coord king_pos[Num_Players];

        // Keep track of hashing information.
        BoardCode code;

        // Keep track of the material on the board.
        Material material;

        // Keep track of the positions on the board.
        Position position;

        // The columns which are eligible for en_passant.
        Coord en_passant_target[Num_Players];

        // Number of half moves since pawn or capture.
        int half_move_clock = 0;

        // Number of full moves, updated after black moves.
        int full_moves = 1;

        Board ();

        Board (const Board &board) = default;

        explicit Board (const vector<BoardPositions> &positions);

        void print () const;

        [[nodiscard]] constexpr auto piece_at (int row, int col) const -> ColoredPiece
        {
            return this->my_squares[row][col];
        }

        [[nodiscard]] constexpr auto piece_at (Coord coord) const -> ColoredPiece
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

        void print_to_file (std::ostream &out) const;

        void dump () const;

        void update_move_clock (Color who, Piece orig_src_piece_type, Move mv, UndoMove &undo_state)
        {
            undo_state.half_move_clock = this->half_move_clock;
            if (is_any_capturing_move (mv) || orig_src_piece_type == Piece::Pawn)
                this->half_move_clock = 0;
            else
                this->half_move_clock++;

            if (who == Color::Black)
            {
                this->full_moves++;
                undo_state.full_move_clock_updated = true;
            }
        }

        void restore_move_clock (const UndoMove &undo_state)
        {
            this->half_move_clock = undo_state.half_move_clock;
            if (undo_state.full_move_clock_updated)
                this->full_moves--;
        }

        // Convert the board to a string.
        [[nodiscard]] auto to_string () const -> string;

        [[nodiscard]] auto get_code () const& -> const BoardCode&;

        [[nodiscard]] auto to_fen_string (Color turn) const -> string;
        [[nodiscard]] auto castled_string (Color color) const -> string;

        auto make_move (Color who, Move move) -> UndoMove;
        void take_back (Color who, Move move, UndoMove undo_state);

        [[nodiscard]] bool able_to_castle (Color who, CastlingState castle_type) const
        {
            ColorIndex c_index = color_index (who);

            bool didnt_castle = castled[c_index] != Castle_Castled;
            bool neg_not_set = ((~castled[c_index]) & castle_type) != 0;

            return didnt_castle && neg_not_set;
        }

        void apply_castle_change (Color who, CastlingState castle_state)
        {
            ColorIndex index = color_index (who);
            castled[index] = castle_state;
        }

        void undo_castle_change (Color who, CastlingState castle_state)
        {
            ColorIndex index = color_index (who);
            castled[index] = castle_state;
        }

        [[nodiscard]] auto get_castle_state (Color who) const -> CastlingState
        {
            ColorIndex index = color_index (who);
            return castled[index];
        }

        void set_castle_state (Color who, CastlingState new_state)
        {
            ColorIndex index = color_index (who);
            castled[index] = new_state;
        }

        [[nodiscard]] bool is_en_passant_vulnerable (Color who)
        {
            return en_passant_target[color_index (who)] != No_En_Passant_Coord;
        }
    };

    constexpr auto piece_at (const Board &board, int row, int col) -> ColoredPiece
    {
        return board.piece_at (row, col);
    }

    constexpr auto piece_at (const Board &board, Coord coord) -> ColoredPiece
    {
        return piece_at (board, coord.row, coord.col);
    }

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

    [[nodiscard]] static inline auto king_position (const Board &board, Color who) -> Coord
    {
        return board.king_pos[color_index (who)];
    }

    static inline void king_position_set (Board &board, Color who, Coord pos)
    {
        board.king_pos[color_index (who)] = pos;
    }

    bool board_equals (const Board &a, const Board &b);
}

#endif // WISDOM_CHESS_BOARD_H_
