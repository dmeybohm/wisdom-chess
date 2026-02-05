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

        // Build an 8-bit occupancy mask from 8 packed bytes in a uint64_t.
        // Bit N is set when byte N is non-zero (occupied square).
        // Uses Mycroft's "has zero byte" SWAR technique to detect non-zero bytes
        // without cross-byte contamination from bit shifts.
        static auto buildOccupancyMask (uint64_t packed_data) -> uint8_t
        {
            constexpr uint64_t low_ones = UINT64_C (0x0101010101010101);
            constexpr uint64_t high_bits = UINT64_C (0x8080808080808080);

            // ((v - 0x0101..01) & ~v & 0x8080..80) has high bit set for each ZERO byte.
            // XOR with 0x8080..80 inverts to get high bit set for NON-ZERO bytes.
            uint64_t zero_detect = (packed_data - low_ones) & ~packed_data & high_bits;
            uint64_t nonzero_mask = zero_detect ^ high_bits;

            // Pack the high bit of each byte into a single byte via multiply-shift.
            auto occupied = static_cast<uint8_t> (
                (nonzero_mask * UINT64_C (0x0002040810204081)) >> 56
            );

            return occupied;
        }

        // Load 8 contiguous rank bytes into a uint64_t.
        static auto loadRank (const ColoredPiece* rank_ptr) -> uint64_t
        {
            uint64_t result;
            std::memcpy (&result, rank_ptr, sizeof (result));
            return result;
        }

        // Gather 8 column bytes (stride-8) into a packed uint64_t.
        // Byte N of the result holds the piece at row N in the given column.
        static auto gatherColumn (const ColoredPiece* data, int col) -> uint64_t
        {
            uint64_t result = 0;
            for (int r = 0; r < Num_Rows; r++)
            {
                auto byte = static_cast<uint64_t> (
                    static_cast<uint8_t> (data[r * Num_Columns + col].piece_type_and_color)
                );
                result |= byte << (r * 8);
            }
            return result;
        }

        // Check if the piece at (row, col) is an opponent rook or queen.
        auto isOpponentRookOrQueen (int row, int col) -> bool
        {
            int index = row * Num_Columns + col;
            ColoredPiece piece = my_board.pieceAtIndex (index);
            auto type = pieceType (piece);
            auto color = pieceColor (piece);
            return color == my_opponent && (type == Piece::Rook || type == Piece::Queen);
        }

        // Scan a lane (rank or column) for rook/queen threats using an occupancy
        // bitmask. king_pos is the king's position along the lane (col for ranks,
        // row for columns). check_piece takes the nearest-piece lane index and
        // returns true if it's an opponent rook/queen.
        template <typename CheckFn>
        static auto scanLane (uint8_t occupied, int king_pos, CheckFn check_piece) -> bool
        {
            occupied &= ~(1u << king_pos);

            // Forward scan: find nearest piece above king_pos
            uint8_t fwd_mask = occupied & static_cast<uint8_t> (~((1u << (king_pos + 1)) - 1));
            if (fwd_mask != 0)
            {
                int nearest = std::countr_zero (static_cast<unsigned> (fwd_mask));
                if (check_piece (nearest))
                    return true;
            }

            // Backward scan: find nearest piece below king_pos
            uint8_t bwd_mask = occupied & static_cast<uint8_t> ((1u << king_pos) - 1);
            if (bwd_mask != 0)
            {
                int nearest = std::bit_width (static_cast<unsigned> (bwd_mask)) - 1;
                if (check_piece (nearest))
                    return true;
            }

            return false;
        }

        // Check an entire row for any rook / queen threats using occupancy bitmask.
        bool row()
        {
            const ColoredPiece* rank_ptr = my_board.squareData() + my_king_row * Num_Columns;
            uint8_t occupied = buildOccupancyMask (loadRank (rank_ptr));

            return scanLane (occupied, my_king_col, [&] (int col) {
                return isOpponentRookOrQueen (my_king_row, col);
            });
        }

        // Check an entire column for any rook / queen threats using occupancy bitmask.
        bool column()
        {
            uint8_t occupied = buildOccupancyMask (
                gatherColumn (my_board.squareData(), my_king_col)
            );

            return scanLane (occupied, my_king_row, [&] (int row) {
                return isOpponentRookOrQueen (row, my_king_col);
            });
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
