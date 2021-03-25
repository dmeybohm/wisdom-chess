#ifndef EVOLVE_CHESS_BOARD_H_
#define EVOLVE_CHESS_BOARD_H_

#include <cassert>
#include <vector>
#include <algorithm>

#include "global.h"
#include "coord.h"
#include "coord_iterator.hpp"
#include "move.h"
#include "piece.h"
#include "material.h"
#include "position.h"
#include "board_code.hpp"

///////////////////////////////////////////////

namespace wisdom
{
    struct move_tree;

    struct BoardPositions
    {
        int rank;
        Color piece_color;
        std::vector<Piece> pieces;
    };

    struct Board
    {
        ColoredPiece squares[Num_Rows][Num_Columns];

        // positions of the kings
        Coord king_pos[Num_Players];

        // castle state of the board
        CastlingState castled[Num_Players];

        // keep track of hashing information
        BoardCode code;

        // keep track of the material on the board
        Material material;

        // keep track of the positions on the board
        Position position;

        // The columns which are eligible for en_passant
        Coord en_passant_target[Num_Players];

        // Number of half moves since pawn or capture.
        int16_t half_move_clock = 0;

        // Number of full moves, updated after black moves.
        int16_t full_moves = 0;

        Board ();

        Board (const Board &board) = default;

        explicit Board (const std::vector<BoardPositions> &positions);

        void print () const;

        void print_to_file (std::ostream &out) const;

        void dump () const;

        void update_half_move_clock (Piece orig_src_piece_type, Move mv, UndoMove &undo_state)
        {
            undo_state.half_move_clock = this->half_move_clock;
            if (is_capture_move (mv) || orig_src_piece_type == Piece::Pawn)
                this->half_move_clock = 0;
            else
                this->half_move_clock++;

        }

        void restore_half_move_clock (const UndoMove &undo_state)
        {
            this->half_move_clock = undo_state.half_move_clock;
        }

        [[nodiscard]] std::string to_string () const;
    };

///////////////////////////////////////////////

    static inline ColoredPiece piece_at (const Board &board, int8_t row, int8_t col)
    {
        assert (is_valid_row (row) && is_valid_column (col));
        return board.squares[row][col];
    }

    static inline ColoredPiece piece_at (const Board &board, Coord coord)
    {
        return piece_at (board, coord.row, coord.col);
    }

///////////////////////////////////////////////

// white moves up (-)
// black moves down (+)
    static inline int8_t pawn_direction (Color color)
    {
        assert (color == Color::White || color == Color::Black);
        return color == Color::Black ? 1 : -1;
    }

    static inline bool need_pawn_promotion (int8_t row, Color who)
    {
        assert (is_color_valid (who));
        switch (who)
        {
            case Color::White: return 0 == row;
            case Color::Black: return 7 == row;
            default: abort ();
        }
    }

    constexpr int able_to_castle (const Board &board, Color who, CastlingState castle_type)
    {
        ColorIndex c_index = color_index (who);

        int didnt_castle = !!(board.castled[c_index] != CASTLE_CASTLED);
        int neg_not_set = !!(((~board.castled[c_index]) & castle_type) != 0);

        return didnt_castle && neg_not_set;
    }

    constexpr CastlingState board_get_castle_state (const struct Board &board, Color who)
    {
        ColorIndex index = color_index (who);
        return board.castled[index];
    }

    static inline void board_apply_castle_change (Board &board, Color who, CastlingState castle_state)
    {
        ColorIndex index = color_index (who);
        board.castled[index] = castle_state;
    }

    static inline void board_undo_castle_change (Board &board, Color who, CastlingState castle_state)
    {
        ColorIndex index = color_index (who);
        board.castled[index] = castle_state;
    }

    static inline Coord king_position (const Board &board, Color who)
    {
        return board.king_pos[color_index (who)];
    }

    static inline void king_position_set (Board &board, Color who, Coord pos)
    {
        board.king_pos[color_index (who)] = pos;
    }

    static inline void board_set_piece (Board &board, Coord place, ColoredPiece piece)
    {
        board.squares[ROW (place)][COLUMN (place)] = piece;
    }

    constexpr bool is_en_passant_vulnerable (const Board &board, Color who)
    {
        return board.en_passant_target[color_index (who)] != no_en_passant_coord;
    }

    static inline bool board_equals (const Board &a, const Board &b)
    {
        for (auto coord : all_coords_iterator)
        {
            if (a.squares[coord.row][coord.col] != b.squares[coord.row][coord.col])
                return false;
        }

        // todo check more
        return true;
    }
}

#endif // EVOLVE_CHESS_BOARD_H_
