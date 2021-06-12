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
        std::vector<Piece> pieces;
    };

    class Board
    {
    private:
        TranspositionTable my_transpositions;
        int my_transposition_hits = 0;
        int my_transposition_misses = 0;
        int my_transposition_dupe_hashes = 0;

    public:
        // The representation of the board.
        ColoredPiece squares[Num_Rows][Num_Columns];

        // positions of the kings.
        Coord king_pos[Num_Players];

        // castle state of the board.
        CastlingState castled[Num_Players];

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

        explicit Board (const std::vector<BoardPositions> &positions);

        void print () const;

        void print_to_file (std::ostream &out) const;

        void dump () const;

        void update_move_clock (Color who, Piece orig_src_piece_type, Move mv, UndoMove &undo_state)
        {
            undo_state.half_move_clock = this->half_move_clock;
            if (is_capture_move (mv) || orig_src_piece_type == Piece::Pawn)
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
        [[nodiscard]] std::string to_string () const;

        // Add an evaluation for the current board to the transposition table.
        void add_evaluation_to_transposition_table (
                int score, Color who, int relative_depth,
                const VariationGlimpse &variation_glimpse
        );

        // Lookup the current board's score in the transposition table.
        [[nodiscard]] std::optional<RelativeTransposition> check_transposition_table (Color who, int relative_depth);

        // Get a move generator for this board.
        [[nodiscard]] MoveGenerator move_generator ()
        {
            return MoveGenerator { my_transpositions };
        }
    };

    ///////////////////////////////////////////////

    constexpr ColoredPiece piece_at (const Board &board, int row, int col)
    {
        assert (is_valid_row (row) && is_valid_column (col));
        return board.squares[row][col];
    }

    constexpr ColoredPiece piece_at (const Board &board, Coord coord)
    {
        return piece_at (board, coord.row, coord.col);
    }

    ///////////////////////////////////////////////

    // white moves up (-)
    // black moves down (+)
    static inline int pawn_direction (Color color)
    {
        assert (color == Color::White || color == Color::Black);
        return color == Color::Black ? 1 : -1;
    }

    static inline bool need_pawn_promotion (int row, Color who)
    {
        assert (is_color_valid (who));
        switch (who)
        {
            case Color::White: return 0 == row;
            case Color::Black: return 7 == row;
            default: throw Error { "Invalid color in need_pawn_promotion" };
        }
    }

    constexpr bool able_to_castle (const Board &board, Color who, CastlingState castle_type)
    {
        ColorIndex c_index = color_index (who);

        bool didnt_castle = board.castled[c_index] != Castle_Castled;
        bool neg_not_set = ((~board.castled[c_index]) & castle_type) != 0;

        return didnt_castle && neg_not_set;
    }

    constexpr CastlingState board_get_castle_state (const Board &board, Color who)
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
        return board.en_passant_target[color_index (who)] != No_En_Passant_Coord;
    }

    static inline bool board_equals (const Board &a, const Board &b)
    {
        for (auto coord : All_Coords_Iterator)
        {
            if (a.squares[coord.row][coord.col] != b.squares[coord.row][coord.col])
                return false;
        }

        // todo check more
        return true;
    }
}

#endif // WISDOM_CHESS_BOARD_H_