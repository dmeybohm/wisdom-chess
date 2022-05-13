#include "check.hpp"
#include "board.hpp"
#include "coord.hpp"
#include "generate.hpp"
#include "history.hpp"
#include "move.hpp"
#include "threats.hpp"

namespace wisdom
{
    bool is_checkmated (Board& board, Color who, MoveGenerator& generator)
    {
        auto coord = board.get_king_position (who);

        if (!is_king_threatened (board, who, coord))
            return false;

        MoveList legal_moves = generator.generate_legal_moves (board, who);

        return legal_moves.empty ();
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
        int8_t my_opponent_color_shifted;

        Threats (const Board& board, Color king_color, int8_t king_row, int8_t king_col) :
                my_board { board },
                my_king_color { king_color },
                my_king_row { king_row },
                my_king_col { king_col }
        {
            my_opponent_color = color_invert (king_color);
            my_opponent_color_shifted = to_colored_piece_shifted (my_opponent_color);
        }

        constexpr bool check_lane_threats (int8_t target_row, int8_t target_col)
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

        constexpr bool check_diagonal_threats (int8_t target_row, int8_t target_col)
        {
            ColoredPiece piece = my_board.piece_at (target_row, target_col);
            auto piece_as_int = to_int8 (piece);
            auto bishop_int = to_int8 (Piece::Bishop);
            auto queen_int = to_int8 (Piece::Queen);

            // 1 or 0: whether to consider a piece or revert to the king
            // position.
            auto is_bishop = gsl::narrow_cast<int8_t> ((piece_as_int & bishop_int) == bishop_int);
            auto is_queen = gsl::narrow_cast<int8_t> ((piece_as_int & queen_int) == queen_int);
            int8_t is_opponent_color = gsl::narrow_cast<int8_t> ((piece_as_int & Piece_Color_Mask)
                == my_opponent_color_shifted);

            auto has_threatening_piece
                = gsl::narrow_cast<int8_t> ((is_bishop | is_queen) & is_opponent_color);

            // If the check is blocked, revert to the the king position itself
            // for the calculation to avoid any branching.
            my_diagonal_threats |= has_threatening_piece;

            return piece_as_int != to_int8 (Piece_And_Color_None);
        }

        bool any_row_threats ()
        {
            for (auto new_col = next_column (my_king_col, +1); new_col <= Last_Column; new_col++)
            {
                auto stop = check_lane_threats (my_king_row, new_col);

                if (my_lane_threats)
                    return true;

                if (stop)
                    break;
            }

            for (auto new_col = next_column (my_king_col, -1); new_col >= First_Column; new_col--)
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
            for (auto new_row = next_row (my_king_row, +1); new_row <= Last_Row; new_row++)
            {
                auto stop = check_lane_threats (new_row, my_king_col);

                if (my_lane_threats)
                    return true;

                if (stop)
                    break;
            }

            for (auto new_row = next_row (my_king_row, -1); new_row >= First_Row; new_row--)
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
            for (auto new_col = next_column (my_king_col, -1), new_row = next_row (my_king_row, -1);
                 new_row >= First_Row && new_col >= First_Column; new_col--, new_row--)
            {
                auto stop = check_diagonal_threats (new_row, new_col);

                if (my_diagonal_threats)
                    return true;

                if (stop)
                    break;
            }

            // northeast
            for (auto new_col = next_column (my_king_col, +1), new_row = next_row (my_king_row, -1);
                 new_row >= First_Row && new_col <= Last_Column; new_col++, new_row--)
            {
                auto stop = check_diagonal_threats (new_row, new_col);

                if (my_diagonal_threats)
                    return true;

                if (stop)
                    break;
            }

            // southeast
            for (auto new_col = next_column (my_king_col, +1), new_row = next_row (my_king_row, +1);
                 new_row >= Last_Row && new_col >= First_Column; new_col++, new_row++)
            {
                auto stop = check_diagonal_threats (new_row, new_col);

                if (my_diagonal_threats)
                    return true;

                if (stop)
                    break;
            }

            // southwest
            for (auto new_col = next_column (my_king_col, -1), new_row = next_row (my_king_row, +1);
                 new_row >= First_Row && new_col >= First_Column; new_col--, new_row++)
            {
                auto stop = check_diagonal_threats (new_row, new_col);

                if (my_diagonal_threats)
                    return true;

                if (stop)
                    break;
            }

            return false;
        }
    };

