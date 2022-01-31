#include "check.hpp"
#include "coord.hpp"
#include "move.hpp"
#include "board.hpp"
#include "generate.hpp"
#include "history.hpp"

namespace wisdom
{
    bool is_checkmated (Board& board, Color who)
    {
        auto coord = board.get_king_position (who);

        if (!is_king_threatened (board, who, coord))
            return false;

        MoveList legal_moves = generate_legal_moves (board, who);

        return legal_moves.empty ();
    }


    bool is_king_threatened (const Board& board, Color who,
                             int king_row, int king_col)
    {
        int row, col;
        int c_dir, r_dir;

        // check each side of the king's row
        col = king_col;
        for (r_dir = -1; r_dir <= 1; r_dir += 2)
        {
            for (row = next_row (king_row, r_dir); is_valid_row (row); row = next_row (row, r_dir))
            {
                auto what = board.piece_at (row, col);

                if (what == Piece_And_Color_None)
                    continue;

                auto check_piece_type = piece_type (what);
                auto check_piece_color = piece_color (what);

                if (check_piece_color == who)
                    break;

                if (check_piece_type == Piece::Rook ||
                    check_piece_type == Piece::Queen)
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
                auto what = board.piece_at (row, col);

                if (what == Piece_And_Color_None)
                    continue;

                if (piece_color (what) == who)
                    break;

                auto check_piece_type = piece_type (what);
                if (check_piece_type == Piece::Rook ||
                    check_piece_type == Piece::Queen)
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
                    auto what = board.piece_at (row, col);

                    if (what == Piece_And_Color_None)
                        continue;

                    if (piece_color (what) == who)
                        break;

                    auto check_piece_type = piece_type (what);
                    if (check_piece_type == Piece::Bishop ||
                        check_piece_type == Piece::Queen)
                    {
                        return true;
                    }

                    break;
                }
            }
        }

        // check for knight checks
        const auto& kt_moves = generate_knight_moves (king_row, king_col);

