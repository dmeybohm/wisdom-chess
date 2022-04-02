#ifndef WISDOM_THREATS_HPP
#define WISDOM_THREATS_HPP

#include "global.hpp"

#include "board.hpp"
#include "piece.hpp"

namespace wisdom
{
    struct InlineThreats
    {
        struct ThreatStatus
        {
            bool blocked;
            bool threatened;
        };

        const Board& my_board;
        Color my_opponent;

        Color my_king_color;
        Coord my_king_coord;
        int my_king_row;
        int my_king_col;
        int my_pawn_direction;
        int my_opponent_color_shifted;

        InlineThreats (const Board& board, Color king_color, Coord king_coord)
            : my_board { board },
                my_opponent { color_invert (king_color) },
                my_king_color { king_color },
                my_king_coord { king_coord },
                my_king_row { Row (king_coord) },
                my_king_col { Column (king_coord) },
                my_pawn_direction { pawn_direction (king_color) },
                my_opponent_color_shifted { to_colored_piece_shifted (my_opponent) }
        {
        }

        // Check if the the king indicated by the WHO argument is in trouble
        // in this position.
        bool check_all ()
        {
            return
                row () ||
                column () ||
                diagonal () ||
                knight_direct () ||
                pawn_inline () ||
                king_inline ();
        }

        constexpr auto check_lane_threats (int target_row, int target_col)
            -> ThreatStatus
        {
            ThreatStatus result {};

            ColoredPiece piece = my_board.piece_at (target_row, target_col);
            auto type = piece_type (piece);
            auto target_color = piece_color (piece);

            // 1 or 0: whether to consider a piece or revert to the king
            // position.
            int is_rook = type == Piece::Rook;
            int is_queen = type == Piece::Queen;
            int is_opponent_color = target_color == my_opponent;

            int has_threatening_piece = (is_rook | is_queen) & is_opponent_color;

            // If the check is blocked, revert to the king position itself
            // for the calculation to avoid any branching.
            result.threatened = has_threatening_piece;
            result.blocked = type != Piece::None;

            return result;
        }

        // Check an entire row for any rook / queen threats.
        bool row ()
        {
            for (auto new_col = next_column (my_king_col, +1); new_col <= Last_Column; new_col++)
            {
                auto status = check_lane_threats (my_king_row, new_col);

                if (status.threatened)
                    return true;

                if (status.blocked)
                    break;
            }

            for (auto new_col = next_column (my_king_col, -1); new_col >= First_Column; new_col--)
            {
                auto status = check_lane_threats (my_king_row, new_col);

                if (status.threatened)
                    return true;

                if (status.blocked)
                    break;
            }

            return false;
        }

        // Check an entire column for any rook / queen threats.
        bool column ()
        {
            for (auto new_row = next_row (my_king_row, +1); new_row <= Last_Row; new_row++)
            {
                auto status = check_lane_threats (new_row, my_king_col);

                if (status.threatened)
                    return true;

                if (status.blocked)
                    break;
            }

            for (auto new_row = next_row (my_king_row, -1); new_row >= First_Row; new_row--)
            {
                auto status = check_lane_threats (new_row, my_king_col);

                if (status.threatened)
                    return true;

                if (status.blocked)
                    break;
            }

            return false;
        }

