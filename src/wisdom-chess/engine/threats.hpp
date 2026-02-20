#pragma once

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/piece.hpp"

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

        InlineThreats (const Board& board, Color king_color, Coord king_coord)
            : my_board { board }
            , my_opponent { colorInvert (king_color) }
            , my_king_color { king_color }
            , my_king_row { king_coord.row() }
            , my_king_col { king_coord.column() }
        {
        }

        // Check if the king indicated by the WHO argument is in trouble
        // in this position.
        bool checkAll()
        {
            // clang-format off
            return
                pawn() ||
                knight() ||
                row() ||
                column() ||
                diagonal() ||
                king();
            // clang-format on
        }

        template <Piece sliding_piece>
        constexpr auto checkSlidingThreats (int target_row, int target_col)
            -> ThreatStatus
        {
            ColoredPiece piece = my_board.pieceAt (target_row, target_col);
            auto type = pieceType (piece);
            auto target_color = pieceColor (piece);

            int is_sliding = type == sliding_piece;
            int is_queen = type == Piece::Queen;
            int is_opponent_color = target_color == my_opponent;

            int has_threatening_piece = (is_sliding | is_queen) & is_opponent_color;

            ThreatStatus result = has_threatening_piece ? ThreatStatus::Threatened :
                type != Piece::None ? ThreatStatus::Blocked :
                                    ThreatStatus::None;

            return result;
        }

        // Check an entire row for any rook / queen threats.
        bool row()
        {
            for (auto new_col = nextColumn (my_king_col, +1); new_col <= Last_Column; new_col++)
            {
                auto status = checkSlidingThreats<Piece::Rook> (my_king_row, new_col);
                if (status == ThreatStatus::Threatened)
                    return true;
                else if (status == ThreatStatus::Blocked)
                    break;
            }

            for (auto new_col = nextColumn (my_king_col, -1); new_col >= First_Column; new_col--)
            {
                auto status = checkSlidingThreats<Piece::Rook> (my_king_row, new_col);
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
                auto status = checkSlidingThreats<Piece::Rook> (new_row, my_king_col);
                if (status == ThreatStatus::Threatened)
                    return true;
                else if (status == ThreatStatus::Blocked)
                    break;
            }

            for (auto new_row = nextRow (my_king_row, -1); new_row >= First_Row; new_row--)
            {
                auto status = checkSlidingThreats<Piece::Rook> (new_row, my_king_col);
                if (status == ThreatStatus::Threatened)
                    return true;
                else if (status == ThreatStatus::Blocked)
                    break;
            }

            return false;
        }

        bool knight()
        {
            static constexpr struct
            {
                int row;
                int col;
            } offsets[] = {
                { -2, -1 }, { -2, +1 }, { -1, -2 }, { -1, +2 },
                { +1, -2 }, { +1, +2 }, { +2, -1 }, { +2, +1 },
            };

            ColoredPiece opponent_knight = ColoredPiece::make (my_opponent, Piece::Knight);

            for (auto [dr, dc] : offsets)
            {
                int target_row = my_king_row + dr;
                int target_col = my_king_col + dc;
                if (isValidRow (target_row) && isValidColumn (target_col)
                    && my_board.pieceAt (target_row, target_col) == opponent_knight)
                {
                    return true;
                }
            }

            return false;
        }

        bool pawn()
        {
            int r_dir = pawnDirection<int> (my_king_color);
            int left_col = my_king_col - 1;
            int right_col = my_king_col + 1;
            int target_row = my_king_row + r_dir;

            int left_attack_exists
                = (isValidRow (target_row) && isValidColumn (left_col)
                   && my_board.pieceAt (target_row, left_col)
                       == ColoredPiece::make (my_opponent, Piece::Pawn));
            int right_attack_exists
                = (isValidRow (target_row) && isValidColumn (right_col)
                   && my_board.pieceAt (target_row, right_col)
                       == ColoredPiece::make (my_opponent, Piece::Pawn));

            return left_attack_exists | right_attack_exists;
        }

        enum class KingThreatCheck
        {
            CheckMiddle,
            DoNotCheckMiddle
        };
        template <KingThreatCheck squares_to_check> auto
        checkKingThreatRow (int target_row, int starting_col, int ending_col)
            -> bool
        {
            int middle_col = nextColumn<int> (starting_col, +1);
            bool middle_attack_exists = false;
            ColoredPiece opponent_king = ColoredPiece::make (my_opponent, Piece::King);

            bool left_attack_exists
                = (isValidRow (target_row) && isValidColumn (starting_col)
                   && my_board.pieceAt (target_row, starting_col) == opponent_king);
            if constexpr (squares_to_check == KingThreatCheck::CheckMiddle)
            {
                middle_attack_exists
                    = (isValidRow (target_row) && isValidColumn (middle_col)
                       && my_board.pieceAt (target_row, middle_col) == opponent_king);
            }
            bool right_attack_exists
                = (isValidRow (target_row) && isValidColumn (ending_col)
                   && my_board.pieceAt (target_row, ending_col) == opponent_king);

            return left_attack_exists | middle_attack_exists | right_attack_exists;
        }

        bool king()
        {
            auto left_col = nextColumn<int> (my_king_col, -1);
            auto right_col = nextColumn<int> (my_king_col, +1);

            // Inline checks across all three possible rows.
            bool top_attack_exists = checkKingThreatRow<KingThreatCheck::CheckMiddle> (
                nextRow<int> (my_king_row, -1),
                left_col,
                right_col
            );

            bool center_attack_exists = checkKingThreatRow<KingThreatCheck::DoNotCheckMiddle> (
                my_king_row,
                left_col,
                right_col
            );

            bool bottom_attack_exists = checkKingThreatRow<KingThreatCheck::CheckMiddle> (
                nextRow (my_king_row, +1),
                left_col,
                right_col
            );

            return top_attack_exists | center_attack_exists | bottom_attack_exists;
        }

        template <int horiz_direction, int vert_direction> auto
        checkDiagonalThreat()
            -> bool
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

                auto status = checkSlidingThreats<Piece::Bishop> (new_row, new_col);
                if (status == ThreatStatus::Threatened)
                    return true;
                else if (status == ThreatStatus::Blocked)
                    break;
            }

            return false;
        }

        // Check a diagonal for any bishop / queen threats.
        bool diagonal()
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
