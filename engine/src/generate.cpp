#include "generate.hpp"
#include "board.hpp"
#include "check.hpp"

namespace wisdom
{
    typedef void (*MoveFunc) (const Board &board, Color who,
                              int piece_row, int piece_col, MoveList &moves);

    static void moves_none (const Board &board, Color who,
                            int piece_row, int piece_col, MoveList &moves);

    static void moves_king (const Board &board, Color who,
                            int piece_row, int piece_col, MoveList &moves);

    static void moves_queen (const Board &board, Color who,
                             int piece_row, int piece_col, MoveList &moves);

    static void moves_rook (const Board &board, [[maybe_unused]] Color who,
                            int piece_row, int piece_col, MoveList &moves);

    static void moves_bishop (const Board &board, [[maybe_unused]] Color who,
                              int piece_row, int piece_col, MoveList &moves);

    static void moves_knight (const Board &board, Color who,
                              int piece_row, int piece_col, MoveList &moves);

    static void moves_pawn (const Board &board, Color who,
                            int piece_row, int piece_col, MoveList &moves);

    static void add_en_passant_move (const Board &board, Color who, int piece_row, int piece_col,
                                     MoveList &moves, int en_passant_column);

    static MoveFunc move_functions[] = {
            moves_none,    // Piece::None   [0]
            moves_king,    // Piece::King   [1]
            moves_queen,   // Piece::Queen  [2]
            moves_rook,    // Piece::Rook   [3]
            moves_bishop,  // Piece::Bishop [4]
            moves_knight,  // Piece::Knight [5]
            moves_pawn,    // Piece::Pawn   [6]
            nullptr,
    };

    static void knight_move_list_init (MoveList knight_moves[Num_Rows][Num_Columns]);

    static MoveList &get_knight_moves (int row, int col)
    {
        static MoveList knight_moves[Num_Rows][Num_Columns];

        if (knight_moves[0][0].empty())
            knight_move_list_init (knight_moves);

        return knight_moves[row][col];
    }

    // generate a lookup table for knight moves
    static void knight_move_list_init (MoveList knight_moves[Num_Rows][Num_Columns])
    {
        int k_row, k_col;

        for (auto[row, col] : All_Coords_Iterator)
        {
            for (k_row = -2; k_row <= 2; k_row++)
            {
                if (!k_row)
                    continue;

                if (!is_valid_row (k_row + row))
                    continue;

                for (k_col = 3 - abs (k_row); k_col >= -2; k_col -= 2 * abs (k_col))
                {
                    if (!is_valid_column (k_col + col))
                        continue;

                    Move knight_move = make_move (k_row + row, k_col + col, row, col);
                    knight_moves[k_row + row][k_col + col].push_back (knight_move);
                }
            }
        }
    }

    static inline int is_pawn_unmoved (const Board &board, int row, int col)
    {
        ColoredPiece piece = piece_at (board, row, col);

        if (piece_color (piece) == Color::White)
            return row == 6;
        else
            return row == 1;
    }

    static int valid_castling_move (const Board &board, Move move)
    {
        // check for an intervening piece
        int direction;
        Coord src, dst;
        ColoredPiece piece1, piece2, piece3;

        src = move_src (move);
        dst = move_dst (move);

        piece3 = make_piece (Color::None, Piece::None);

        // find which direction the king was castling in
        direction = (COLUMN (dst) - COLUMN (src)) / 2;

        piece1 = piece_at (board, ROW (src), COLUMN (dst) - direction);
        piece2 = piece_at (board, ROW (src), COLUMN (dst));

        if (direction < 0)
        {
            // check for piece next to rook on queenside
            piece3 = piece_at (board, ROW (src), COLUMN (dst) - 1);
        }

        return piece_type (piece1) == Piece::None &&
               piece_type (piece2) == Piece::None &&
               piece_type (piece3) == Piece::None;
    }

    static std::optional<Move> validate_move (const Board &board, Move move)
    {
        Coord src, dst;
        ColoredPiece src_piece, dst_piece;

        src = move_src (move);
        dst = move_dst (move);

        src_piece = piece_at (board, src);
        dst_piece = piece_at (board, dst);

        assert (piece_type (src_piece) != Piece::None);
        assert (piece_color (src_piece) != Color::None);

        bool is_capture = (piece_type (dst_piece) != Piece::None);

        if (is_en_passant_move (move))
            is_capture = true;

        if (is_castling_move (move))
        {
            if (!valid_castling_move (board, move))
                return {};
        }

        if (piece_color (src_piece) == piece_color (dst_piece))
        {
            assert (piece_type (dst_piece) != Piece::None);
            assert (is_capture);
            return {};
        }

        // check for an illegal king capture
        assert (piece_type (dst_piece) != Piece::King);

        if (is_capture)
        {
            if (!is_capture_move (move) && !is_en_passant_move (move))
                move = copy_move_with_capture (move);
        }

        return move;
    }