        for (auto move : kt_moves)
        {
            Coord dst = move_dst (move);

            row = Row (dst);
            col = Column (dst);

            auto what = board.piece_at (row, col);
            if (what == Piece_And_Color_None)
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

            auto what = board.piece_at (row, col);

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

                auto what = board.piece_at (row, col);

                if (piece_type (what) == Piece::King && piece_color (what) != who)
                    return true;
            }
        }

        return false;
    }

    struct Threats
    {
        const Board& my_board;
        int my_king_row;
        int my_king_col;
        int my_king_color_index;
        int my_opponent_color_index;

        template <int r_dir, int c_dir>
        constexpr auto bit_from_rdir_and_cdir () -> int
        {
            if constexpr (r_dir == -1 && c_dir == 0)
                return 0;
            if constexpr (r_dir == 1 && c_dir == 0)
                return 1;
            if constexpr (r_dir == 0 && c_dir == -1)
                return 2;
            if constexpr (r_dir == 0 && c_dir == 1)
                return 3;
            throw new Error { "invalid arguments" };
        }

        int my_blocked_lanes = 0;
        int my_blocked_diagonals = 0;
        int my_lane_threats = 0;
        int my_diagonal_threats = 0;

        Threats (const Board& board, Color king_color, int king_row, int king_col)
                : my_board { board }, my_king_color_index { color_index (king_color) },
                my_king_row { king_row }, my_king_col { king_col }
        {
            my_opponent_color_index = color_index (color_invert (king_color));
        }

        constexpr bool check_lane_threats (int target_row, int target_col)
        {
            ColoredPiece piece = my_board.piece_at (target_row, target_col);
            int type = piece_index (piece_type (piece));
            int target_color = color_index (piece_color (piece));

            // 1 or 0: whether to consider a piece or revert to the king
            // position.
            int is_rook = type == piece_index (Piece::Rook);
            int is_queen = type == piece_index (Piece::Queen);
            int is_opponent_color = target_color == my_opponent_color_index;

            int has_threatening_piece = (is_rook | is_queen) & is_opponent_color;

            // If the check is blocked, revert to the the king position itself
            // for the calculation to avoid any branching.
            my_lane_threats |= has_threatening_piece;

            return type != piece_index (Piece::None);
        }

        bool any_row_threats ()
        {
            for (int new_col = my_king_col; new_col <= Last_Column; new_col++)
            {
                auto stop = check_lane_threats (my_king_row, new_col);

                if (my_lane_threats)
                    return true;

                if (stop)
                    break;
            }

            for (int new_col = my_king_col; new_col >= First_Column; new_col--)
            {
                auto stop = check_lane_threats (my_king_row, new_col);

                if (my_lane_threats)
                    return true;

                if (stop)
                    break;
            }

            return false;
        }

        bool any_column_threats ()
        {
            for (int new_row = my_king_row + 1; new_row <= Last_Row; new_row++)
            {
                auto stop = check_lane_threats (new_row, my_king_col);

                if (my_lane_threats)
                    return true;

                if (stop)
                    break;
            }

            for (int new_row = my_king_row - 1; new_row >= First_Row; new_row--)
            {
                auto stop = check_lane_threats (new_row, my_king_col);

                if (my_lane_threats)
                    return true;

                if (stop)
                    break;
            }

            return false;
        }

        bool any_diagonal_threats ()
        {
            int row, col;

            for (int r_dir = -1; r_dir <= 1; r_dir += 2)
            {
                for (int c_dir = -1; c_dir <= 1; c_dir += 2)
                {
                    for (row = next_row (my_king_row, r_dir), col = next_column (my_king_col, c_dir);
                         is_valid_row (row) && is_valid_column (col);
                         row = next_row (row, r_dir), col = next_column (col, c_dir))
                    {
                        auto what = my_board.piece_at (row, col);

                        if (what == Piece_And_Color_None)
                            continue;

                        if (color_index (piece_color (what)) == my_king_color_index)
                            break;

                        int check_piece_type = piece_index (piece_type (what));
                        if (check_piece_type == piece_index (Piece::Bishop) ||
                            check_piece_type == piece_index (Piece::Queen))
                        {
                            return true;
                        }

                        break;
                    }
                }
            }


            return false;
        }
    };

    bool is_king_threatened_inline (const Board& board, Color who,
                                    int king_row, int king_col)
    {
        int row, col;
        int c_dir, r_dir;
        Threats threats { board, who, king_row, king_col };

        if (threats.any_row_threats ())
            return true;

        if (threats.any_column_threats ())
            return true;

        if (threats.any_diagonal_threats ())
            return true;

        // check for knight checks
        const auto& kt_moves = generate_knight_moves (king_row, king_col);

        for (auto move : kt_moves)
        {
            Coord dst = move_dst (move);

            row = Row (dst);
            col = Column (dst);

            auto what = board.piece_at (row, col);
            if (what == Piece_And_Color_None)
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

            auto what = board.piece_at (row, col);

            if (piece_type (what) == Piece::Pawn && piece_color (what) != who)
                return true;
        }

        // check for king checks
        int min_row = std::max (king_row - 1, 0);
        int max_row = std::min (king_row + 1, Last_Row);
        int min_col = std::max (king_col - 1, 0);
        int max_col = std::min (king_col + 1, Last_Column);

        for (row = min_row; row <= max_row; row++)
        {
            for (col = min_col; col <= max_col; col++)
            {
                if (col == king_col && row == king_row)
                    continue;

                auto what = board.piece_at (row, col);

                if (piece_type (what) == Piece::King && piece_color (what) != who)
                    return true;
            }
        }

        return false;
    }

    bool was_legal_move (Board& board, Color who, Move mv)
    {
        auto[king_row, king_col] = board.get_king_position (who);

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