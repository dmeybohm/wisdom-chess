#ifndef WISDOM_THREATS_HPP
#define WISDOM_THREATS_HPP

#include "global.hpp"

#include "board.hpp"
#include "piece.hpp"

namespace wisdom
{
    struct Threats
    {
        const Board& my_board;
        Color my_opponent;
        Color my_king_color;
        Coord my_king_coord;
        int8_t my_king_row;
        int8_t my_king_col;
        int8_t my_pawn_direction;

        Threats (const Board& board, Color king_color, Coord king_coord)
            : my_board { board },
                my_opponent { color_invert (king_color) },
                my_king_color { king_color },
                my_king_coord { my_king_coord },
                my_king_row { Row (king_coord) },
                my_king_col { Column (king_coord) },
                my_pawn_direction { pawn_direction (king_color) }

        {
        }

        // Check if the the king indicated by the WHO argument is in trouble
        // in this position.
        bool check_all ()
        {}

        // Old check all.
        bool old_check_all ()
        {}

        // Check an entire row for any rook / queen threats.
        bool row ()
        {}

        // Check an entire column for any rook / queen threats.
        bool column ()
        {}

        // Check a diagonal for any bishop / queen threats.
        bool diagonal ()
        {}

        bool knight ()
        {}

        bool knight_direct ()
        {}

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

        bool pawn_inline ()
        {
            int8_t r_dir = pawn_direction (my_king_color);
            int8_t left_col = next_column (my_king_col, -1);
            int8_t right_col = next_column (my_king_col, +1);
            int8_t target_row = next_row (my_king_row, r_dir);

            bool left_attack_exists
                = (is_valid_row (target_row) && is_valid_column (left_col)
                   && my_board.piece_at (target_row, left_col) == make_piece (my_opponent, Piece::Pawn));
            bool right_attack_exists
                = (is_valid_row (target_row) && is_valid_column (right_col)
                   && my_board.piece_at (target_row, right_col) == make_piece (my_opponent, Piece::Pawn));

            return left_attack_exists || right_attack_exists;

        }

        bool king ()
        {
            int8_t row, col;

            // check for king checks
            int8_t min_row = std::max (next_row (my_king_row, -1), (int8_t)0);
            int8_t max_row = std::min (next_row (my_king_row, +1), Last_Row);
            int8_t min_col = std::max (next_column (my_king_col, -1), (int8_t)0);
            int8_t max_col = std::min (next_column (my_king_col, +1), Last_Column);

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

        enum class KingThreatCheck {
            CheckMiddle,
            DoNotCheckMiddle
        };
        template <KingThreatCheck squares_to_check>
        bool check_king_threat_row (int8_t target_row, int8_t starting_col,
                                    int8_t ending_col)
        {
            int8_t middle_col = next_column (starting_col, +1);
            bool middle_attack_exists = false;

            bool left_attack_exists = (
                is_valid_row (target_row) && is_valid_column (starting_col)
                && my_board.piece_at (target_row, starting_col) == make_piece (my_opponent, Piece::King)
            );
            if constexpr (squares_to_check == KingThreatCheck::CheckMiddle)
            {
                middle_attack_exists = (
                    is_valid_row (target_row) && is_valid_column (middle_col)
                    && my_board.piece_at (target_row, middle_col) == make_piece (my_opponent, Piece::King)
                );
            }
            bool right_attack_exists = (
                is_valid_row (target_row) && is_valid_column (ending_col)
                && my_board.piece_at (target_row, ending_col) == make_piece (my_opponent, Piece::King)
            );

            return left_attack_exists || middle_attack_exists || right_attack_exists;
        }

        bool king_inline ()
        {
            auto left_col = next_column (my_king_col, -1);
            auto right_col = next_column (my_king_col, +1);

            // Inline checks across all three possible rows.
            bool top_attack_exists = check_king_threat_row<KingThreatCheck::CheckMiddle> (
                next_row (my_king_row, -1),
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

            return top_attack_exists || center_attack_exists || bottom_attack_exists;
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
    };
}

#endif // WISDOM_THREATS_HPP