    static void append_move (const Board &board, MoveList &list, Move move)
    {
        if (auto validated_move = validate_move (board, move); validated_move.has_value())
        {
            list.push_back (*validated_move);
        }
    }

    static void moves_none ([[maybe_unused]] const Board &board, [[maybe_unused]] Color who,
                            [[maybe_unused]] int piece_row, [[maybe_unused]] int piece_col,
                            [[maybe_unused]] MoveList &moves)
    {
        throw Error ("Generating moves for \"none\" piece.");
    }

    static void moves_king (const Board &board, Color who,
                            int piece_row, int piece_col, MoveList &moves)
    {
        for (int row = piece_row - 1; row < 8 && row <= piece_row + 1; row++)
        {
            if (!is_valid_row (row))
                continue;

            for (int col = piece_col - 1; col < 8 && col <= piece_col + 1; col++)
            {
                if (!is_valid_column (col))
                    continue;

                append_move (board, moves, make_move (piece_row, piece_col, row, col));
            }
        }

        if (able_to_castle (board, who, Castle_Queenside) && piece_col == King_Column)
        {
            Move queenside_castle = make_castling_move (
                    piece_row, piece_col,
                    piece_row, piece_col - 2
            );
            append_move (board, moves, queenside_castle);
        }

        if (able_to_castle (board, who, Castle_Kingside) && piece_col == King_Column)
        {
            Move kingside_castle = make_castling_move (
                    piece_row, piece_col,
                    piece_row, piece_col + 2
            );
            append_move (board, moves, kingside_castle);
        }
    }

    static void moves_queen (const Board &board, Color who,
                             int piece_row, int piece_col, MoveList &moves)
    {
        // use the generators for bishop and rook
        moves_bishop (board, who, piece_row, piece_col, moves);
        moves_rook (board, who, piece_row, piece_col, moves);
    }

    static void moves_rook (const Board &board, [[maybe_unused]] Color who,
                            int piece_row, int piece_col, MoveList &moves)
    {
        int dir;
        int row, col;
        ColoredPiece piece;

        for (dir = -1; dir <= 1; dir += 2)
        {
            for (row = next_row (piece_row, dir); is_valid_row (row); row = next_row (row, dir))
            {
                piece = piece_at (board, row, piece_col);

                append_move (board, moves, make_move (piece_row, piece_col, row, piece_col));

                if (piece_type (piece) != Piece::None)
                    break;
            }

            for (col = next_column (piece_col, dir); is_valid_column (col); col = next_column (col, dir))
            {
                piece = piece_at (board, piece_row, col);

                append_move (board, moves, make_move (piece_row, piece_col, piece_row, col));

                if (piece_type (piece) != Piece::None)
                    break;
            }
        }
    }

    static void moves_bishop (const Board &board, [[maybe_unused]] Color who,
                              int piece_row, int piece_col, MoveList &moves)
    {
        int r_dir, c_dir;
        int row, col;

        for (r_dir = -1; r_dir <= 1; r_dir += 2)
        {
            for (c_dir = -1; c_dir <= 1; c_dir += 2)
            {
                for (row = next_row (piece_row, r_dir), col = next_column (piece_col, c_dir);
                     is_valid_row (row) && is_valid_column (col);
                     row = next_row (row, r_dir), col = next_column (col, c_dir))
                {
                    ColoredPiece piece = piece_at (board, row, col);

                    append_move (board, moves, make_move (piece_row, piece_col, row, col));

                    if (piece_type (piece) != Piece::None)
                        break;
                }
            }
        }
    }

    static void moves_knight ([[maybe_unused]] const Board &board, [[maybe_unused]] Color who,
                              int piece_row, int piece_col, MoveList &moves)
    {
        const auto &kt_moves = generate_knight_moves (piece_row, piece_col);

        for (auto knight_move : kt_moves)
            append_move (board, moves, knight_move);
    }

    // Returns -1 if no column is eligible.
    static int eligible_en_passant_column (const Board &board, int row, int column, Color who)
    {
        ColorIndex opponent_index = color_index (color_invert (who));

        if (coord_equals (board.en_passant_target[opponent_index], No_En_Passant_Coord))
            return -1;

        // if WHITE rank 4, black rank 3
        if ((who == Color::White ? 3 : 4) != row)
            return -1;

        int left_column = column - 1;
        int right_column = column + 1;
        int target_column = COLUMN (board.en_passant_target[opponent_index]);

        if (left_column == target_column)
        {
            assert (is_valid_column (left_column));
            return left_column;
        }

        if (right_column == target_column)
        {
            assert (is_valid_column (right_column));
            return right_column;
        }

        return -1;
    }

