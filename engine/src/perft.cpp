#include "perft.hpp"
#include "board.hpp"
#include "move.hpp"
#include "check.hpp"
#include "piece.hpp"
#include "str.hpp"

namespace wisdom
{
    using wisdom::perft::Stats;
    using wisdom::perft::to_move_list;

    void Stats::search_moves (Board &board, Color side, int depth, int max_depth)
    {
        if (depth >= max_depth)
            return;

        const auto moves = generate_moves (board, side);

        for (auto move : moves)
        {
            UndoMove undo_state = board.make_move (side, move);

            if (!was_legal_move (board, side, move))
            {
                board.take_back (side, move, undo_state);
                continue;
            }

            counters.nodes++;
            if (is_any_capturing_move (move))
                counters.captures++;

            if (is_en_passant_move (move))
                counters.en_passants++;

            search_moves (board, color_invert (side), depth + 1, max_depth);

            board.take_back (side, move, undo_state);
        }
    }

    auto wisdom::perft::convert_move (const wisdom::Board &board, Color who, std::string move_str)
        -> wisdom::Move
    {
        if (move_str.size () != 4 && move_str.size () != 5)
            throw wisdom::Error { "Invalid size of move" };

        // parse the move into the coordinates
        auto src = wisdom::coord_parse (move_str.substr (0, 2));
        auto dst = wisdom::coord_parse (move_str.substr (2, 2));

        auto promoted = Piece_And_Color_None;
        if (move_str.size () == 5)
        {
            auto promoted_type = piece_from_char (move_str[4]);
            promoted = make_piece (who, promoted_type);
        }

        auto src_piece = board.piece_at (src);
        auto dst_piece = board.piece_at (dst);
        assert (piece_color (src_piece) == who);

        wisdom::Move result = wisdom::make_noncapture_move (src, dst);

        // 1. castling is represented by two space king moves
        if (wisdom::piece_type (src_piece) == Piece::King)
        {
            int castling_row = piece_color (src_piece) == Color::White ?
                   wisdom::Last_Row : wisdom::First_Row;
            if (castling_row == Row (src) && castling_row == Row (dst)
                && std::abs (Column (src) - Column (dst)))
            {
                result = make_castling_move (src, dst);
            }
        }

        // 2. en-passant is represented without (ep) suffix
        if (wisdom::piece_type (src_piece) == Piece::Pawn)
        {
            if (Row (src) != Row (dst) && Column (src) != Column (dst) &&
                dst_piece == Piece_And_Color_None)
            {
                result = make_en_passant_move (src, dst);
            }
        }

        // 3. captures denoted without x
        if (dst_piece != Piece_And_Color_None)
            result = wisdom::copy_move_with_capture (result);

        // 4. Promotions are not in parenthesis
        if (promoted != Piece_And_Color_None)
            result = wisdom::copy_move_with_promotion (result, promoted);

        return result;
    }

    auto wisdom::perft::to_move_list (const wisdom::Board &board, Color who,
                                      const std::string &move_list) -> wisdom::MoveList
    {
        MoveList result;

        // Make a copy of the board for modifications:
        auto board_copy = board;

        auto moves_str_list = wisdom::split (move_list, "\n");

        for (const auto &move_str : moves_str_list)
        {
            auto move = convert_move (board_copy, who, move_str);
            board_copy.make_move (who, move);
            who = color_invert (who);
            result.push_back (move);
        }

        return result;
    }
}