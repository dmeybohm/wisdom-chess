#include "check.hpp"
#include "coord.hpp"
#include "move.hpp"
#include "board.hpp"
#include "generate.hpp"
#include "history.hpp"

namespace wisdom
{
    bool is_checkmated (Board &board, Color who)
    {
        auto coord = king_position (board, who);

        if (!is_king_threatened (board, who, coord))
            return false;

        MoveList legal_moves = generate_legal_moves (board, who);

        return legal_moves.empty ();
    }

    bool is_king_threatened (Board &board, Color who,
                             int king_row, int king_col)
    {
        int row, col;
        ColoredPiece what;
        int c_dir, r_dir;

        // check each side of the king's row
        col = king_col;
        for (r_dir = -1; r_dir <= 1; r_dir += 2)
        {
            for (row = next_row (king_row, r_dir); is_valid_row (row); row = next_row (row, r_dir))
            {
                what = piece_at (board, row, col);

                if (piece_type (what) == Piece::None)
                    continue;

                if (piece_color (what) == who)
                    break;

                if (piece_type (what) == Piece::Rook ||
                    piece_type (what) == Piece::Queen)
                    return true;

                break;
            }
        }

        // check each side of the king's column
        row = king_row;
        for (c_dir = -1; c_dir <= 1; c_dir += 2)
        {
            for (col = next_column (king_col, c_dir); is_valid_column (col); col = next_column (col, c_dir))
            {
                what = piece_at (board, row, col);

                if (piece_type (what) == Piece::None)
                    continue;

                if (piece_color (what) == who)
                    break;

                if (piece_type (what) == Piece::Rook ||
                    piece_type (what) == Piece::Queen)
                    return true;

                break;
            }
        }

        // check each diagonal direction
        for (r_dir = -1; r_dir <= 1; r_dir += 2)
        {
            for (c_dir = -1; c_dir <= 1; c_dir += 2)
            {
                for (row = next_row (king_row, r_dir), col = next_column (king_col, c_dir);
                     is_valid_row (row) && is_valid_column (col);
                     row = next_row (row, r_dir), col = next_column (col, c_dir))
                {
                    what = piece_at (board, row, col);

                    if (piece_type (what) == Piece::None)
                        continue;

                    if (piece_color (what) == who)
                        break;

                    if (piece_type (what) == Piece::Bishop ||
                        piece_type (what) == Piece::Queen)
                    {
                        return true;
                    }

                    break;
                }
            }
        }

        // check for knight checks
        const auto &kt_moves = generate_knight_moves (king_row, king_col);

        for (auto move : kt_moves)
        {
            Coord dst;

            dst = move_dst (move);

            row = ROW (dst);
            col = COLUMN (dst);

            what = piece_at (board, row, col);

            if (piece_type (what) == Piece::None)
                continue;

            if (piece_type (what) == Piece::Knight && piece_color (what) != who)
                return true;
        }

        // check for pawn checks
        r_dir = pawn_direction (who);

        for (c_dir = -1; c_dir <= 1; c_dir += 2)
        {
            row = next_row (king_row, r_dir);
            col = next_column (king_col, c_dir);

            if (!is_valid_row (row) || !is_valid_column (col))
                continue;

            what = piece_at (board, row, col);

            if (piece_type (what) == Piece::Pawn && piece_color (what) != who)
                return true;
        }

        // check for king checks
        for (row = king_row - 1; row <= king_row + 1; row++)
        {
            if (!is_valid_row (row))
                continue;

            for (col = king_col - 1; col <= king_col + 1; col++)
            {
                if (!is_valid_column (col))
                    continue;

                if (col == king_col && row == king_row)
                    continue;

                what = piece_at (board, row, col);

                if (piece_type (what) == Piece::King && piece_color (what) != who)
                    return true;
            }
        }

        return false;
    }

    bool was_legal_move (Board &board, Color who, Move mv)
    {
        auto[king_row, king_col] = king_position (board, who);

        if (is_king_threatened (board, who, king_row, king_col))
            return false;

        if (is_castling_move (mv))
        {
            Coord castled_pos = move_dst (mv);
            auto[castled_row, castled_col] = castled_pos;

            assert (king_row == castled_row);
            assert (king_col == castled_col);

            int direction = is_castling_move_on_king_side (mv) ? -1 : 1;

            int plus_one_column = next_column (castled_col, direction);
            int plus_two_column = next_column (plus_one_column, direction);
            if (is_king_threatened (board, who, castled_row, plus_one_column) ||
                is_king_threatened (board, who, castled_row, plus_two_column))
            {
                return false;
            }

        }

        return true;
    }
}