    bool any_row_threats (Threats threats) { return threats.any_row_threats (); }

    bool any_column_threats (Threats threats) { return threats.any_column_threats (); }

    bool any_diagonal_threats (Threats threats) { return threats.any_diagonal_threats (); }

    bool is_king_threatened_row (const Board& board, Color who, int8_t king_row, int8_t king_col)
    {
        Threats threats { board, who, king_row, king_col };

        if (threats.any_row_threats ())
            return true;

        return false;
    }

    bool is_king_threatened_column (const Board& board, Color who, int8_t king_row, int8_t king_col)
    {
        Threats threats { board, who, king_row, king_col };

        if (threats.any_column_threats ())
            return true;

        return false;
    }

    bool is_king_threatened_diagonal (const Board& board, Color who, int8_t king_row,
                                      int8_t king_col)
    {
        Threats threats { board, who, king_row, king_col };

        if (threats.any_diagonal_threats ())
            return true;

        return false;
    }

    static bool check_knight_at_square (const Board& board, Color opponent, int8_t target_row,
                                        int8_t target_col)
    {
        auto piece = board.piece_at (target_row, target_col);
        return piece_color (piece) == opponent && piece_type (piece) == Piece::Knight;
    }

    template <int8_t row_dir, int8_t col_dir>
    static bool check_knight (const Board& board, Color opponent, int8_t king_row, int8_t king_col)
    {
        auto starting_row = next_row (king_row, row_dir);
        auto starting_col = next_column (king_col, col_dir);

        if constexpr (row_dir != 0)
        {
            // starting square is along the row - so go left or right
            // with the column along the same direction as the row.
            auto target_row = next_row (starting_row, row_dir);
            if (!is_valid_row (target_row))
                return false;

            auto left_col = next_column (king_col, -1);
            auto right_col = next_column (king_col, +1);

            bool attacked = false;
            if (is_valid_column (left_col))
                attacked |= check_knight_at_square (board, opponent, target_row, left_col);
            if (is_valid_column (right_col))
                attacked |= check_knight_at_square (board, opponent, target_row, right_col);
            return attacked;
        }
        else
        {
            // starting square is along the row - so go left or right
            // with the column along the same direction as the row.
            auto target_col = next_column (starting_col, col_dir);
            if (!is_valid_column (target_col))
                return false;

            auto left_row = next_row (king_row, -1);
            auto right_row = next_row (king_row, +1);

            bool attacked = false;
            if (is_valid_row (left_row))
                attacked |= check_knight_at_square (board, opponent, left_row, target_col);
            if (is_valid_row (right_row))
                attacked |= check_knight_at_square (board, opponent, right_row, target_col);
            return attacked;
        }
    }

    bool is_king_threatened_knight_direct (const Board& board, Color who, int8_t king_row,
                                           int8_t king_col)
    {

        Color opponent = color_invert (who);
        return check_knight<-1, 0> (board, opponent, king_row, king_col)
            || check_knight<0, +1> (board, opponent, king_row, king_col)
            || check_knight<+1, 0> (board, opponent, king_row, king_col)
            || check_knight<0, -1> (board, opponent, king_row, king_col);
    }