    static void moves_pawn (const Board &board, Color who,
                            int piece_row, int piece_col, MoveList &moves)
    {
        int dir;
        int row;
        int take_col;
        int c_dir;
        ColoredPiece piece;

        dir = pawn_direction (who);

        // row is _guaranteed_ to be on the board, because
        // a pawn on the eight rank can't remain a pawn, and that's
        // the only direction moved in
        assert (is_valid_row (piece_row));

        row = next_row (piece_row, dir);
        assert (is_valid_row (row));

        std::array<std::optional<Move>, 4> all_pawn_moves { std::nullopt, std::nullopt, std::nullopt, std::nullopt };

        // single move
        if (piece_type (piece_at (board, row, piece_col)) == Piece::None)
            all_pawn_moves[0] = make_move (piece_row, piece_col, row, piece_col);

        // double move
        if (is_pawn_unmoved (board, piece_row, piece_col))
        {
            int double_row = next_row (row, dir);

            if (all_pawn_moves[0].has_value () &&
                piece_type (piece_at (board, double_row, piece_col)) == Piece::None)
            {
                all_pawn_moves[1] = make_move (piece_row, piece_col, double_row, piece_col);
            }
        }

        // take pieces
        for (c_dir = -1; c_dir <= 1; c_dir += 2)
        {
            take_col = next_column (piece_col, c_dir);

            if (!is_valid_column (take_col))
                continue;

            piece = piece_at (board, row, take_col);

            if (piece_type (piece_at (board, row, take_col)) != Piece::None &&
                piece_color (piece) != who)
            {
                if (c_dir == -1)
                    all_pawn_moves[2] = make_capturing_move (piece_row, piece_col, row, take_col);
                else
                    all_pawn_moves[3] = make_capturing_move (piece_row, piece_col, row, take_col);
            }
        }

        // promotion
        if (need_pawn_promotion (row, who))
        {
            for (auto promotable_piece_type : all_promotable_piece_types)
            {
                auto promoted_piece = make_piece (who, promotable_piece_type);

                // promotion moves dont include en passant
                for (auto &optional_move: all_pawn_moves)
                {
                    if (optional_move.has_value ())
                    {
                        auto move = *optional_move;
                        move = copy_move_with_promotion (move, promoted_piece);
                        append_move (board, moves, move);
                    }
                }
            }

            return;
        }

        // en passant
        int en_passant_column = eligible_en_passant_column (board, piece_row, piece_col, who);
        if (is_valid_column (en_passant_column))
            add_en_passant_move (board, who, piece_row, piece_col, moves, en_passant_column);

        for (auto &check_pawn_move : all_pawn_moves)
            if (check_pawn_move.has_value ())
                append_move (board, moves, *check_pawn_move);
    }

    // put en passant in a separate handler
    // in order to not pollute instruction cache with it
    static void add_en_passant_move (const Board &board, Color who, int piece_row, int piece_col,
                                     MoveList &moves, int en_passant_column)
    {
        Move new_move;
        int direction;
        int take_row, take_col;

        direction = pawn_direction (who);

        take_row = next_row (piece_row, direction);
        take_col = en_passant_column;

        [[maybe_unused]]
        ColoredPiece take_piece = piece_at (board, piece_row, take_col);

        assert (piece_type (take_piece) == Piece::Pawn);
        assert (piece_color (take_piece) == color_invert (who));

        new_move = make_en_passant_move (piece_row, piece_col, take_row, take_col);

        append_move (board, moves, new_move);
    }

    const MoveList &generate_knight_moves (int row, int col)
    {
        return get_knight_moves (row, col);
    }

    MoveList generate_legal_moves (Board &board, Color who)
    {
        MoveList non_checks;

        MoveList all_moves = generate_moves (board, who);
        for (auto move : all_moves)
        {
            UndoMove undo_state = do_move (board, who, move);

            if (was_legal_move (board, who, move))
                non_checks.push_back (move);

            undo_move (board, who, move, undo_state);
        }

        return non_checks;
    }

    MoveList generate_moves (const Board &board, Color who)
    {
        MoveList new_moves;

        for (const auto coord : All_Coords_Iterator)
        {
            ColoredPiece piece = piece_at (board, coord);

            if (piece_type (piece) == Piece::None)
                continue;

            Color color = piece_color (piece);
            if (color != who)
                continue;

            auto[row, col] = coord;

            (*move_functions[piece_index (piece_type (piece))])
                    (board, who, row, col, new_moves);
        }

        return new_moves;
    }

    ScoredMoveList MoveGenerator::to_scored_move_list (const Board &board, Color who,
                                                       const MoveList &move_list)
    {
        ScoredMoveList result;
        auto default_transposition = RelativeTransposition::from_defaults ();

        result.reserve (move_list.size ());
        for (auto move : move_list)
        {
            auto board_code = board.code.with_move (board, move);

            RelativeTransposition transposition = my_transposition_table.lookup (board_code.hash_code (), who)
                    .value_or (default_transposition);

            result.push_back ({ move, transposition.score });
        }

        return result;
    }

    ScoredMoveList MoveGenerator::generate (const Board &board, Color who)
    {
        auto move_list = generate_moves (board, who);
        auto scored_moves = to_scored_move_list (board, who, move_list);

        std::stable_sort (scored_moves.begin(), scored_moves.end(),[](ScoredMove a, ScoredMove b){
              return a.score > b.score;
        });

        return scored_moves;
    }
}