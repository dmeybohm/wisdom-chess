#ifndef WISDOM_CHESS_THREATS_HPP
#define WISDOM_CHESS_THREATS_HPP

#include "global.hpp"

#include "board.hpp"
#include "piece.hpp"

namespace wisdom
{
    struct InlineThreats
    {
        enum class ThreatStatus
        {
            None = 0,
            Blocked,
            Threatened,
        };

        const Board& my_board;
        Color my_opponent;

        Color my_king_color;
        int my_king_row;
        int my_king_col;
        int my_pawn_direction;

        InlineThreats (const Board& board, Color king_color, Coord king_coord)
            : my_board { board },
                my_opponent { colorInvert (king_color) },
                my_king_color { king_color },
                my_king_row { Row (king_coord) },
                my_king_col { Column (king_coord) },
                my_pawn_direction { pawnDirection (king_color) }
        {
        }

        // Check if the king indicated by the WHO argument is in trouble
        // in this position.
        bool checkAll()
        {
            // clang-format off
            return
                row() ||
                column() ||
                diagonal() ||
                knight() ||
                pawn() ||
                king();
            // clang-format on
        }

        constexpr auto checkLaneThreats (int target_row, int target_col)
            -> ThreatStatus
        {
            ThreatStatus result {};

            ColoredPiece piece = my_board.pieceAt (target_row, target_col);
            auto type = pieceType (piece);
            auto target_color = pieceColor (piece);

            // 1 or 0: whether to consider a piece or revert to the king
            // position.
            int is_rook = type == Piece::Rook;
            int is_queen = type == Piece::Queen;
            int is_opponent_color = target_color == my_opponent;

            int has_threatening_piece = (is_rook | is_queen) & is_opponent_color;

            // If the check is blocked, revert to the king position itself
            // for the calculation to avoid any branching.
            result = has_threatening_piece ? ThreatStatus::Threatened :
                type != Piece::None ? ThreatStatus::Blocked :
                                    ThreatStatus::None;

            return result;
        }

        // Check an entire row for any rook / queen threats.
        bool row()
        {
            for (auto new_col = nextColumn (my_king_col, +1); new_col <= Last_Column; new_col++)
            {
                auto status = checkLaneThreats (my_king_row, new_col);
                if (status == ThreatStatus::Threatened)
                    return true;
                else if (status == ThreatStatus::Blocked)
                    break;
            }

            for (auto new_col = nextColumn (my_king_col, -1); new_col >= First_Column; new_col--)
            {
                auto status = checkLaneThreats (my_king_row, new_col);
                if (status == ThreatStatus::Threatened)
                    return true;
                else if (status == ThreatStatus::Blocked)
                    break;
            }

            return false;
        }

        // Check an entire column for any rook / queen threats.
        bool column()
        {
            for (auto new_row = nextRow (my_king_row, +1); new_row <= Last_Row; new_row++)
            {
                auto status = checkLaneThreats (new_row, my_king_col);
                if (status == ThreatStatus::Threatened)
                    return true;
                else if (status == ThreatStatus::Blocked)
                    break;
            }

            for (auto new_row = nextRow (my_king_row, -1); new_row >= First_Row; new_row--)
            {
                auto status = checkLaneThreats (new_row, my_king_col);
                if (status == ThreatStatus::Threatened)
                    return true;
                else if (status == ThreatStatus::Blocked)
                    break;
            }

            return false;
        }

        int checkKnightAtSquare (int target_row, int target_col)
        {
            auto piece = my_board.pieceAt (target_row, target_col);
            return pieceColor (piece) == my_opponent && pieceType (piece) == Piece::Knight;
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
                    attacked |= checkKnightAtSquare (target_row, left_col);
                if (right_col <= Last_Column)
                    attacked |= checkKnightAtSquare (target_row, right_col);

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
                    attacked |= checkKnightAtSquare (left_row, target_col);
                if (right_row <= Last_Row)
                    attacked |= checkKnightAtSquare (right_row, target_col);

                return attacked;
            }
        }

        int knight()
        {
            return check_knight<-1, 0> ()
                || check_knight<0, +1> ()
                || check_knight<+1, 0> ()
                || check_knight<0, -1> ();
        }

