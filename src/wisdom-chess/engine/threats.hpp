#pragma once

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/piece.hpp"

#include <bit>
#include <cstring>

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

        // Build an 8-bit occupancy mask from 8 contiguous ColoredPiece bytes.
        // Bit N is set when column N is occupied (non-zero byte).
        // Uses the standard "has zero byte" SWAR technique to detect non-zero bytes
        // without cross-byte contamination from bit shifts.
        static auto buildOccupancyMask (const ColoredPiece* rank_ptr) -> uint8_t
        {
            uint64_t rank_data;
            std::memcpy (&rank_data, rank_ptr, sizeof (rank_data));

            // Detect zero bytes using Mycroft's "has zero byte" trick:
            //   ((v - 0x0101..01) & ~v & 0x8080..80) has high bit set for each ZERO byte.
            // Invert to get high bit set for each NON-ZERO byte (occupied squares).
            constexpr uint64_t low_ones = UINT64_C (0x0101010101010101);
            constexpr uint64_t high_bits = UINT64_C (0x8080808080808080);

            uint64_t zero_detect = (rank_data - low_ones) & ~rank_data & high_bits;
            uint64_t nonzero_mask = zero_detect ^ high_bits;

            // Pack the high bit of each byte into a single byte via multiply-shift.
            // The multiply by 0x0002040810204081 accumulates bit 7 of each byte
            // into the top byte, and >> 56 extracts it.
            auto occupied = static_cast<uint8_t> (
                (nonzero_mask * UINT64_C (0x0002040810204081)) >> 56
            );

            return occupied;
        }

        // Check if the nearest piece in a direction is an opponent rook or queen.
        auto checkRankPieceAt (int col) -> bool
        {
            int index = my_king_row * Num_Columns + col;
            ColoredPiece piece = my_board.pieceAtIndex (index);
            auto type = pieceType (piece);
            auto color = pieceColor (piece);
            return color == my_opponent && (type == Piece::Rook || type == Piece::Queen);
        }

        // Check an entire row for any rook / queen threats using occupancy bitmask.
        bool row()
        {
            const ColoredPiece* rank_ptr = my_board.squareData() + my_king_row * Num_Columns;
            uint8_t occupied = buildOccupancyMask (rank_ptr);

            // Clear the king's own bit
            occupied &= ~(1u << my_king_col);

            // Right scan: mask off columns <= king_col, find nearest piece rightward
            uint8_t right_mask = occupied & static_cast<uint8_t> (~((1u << (my_king_col + 1)) - 1));
            if (right_mask != 0)
            {
                int nearest_right = std::countr_zero (static_cast<unsigned> (right_mask));
                if (checkRankPieceAt (nearest_right))
                    return true;
            }

            // Left scan: mask off columns >= king_col, find nearest piece leftward
            uint8_t left_mask = occupied & static_cast<uint8_t> ((1u << my_king_col) - 1);
            if (left_mask != 0)
            {
                int nearest_left = std::bit_width (static_cast<unsigned> (left_mask)) - 1;
                if (checkRankPieceAt (nearest_left))
                    return true;
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
            int found = 0;

            for (auto [dr, dc] : offsets)
            {
                auto r = static_cast<unsigned> (my_king_row + dr);
                auto c = static_cast<unsigned> (my_king_col + dc);
                int valid = (r < 8u) & (c < 8u);
                int index = valid ? static_cast<int> (r * 8 + c) : 0;
                int match = (my_board.pieceAtIndex (index) == opponent_knight) & valid;
                found |= match;
            }

            return found != 0;
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