        bool diagonal_dumb ()
        {
            // check each diagonal direction
            for (int8_t r_dir = -1; r_dir <= 1; r_dir += 2)
            {
                for (int8_t c_dir = -1; c_dir <= 1; c_dir += 2)
                {
                    for (int8_t row = next_row (my_king_row, r_dir),
                                col = next_column (my_king_col, c_dir);
                         is_valid_row (row) && is_valid_column (col);
                         row = next_row (row, r_dir), col = next_column (col, c_dir))
                    {
                        auto what = my_board.piece_at (row, col);

                        if (what == Piece_And_Color_None)
                            continue;

                        if (piece_color (what) == my_king_color)
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

        int check_knight_at_square (int target_row, int target_col)
        {
            auto piece = my_board.piece_at (target_row, target_col);
            return piece_color (piece) == my_opponent && piece_type (piece) == Piece::Knight;
        }

        template <int row_dir, int col_dir>
        int check_knight ()
        {
            int starting_row = my_king_row + row_dir;
            int starting_col = my_king_col + col_dir;

            if constexpr (row_dir != 0)
            {
                // starting square is along the row - so go left or right
                // with the column along the same direction as the row.
                auto target_row = starting_row + row_dir;
                if constexpr (row_dir > 0)
                {
                    if (target_row > Last_Row)
                        return false;
                }
                else
                {
                    if (target_row < First_Row)
                        return false;
                }

                auto left_col = my_king_col - 1;
                auto right_col = my_king_col + 1;

                int attacked = 0;
                if (left_col >= First_Column)
                    attacked |= check_knight_at_square (target_row, left_col);
                if (right_col <= Last_Column)
                    attacked |= check_knight_at_square (target_row, right_col);

                return attacked;
            }
            else
            {
                // starting square is along the row - so go left or right
                // with the column along the same direction as the row.
                auto target_col = starting_col + col_dir;
                if constexpr (col_dir > 0)
                {
                    if (target_col > Last_Column)
                        return false;
                }
                else
                {
                    if (target_col < First_Column)
                        return false;
                }

                auto left_row = my_king_row - 1;
                auto right_row = my_king_row + 1;

                int attacked = 0;
                if (left_row >= First_Row)
                    attacked |= check_knight_at_square (left_row, target_col);
                if (right_row <= Last_Row)
                    attacked |= check_knight_at_square (right_row, target_col);

                return attacked;
            }
        }

        int knight_direct ()
        {
            return check_knight<-1, 0> ()
                || check_knight<0, +1> ()
                || check_knight<+1, 0> ()
                || check_knight<0, -1> ();
        }

        bool pawn ()
        {
            // check for pawn checks
            auto r_dir = my_pawn_direction;

            int8_t left_col = next_column (my_king_col, -1);
            int8_t right_col = next_column (my_king_col, +1);
            int8_t pawn_row = next_row (my_king_row, r_dir);
            if (pawn_row >= Last_Row || pawn_row <= First_Row)
                return false;

            if (left_col > First_Column)
            {
                auto what = my_board.piece_at (pawn_row, left_col);
                if (piece_type (what) == Piece::Pawn && piece_color (what) == my_opponent)
                    return true;
            }

            if (right_col < Last_Column)
            {
                auto what = my_board.piece_at (pawn_row, right_col);
                if (piece_type (what) == Piece::Pawn && piece_color (what) == my_opponent)
                    return true;
            }

            return false;
        }

        bool pawn_dumb ()
        {
            int8_t r_dir = my_pawn_direction;

            for (int8_t c_dir = -1; c_dir <= 1; c_dir += 2)
            {
                int8_t row = next_row (my_king_row, r_dir);
                int8_t col = next_column (my_king_col, c_dir);

                if (!is_valid_row (row) || !is_valid_column (col))
                    continue;

                auto what = my_board.piece_at (row, col);

                if (piece_type (what) == Piece::Pawn && piece_color (what) == my_opponent)
                    return true;
            }

            return false;
        }

        bool pawn_inline ()
        {
            int r_dir = pawn_direction<int> (my_king_color);
            int left_col = my_king_col - 1;
            int right_col = my_king_col + 1;
            int target_row = my_king_row + r_dir;

            int left_attack_exists
                = (is_valid_row (target_row) && is_valid_column (left_col)
                   && my_board.piece_at (target_row, left_col) == make_piece (my_opponent, Piece::Pawn));
            int right_attack_exists
                = (is_valid_row (target_row) && is_valid_column (right_col)
                   && my_board.piece_at (target_row, right_col) == make_piece (my_opponent, Piece::Pawn));

            return left_attack_exists | right_attack_exists;
        }

        enum class KingThreatCheck {
            CheckMiddle,
            DoNotCheckMiddle
        };
        template <KingThreatCheck squares_to_check>
        bool check_king_threat_row (int target_row, int starting_col,
                                    int ending_col)
        {
            int middle_col = next_column<int> (starting_col, +1);
            bool middle_attack_exists = false;
            ColoredPiece opponent_king = make_piece (my_opponent, Piece::King);

            bool left_attack_exists = (
                is_valid_row (target_row) && is_valid_column (starting_col)
                && my_board.piece_at (target_row, starting_col) == opponent_king
            );
            if constexpr (squares_to_check == KingThreatCheck::CheckMiddle)
            {
                middle_attack_exists = (
                    is_valid_row (target_row) && is_valid_column (middle_col)
                    && my_board.piece_at (target_row, middle_col) == opponent_king
                );
            }
            bool right_attack_exists = (
                is_valid_row (target_row) && is_valid_column (ending_col)
                && my_board.piece_at (target_row, ending_col) == opponent_king
            );

            return left_attack_exists | middle_attack_exists | right_attack_exists;
        }

        bool king_inline ()
        {
            auto left_col = next_column<int> (my_king_col, -1);
            auto right_col = next_column<int> (my_king_col, +1);

            // Inline checks across all three possible rows.
            bool top_attack_exists = check_king_threat_row<KingThreatCheck::CheckMiddle> (
                next_row<int> (my_king_row, -1),
                left_col,
                right_col
            );

            bool center_attack_exists = check_king_threat_row<KingThreatCheck::DoNotCheckMiddle> (
                my_king_row,
                left_col,
                right_col
            );

            bool bottom_attack_exists = check_king_threat_row<KingThreatCheck::CheckMiddle> (
                next_row (my_king_row, +1),
                left_col,
                right_col
            );

            return top_attack_exists | center_attack_exists | bottom_attack_exists;
        }

        //
        // Unused methods here
        //
        bool king ()
        {
            int row, col;

            // check for king checks
            int min_row = std::max (next_row<int> (my_king_row, -1), 0);
            int max_row = std::min (next_row<int> (my_king_row, +1), Last_Row);
            int min_col = std::max (next_column<int> (my_king_col, -1), 0);
            int max_col = std::min (next_column<int> (my_king_col, +1), Last_Column);

            for (row = min_row; row <= max_row; row = next_row (row, 1))
            {
                for (col = min_col; col <= max_col; col = next_column (col, 1))
                {
                    if (col == my_king_col && row == my_king_row)
                        continue;

                    auto what = my_board.piece_at (row, col);

                    if (piece_type (what) == Piece::King && piece_color (what) == my_opponent)
                        return true;
                }
            }

            return false;

        }

        // todo: make this really constexpr by unrolling the checks
        constexpr auto check_diagonal_threats (int target_row, int target_col)
            -> ThreatStatus
        {
            ThreatStatus result {};

            ColoredPiece piece = my_board.piece_at (target_row, target_col);
            auto type = piece_type (piece);
            auto target_color = piece_color (piece);

            // 1 or 0: whether to consider a piece or revert to the king
            // position.
            int is_bishop = type == Piece::Bishop;
            int is_queen = type == Piece::Queen;
            int is_opponent_color = target_color == my_opponent;

            int has_threatening_piece = (is_bishop | is_queen) & is_opponent_color;

            // If the check is blocked, revert to the king position itself
            // for the calculation to avoid any branching.
            result.threatened = has_threatening_piece;
            result.blocked = type != Piece::None;

            return result;
        }

        // Check a diagonal for any bishop / queen threats.
        bool diagonal ()
        {
            // northwest
            for (int distance = 1; distance < Num_Columns; distance++)
            {
                int new_row = my_king_row - distance;
                int new_col = my_king_col - distance;
                if (new_row < 0 || new_col < 0)
                    break;

                auto status = check_diagonal_threats (new_row, new_col);

                if (status.threatened)
                    return true;

                if (status.blocked)
                    break;
            }

            // northeast
            for (int distance = 1; distance < Num_Columns; distance++)
            {
                int new_row = my_king_row - distance;
                int new_col = my_king_col + distance;
                if (new_row < 0 || new_col > Last_Column)
                    break;

                auto status = check_diagonal_threats (new_row, new_col);

                if (status.threatened)
                    return true;

                if (status.blocked)
                    break;
            }

            // southeast
            for (int distance = 1; distance < Num_Columns; distance++)
            {
                int new_row = my_king_row + distance;
                int new_col = my_king_col + distance;
                if (new_row > Last_Row || new_col > Last_Column)
                    break;

                auto status = check_diagonal_threats (new_row, new_col);

                if (status.threatened)
                    return true;

                if (status.blocked)
                    break;
            }

            // southwest
            for (int distance = 1; distance < Num_Columns; distance++)
            {
                int new_row = my_king_row + distance;
                int new_col = my_king_col - distance;
                if (new_row > Last_Row || new_col < 0)
                    break;

                auto status = check_diagonal_threats (new_row, new_col);

                if (status.threatened)
                    return true;

                if (status.blocked)
                    break;
            }

            return false;
        }

        bool pawn_c (int who)
        {
            // check for pawn checks
            int8_t r_dir = gsl::narrow_cast<int8_t> (-1 + 2 * who);

            int8_t row, col;
            int8_t what;
            int8_t c_dir;

            for (c_dir = -1; c_dir <= 1; c_dir += 2)
            {
                row = next_row (my_king_row, r_dir);
                col = next_column (my_king_col, c_dir);

                if (row > Last_Row || row < First_Row || col < First_Column || col > Last_Column)
                    continue;

                what = my_board.raw_squares[row][col];
                int8_t piece_type = gsl::narrow_cast<int8_t> (what & (0x8 - 1));
                int8_t color = gsl::narrow_cast<int8_t> ((what & (0x18)) >> 3);

                if (piece_type == 6 && color != who)
                    return true;
            }

            return false;
        }

        bool knight ()
        {
            int8_t row, col;

            // check for knight checks
            const auto& kt_moves = generate_knight_moves (my_king_row, my_king_col);

            for (auto move : kt_moves)
            {
                Coord dst = move_dst (move);

                row = Row (dst);
                col = Column (dst);

                auto what = my_board.piece_at (row, col);
                if (what == Piece_And_Color_None)
                    continue;

                if (piece_type (what) == Piece::Knight && piece_color (what) == my_opponent)
                    return true;
            }

            return false;
        }
    };
}

#endif // WISDOM_THREATS_HPP