        bool pawn()
        {
            int r_dir = pawnDirection<int> (my_king_color);
            int left_col = my_king_col - 1;
            int right_col = my_king_col + 1;
            int target_row = my_king_row + r_dir;

            int left_attack_exists
                = (isValidRow (target_row) && isValidColumn (left_col)
                   && my_board.pieceAt (target_row, left_col) == ColoredPiece::make (my_opponent, Piece::Pawn));
            int right_attack_exists
                = (isValidRow (target_row) && isValidColumn (right_col)
                   && my_board.pieceAt (target_row, right_col) == ColoredPiece::make (my_opponent, Piece::Pawn));

            return left_attack_exists | right_attack_exists;
        }

        enum class KingThreatCheck {
            CheckMiddle,
            DoNotCheckMiddle
        };
        template <KingThreatCheck squares_to_check>
        bool checkKingThreatRow (int target_row, int starting_col, int ending_col)
        {
            int middle_col = nextColumn<int> (starting_col, +1);
            bool middle_attack_exists = false;
            ColoredPiece opponent_king = ColoredPiece::make (my_opponent, Piece::King);

            bool left_attack_exists = (isValidRow (target_row) && isValidColumn (starting_col)
                && my_board.pieceAt (target_row, starting_col) == opponent_king
            );
            if constexpr (squares_to_check == KingThreatCheck::CheckMiddle)
            {
                middle_attack_exists = (isValidRow (target_row) && isValidColumn (middle_col)
                    && my_board.pieceAt (target_row, middle_col) == opponent_king
                );
            }
            bool right_attack_exists = (isValidRow (target_row) && isValidColumn (ending_col)
                && my_board.pieceAt (target_row, ending_col) == opponent_king
            );

            return left_attack_exists | middle_attack_exists | right_attack_exists;
        }

        bool king()
        {
            auto left_col = nextColumn<int> (my_king_col, -1);
            auto right_col = nextColumn<int> (my_king_col, +1);

            // Inline checks across all three possible rows.
            bool top_attack_exists = checkKingThreatRow<KingThreatCheck::CheckMiddle> (
                nextRow<int> (my_king_row, -1), left_col, right_col);

            bool center_attack_exists = checkKingThreatRow<KingThreatCheck::DoNotCheckMiddle> (
                my_king_row, left_col, right_col);

            bool bottom_attack_exists = checkKingThreatRow<KingThreatCheck::CheckMiddle> (
                nextRow (my_king_row, +1), left_col, right_col);

            return top_attack_exists | center_attack_exists | bottom_attack_exists;
        }

        constexpr auto checkDiagonalThreats (int target_row, int target_col)
            -> ThreatStatus
        {
            ThreatStatus result = ThreatStatus::None;

            ColoredPiece piece = my_board.pieceAt (target_row, target_col);
            auto type = pieceType (piece);
            auto target_color = pieceColor (piece);

            // 1 or 0: whether to consider a piece or revert to the king
            // position.
            int is_bishop = type == Piece::Bishop;
            int is_queen = type == Piece::Queen;
            int is_opponent_color = target_color == my_opponent;

            int has_threatening_piece = (is_bishop | is_queen) & is_opponent_color;

            // If the check is blocked, revert to the king position itself
            // for the calculation to avoid any branching.
            result = has_threatening_piece ? ThreatStatus::Threatened :
                                           type != Piece::None ? ThreatStatus::Blocked :
                                    ThreatStatus::None;

            return result;
        }

        template <int horiz_direction, int vert_direction>
        auto checkDiagonalThreat() -> bool
        {
            int new_row = my_king_row;
            int new_col = my_king_col;

            for (int distance = 1; distance < Num_Columns; distance++)
            {
                new_row += vert_direction;
                new_col += horiz_direction;

                if constexpr (vert_direction < 0)
                {
                    if (new_row < 0)
                        break;
                }
                else
                {
                    if (new_row > Last_Row)
                        break;
                }

                if constexpr (horiz_direction < 0)
                {
                    if (new_col < 0)
                        break;
                }
                else
                {
                    if (new_col > Last_Column)
                        break;
                }

                auto status = checkDiagonalThreats (new_row, new_col);
                if (status == ThreatStatus::Threatened)
                    return true;
                else if (status == ThreatStatus::Blocked)
                    break;
            }

            return false;
        }

        // Check a diagonal for any bishop / queen threats.
        bool diagonal ()
        {
            return
                // northwest:
                checkDiagonalThreat<-1, -1>() ||
                // northeast:
                checkDiagonalThreat<-1, +1>() ||
                // southwest:
                checkDiagonalThreat<+1, -1>() ||
                // southeast:
                checkDiagonalThreat<+1, +1>();
        }
    };
}

#endif // WISDOM_CHESS_THREATS_HPP