    bool is_king_threatened_pawn (const Board& board, Color who, int8_t king_row, int8_t king_col)
    {
        // check for pawn checks
        auto r_dir = pawn_direction (who);

        int8_t left_col = next_column (king_col, -1);
        int8_t right_col = next_column (king_col, +1);
        int8_t pawn_row = next_row (king_row, r_dir);
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

    bool is_king_threatened_pawn_dumb (const Board& board, Color who, int8_t king_row,
                                       int8_t king_col)
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

    bool is_king_threatened_pawn_inline (const Board& board, Color who, int8_t king_row,
                                         int8_t king_col)
    {
        int8_t r_dir = pawn_direction (who);
        int8_t left_col = next_column (king_col, -1);
        int8_t right_col = next_column (king_col, +1);
        int8_t target_row = next_row (king_row, r_dir);
        Color opponent = color_invert (who);

        bool left_attack_exists
            = (is_valid_row (target_row) && is_valid_column (left_col)
               && board.piece_at (target_row, left_col) == make_piece (opponent, Piece::Pawn));
        bool right_attack_exists
            = (is_valid_row (target_row) && is_valid_column (right_col)
               && board.piece_at (target_row, right_col) == make_piece (opponent, Piece::Pawn));

        return left_attack_exists || right_attack_exists;
    }

    bool is_king_threatened_pawn_c (const Board& board, int who, int8_t king_row, int8_t king_col)
    {
        // check for pawn checks
        int8_t r_dir = gsl::narrow_cast<int8_t> (-1 + 2 * who);

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
            int8_t piece_type = gsl::narrow_cast<int8_t> (what & (0x8 - 1));
            int8_t color = gsl::narrow_cast<int8_t> ((what & (0x18)) >> 3);

            if (piece_type == 6 && color != who)
                return true;
        }

        return false;
    }

    bool is_king_threatened_king (const Board& board, Color who, int8_t king_row, int8_t king_col)
    {
        int row, col;

        // check for king checks
        int min_row = std::max (next_row<int> (king_row, -1), 0);
        int max_row = std::min (next_row<int> (king_row, +1), Last_Row);
        int min_col = std::max (next_column<int> (king_col, -1), 0);
        int max_col = std::min (next_column<int> (king_col, +1), Last_Column);

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

    enum class KingThreatCheck {
        CheckMiddle,
        DoNotCheckMiddle
    };
    template <KingThreatCheck squares_to_check>
    static bool check_king_threat_row (const Board& board, Color opponent,
                                       int target_row, int starting_col,
                                       int ending_col)
    {
        int middle_col = next_column<int> (starting_col, +1);
        bool middle_attack_exists = false;

        bool left_attack_exists = (
            is_valid_row (target_row) && is_valid_column (starting_col)
               && board.piece_at (target_row, starting_col) == make_piece (opponent, Piece::King)
        );
        if constexpr (squares_to_check == KingThreatCheck::CheckMiddle)
        {
            middle_attack_exists = (
                is_valid_row (target_row) && is_valid_column (middle_col)
                    && board.piece_at (target_row, middle_col) == make_piece (opponent, Piece::King)
            );
        }
        bool right_attack_exists = (
            is_valid_row (target_row) && is_valid_column (ending_col)
                && board.piece_at (target_row, ending_col) == make_piece (opponent, Piece::King)
        );

        return left_attack_exists || middle_attack_exists || right_attack_exists;
    }

    bool is_king_threatened_king_inline (const Board& board, Color who,
                                         int king_row, int king_col)
    {
        Color opponent = color_invert (who);
        auto left_col = next_column<int> (king_col, -1);
        auto right_col = next_column<int> (king_col, +1);

        // Inline checks across all three possible rows.
        bool top_attack_exists = check_king_threat_row<KingThreatCheck::CheckMiddle> (
                board,
                opponent,
                next_row<int> (king_row, -1),
                left_col,
                right_col
         );

        bool center_attack_exists = check_king_threat_row<KingThreatCheck::DoNotCheckMiddle> (
                board,
                opponent,
                king_row,
                left_col,
                right_col
        );

        bool bottom_attack_exists = check_king_threat_row<KingThreatCheck::CheckMiddle> (
                board,
                opponent,
                next_row (king_row, +1),
                left_col,
                right_col
        );

        return top_attack_exists || center_attack_exists || bottom_attack_exists;
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
        auto king_coord = board.get_king_position (who);

        if (is_king_threatened (board, who, king_coord))
            return false;

        auto king_row = Row (king_coord);
        auto king_col = Column (king_coord);

        if (is_special_castling_move (mv))
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

    bool is_stalemated_slow (Board& board, Color who, MoveGenerator& generator)
    {
        auto legal_moves = generator.generate_legal_moves (board, who);

        return legal_moves.empty () &&
                !is_checkmated (board, who, generator);
    }
}
