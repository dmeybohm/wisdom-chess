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

    bool is_king_threatened_inline (const Board& board, Color who,
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
        int8_t my_king_row;
        int8_t my_king_col;
        int my_lane_threats = 0;
        int my_diagonal_threats = 0;

        Color my_king_color;
        Color my_opponent_color;

        Threats (const Board& board, Color king_color, int8_t king_row, int8_t king_col)
                : my_board { board }, my_king_color { king_color },
                my_king_row { king_row }, my_king_col { king_col }
        {
            my_opponent_color = color_invert (king_color);
        }

        constexpr bool check_lane_threats (int target_row, int target_col)
        {
            ColoredPiece piece = my_board.piece_at (target_row, target_col);
            auto type = piece_type (piece);
            auto target_color = piece_color (piece);

            // 1 or 0: whether to consider a piece or revert to the king
            // position.
            int is_rook = type == Piece::Rook;
            int is_queen = type == Piece::Queen;
            int is_opponent_color = target_color == my_opponent_color;

            int has_threatening_piece = (is_rook | is_queen) & is_opponent_color;

            // If the check is blocked, revert to the the king position itself
            // for the calculation to avoid any branching.
            my_lane_threats |= has_threatening_piece;

            return type != Piece::None;
        }

        constexpr bool check_diagonal_threats (int target_row, int target_col)
        {
            ColoredPiece piece = my_board.piece_at (target_row, target_col);
            auto type = piece_type (piece);
            auto target_color = piece_color (piece);

            // 1 or 0: whether to consider a piece or revert to the king
            // position.
            int8_t is_bishop = type == Piece::Bishop;
            int8_t is_queen = type == Piece::Queen;
            int8_t is_opponent_color = target_color == my_opponent_color;

            int8_t has_threatening_piece = (is_bishop | is_queen) & is_opponent_color;

            // If the check is blocked, revert to the the king position itself
            // for the calculation to avoid any branching.
            my_diagonal_threats |= has_threatening_piece;

            return type != Piece::None;
        }

        bool any_row_threats ()
        {
            for (int8_t new_col = my_king_col; new_col <= Last_Column; new_col++)
            {
                auto stop = check_lane_threats (my_king_row, new_col);

                if (my_lane_threats)
                    return true;

                if (stop)
                    break;
            }

            for (int8_t new_col = my_king_col; new_col >= First_Column; new_col--)
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
            for (int8_t new_row = my_king_row + 1; new_row <= Last_Row; new_row++)
            {
                auto stop = check_lane_threats (new_row, my_king_col);

                if (my_lane_threats)
                    return true;

                if (stop)
                    break;
            }

            for (int8_t new_row = my_king_row - 1; new_row >= First_Row; new_row--)
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
            // northwest
            for (int8_t new_col = next_column (my_king_col,  -1),
                         new_row = next_row (my_king_row,  -1);
                 new_row >= First_Row && new_col >= First_Column;
                 new_col--, new_row--
            ) {
                auto stop = check_diagonal_threats (new_row, new_col);

                if (my_diagonal_threats)
                    return true;

                if (stop)
                    break;
            }

            // northeast
            for (int8_t new_col = next_column (my_king_col,  +1),
                        new_row = next_row (my_king_row,  -1);
                 new_row >= First_Row && new_col <= Last_Column;
                 new_col++, new_row--
            ) {
                auto stop = check_diagonal_threats (new_row, new_col);

                if (my_diagonal_threats)
                    return true;

                if (stop)
                    break;
            }

            // southeast
            for (int8_t new_col = next_column (my_king_col,  +1),
                        new_row = next_row (my_king_row,  +1);
                 new_row >= Last_Row && new_col >= First_Column;
                 new_col++, new_row++
            ) {
                auto stop = check_diagonal_threats (new_row, new_col);

                if (my_diagonal_threats)
                    return true;

                if (stop)
                    break;
            }

            // southwest
            for (int8_t new_col = next_column (my_king_col, -1),
                        new_row = next_row (my_king_row,  +1);
                 new_row >= First_Row && new_col >= First_Column;
                 new_col--, new_row++
            ) {
                auto stop = check_diagonal_threats (new_row, new_col);

                if (my_diagonal_threats)
                    return true;

                if (stop)
                    break;
            }

            return false;
        }
    };

    bool any_row_threats (Threats threats)
    {
        return threats.any_row_threats();
    }

    bool any_column_threats (Threats threats)
    {
        return threats.any_column_threats();
    }

    bool any_diagonal_threats (Threats threats)
    {
        return threats.any_diagonal_threats();
    }

    bool is_king_threatened (const Board& board, Color who,
                             int8_t king_row, int8_t king_col)
    {
        int8_t row, col;
        int8_t c_dir, r_dir;
        Threats threats { board, who, king_row, king_col };

        if (threats.any_row_threats ())
            return true;

        if (threats.any_column_threats ())
            return true;

        if (is_king_threatened_diagonal_dumb (board, who, king_row, king_col))
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
        if (is_king_threatened_pawn_dumb (board, who, king_row, king_col))
            return true;

        if (is_king_threatened_king (board, who, king_row, king_col))
            return true;

        return false;
    }

    bool is_king_threatened_row (const Board& board, Color who,
                                 int8_t king_row, int8_t king_col)
    {
        Threats threats { board, who, king_row, king_col };

        if (threats.any_row_threats ())
            return true;

        return false;
    }

    bool is_king_threatened_column (const Board& board, Color who,
                                    int8_t king_row, int8_t king_col)
    {
        Threats threats { board, who, king_row, king_col };

        if (threats.any_column_threats ())
            return true;

        return false;
    }

    bool is_king_threatened_diagonal (const Board& board, Color who,
                                      int8_t king_row, int8_t king_col)
    {
        Threats threats { board, who, king_row, king_col };

        if (threats.any_diagonal_threats ())
            return true;

        return false;
    }

    bool is_king_threatened_knight (const Board& board, Color who,
                                    int8_t king_row, int8_t king_col)
    {
        int8_t row, col;

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

        return false;
    }

    bool is_king_threatened_pawn (const Board& board, Color who,
                                  int8_t king_row, int8_t king_col)
    {
        // check for pawn checks
        auto r_dir = pawn_direction (who);

        int8_t left_col = next_column (king_col,  -1);
        int8_t right_col = next_column (king_col, +1);
        int8_t pawn_row = next_row (king_row,  r_dir);
        if (pawn_row >= Last_Row || pawn_row <= First_Row)
            return false;

        if (left_col > First_Column)
        {
            auto what = board.piece_at (pawn_row, left_col);
            if (piece_type (what) == Piece::Pawn && piece_color (what) != who)
                return true;
        }

        if (right_col < Last_Column)
        {
            auto what = board.piece_at (pawn_row, right_col);
            if (piece_type (what) == Piece::Pawn && piece_color (what) != who)
                return true;

        }

        return false;
    }

    bool is_king_threatened_pawn_dumb (const Board& board, Color who,
                                       int8_t king_row, int8_t king_col)
    {
        int8_t r_dir = pawn_direction (who);

        for (int8_t c_dir = -1; c_dir <= 1; c_dir += 2)
        {
            int8_t row = next_row (king_row, r_dir);
            int8_t col = next_column (king_col, c_dir);

            if (!is_valid_row (row) || !is_valid_column (col))
                continue;

            auto what = board.piece_at (row, col);

            if (piece_type (what) == Piece::Pawn && piece_color (what) != who)
                return true;
        }

        return false;
    }

    bool is_king_threatened_pawn_c (const Board& board, int who,
                                  int8_t king_row, int8_t king_col)
    {
        // check for pawn checks
        int8_t r_dir = gsl::narrow_cast<int8_t>(-1 + 2 * who);

        int8_t row, col;
        int8_t what;
        int8_t c_dir;

        for (c_dir = -1; c_dir <= 1; c_dir += 2)
        {
            row = next_row (king_row, r_dir);
            col = next_column (king_col, c_dir);

            if (row > Last_Row || row < First_Row || col < First_Column || col > Last_Column)
                continue;

            what = board.raw_squares[row][col];
            int8_t piece_type = gsl::narrow_cast<int8_t>(what & (0x8-1));
            int8_t color = gsl::narrow_cast<int8_t>((what & (0x18)) >> 3);

            if (piece_type == 6 && color != who)
                return true;
        }

        return false;
    }

    bool is_king_threatened_king (const Board& board, Color who,
                                  int8_t king_row, int8_t king_col)
    {
        int8_t row, col;

        // check for king checks
        int8_t min_row = std::max (next_row (king_row,  -1), (int8_t)0);
        int8_t max_row = std::min (next_row (king_row,  +1), Last_Row);
        int8_t min_col = std::max (next_column (king_col, -1), (int8_t)0);
        int8_t max_col = std::min (next_column (king_col,  +1), Last_Column);

        for (row = min_row; row <= max_row; row = next_row (row, 1))
        {
            for (col = min_col; col <= max_col; col = next_column (col, 1))
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

    bool is_king_threatened_diagonal_dumb (const Board& board, Color who,
                                           int8_t king_row, int8_t king_col)
    {
        // check each diagonal direction
        for (int8_t r_dir = -1; r_dir <= 1; r_dir += 2)
        {
            for (int8_t c_dir = -1; c_dir <= 1; c_dir += 2)
            {
                for (int8_t row = next_row (king_row, r_dir), col = next_column (king_col, c_dir);
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

        return false;
    }

    bool was_legal_move (Board& board, Color who, Move mv)
    {
        auto king_coord= board.get_king_position (who);

        if (is_king_threatened (board, who, king_coord))
            return false;

        auto king_row = Row (king_coord);
        auto king_col = Column (king_coord);

        if (is_castling_move (mv))
        {
            Coord castled_pos = move_dst (mv);
            auto castled_row = Row (castled_pos);
            auto castled_col = Column (castled_pos);

            assert (king_row == castled_row);
            assert (king_col == castled_col);

            int8_t direction = is_castling_move_on_king_side (mv) ? -1 : 1;

            int8_t plus_one_column = next_column (castled_col, direction);
            int8_t plus_two_column = next_column (plus_one_column, direction);

            if (is_king_threatened (board, who, castled_row, plus_one_column) ||
                is_king_threatened (board, who, castled_row, plus_two_column))
            {
                return false;
            }

        }

        return true;
    }
}