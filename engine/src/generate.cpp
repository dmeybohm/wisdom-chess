#include "generate.hpp"
#include "board.hpp"
#include "check.hpp"
#include "coord.hpp"

namespace wisdom
{
    struct MoveGeneration
    {
        const Board& board;
        MoveList& moves;
        int piece_row;
        int piece_col;
        const Color who;
        MoveGenerator& generator;

        void generate (ColoredPiece piece, Coord coord);

        [[nodiscard]] auto compare_moves (const Move& a, const Move& b) const -> bool;

        void none () const;
        void pawn () const;
        void knight () const;
        void bishop () const;
        void rook () const;
        void queen () const;
        void king () const;

        void en_passant (int en_passant_column) const;
    };

    auto need_pawn_promotion (int row, Color who) -> bool
    {
        assert (is_color_valid (who));
        switch (who)
        {
            case Color::White: return 0 == row;
            case Color::Black: return 7 == row;
            default: throw Error { "Invalid color in need_pawn_promotion()" };
        }
    }

    void MoveGenerator::knight_move_list_init ()
    {
        for (auto coord : CoordIterator {})
        {
            int row = Row<int> (coord);
            int col = Column<int> (coord);

            for (int k_row = -2; k_row <= 2; k_row++)
            {
                if (!k_row)
                    continue;

                if (!is_valid_row (k_row + row))
                    continue;

                for (int k_col = 3 - abs (k_row); k_col >= -2; k_col -= 2 * abs (k_col))
                {
                    if (!is_valid_column (k_col + col))
                        continue;

                    Move knight_move = make_regular_move (k_row + row, k_col + col, row, col);
                    int dst_row = k_row + row;
                    int dst_col = k_col + col;
                    auto index = coord_index (dst_row, dst_col);
                    if (my_knight_moves[index] == nullptr) {
                        my_knight_moves[index] = make_unique<MoveList> (my_move_list_allocator.get ());
                    }
                    my_knight_moves[index]->push_back (knight_move);
                }
            }
        }
    }

    auto MoveGenerator::generate_knight_moves (int row, int col) -> const MoveList&
    {
        auto index = coord_index (row, col);

        if (my_knight_moves[0] == nullptr)
            knight_move_list_init ();

        return *my_knight_moves[index];
    }

    static auto is_pawn_unmoved (const Board &board, int row, int col) -> bool
    {
        ColoredPiece piece = board.piece_at (row, col);

        if (piece_color (piece) == Color::White)
            return row == 6;
        else
            return row == 1;
    }

    static auto valid_castling_move (const Board &board, Move move) noexcept
        -> bool
    {
        // check for an intervening piece
        int direction;

        Coord src = move_src (move);
        Coord dst = move_dst (move);

        ColoredPiece piece3 = make_piece (Color::None, Piece::None);

        // find which direction the king was castling in
        direction = (Column (dst) - Column (src)) / 2;

        ColoredPiece piece1 = board.piece_at (Row (src), Column (dst) - direction);
        ColoredPiece piece2 = board.piece_at (Row (src), Column (dst));

        if (direction < 0)
        {
            // check for piece next to rook on queenside
            piece3 = board.piece_at (Row (src), Column (dst) - 1);
        }

        return piece_type (piece1) == Piece::None &&
               piece_type (piece2) == Piece::None &&
               piece_type (piece3) == Piece::None;
    }

    static auto transform_move (const Board &board, ColoredPiece dst_piece, Move move) noexcept
        -> Move
    {
        bool is_capture = (piece_type (dst_piece) != Piece::None);
        if (is_capture && !is_special_en_passant_move (move) && !is_normal_capturing_move (move))
            move = copy_move_with_capture (move);

        return move;
    }

    static void append_move (const Board &board, MoveList& list, Move move) noexcept
    {
        Coord src = move_src (move);
        Coord dst = move_dst (move);

        ColoredPiece src_piece = board.piece_at (src);
        ColoredPiece dst_piece = board.piece_at (dst);

        assert (piece_type (src_piece) != Piece::None);
        assert (piece_color (src_piece) != Color::None);

        if (piece_color (src_piece) == piece_color (dst_piece))
            return;

        auto transformed_move = transform_move (board, dst_piece, move);
        list.push_back (transformed_move);
    }

    void MoveGeneration::none () const
    {
    }

    void MoveGeneration::king () const
    {
        for (int row = piece_row - 1; row < 8 && row <= piece_row + 1; row++)
        {
            if (!is_valid_row (row))
                continue;

            for (int col = piece_col - 1; col < 8 && col <= piece_col + 1; col++)
            {
                if (!is_valid_column (col))
                    continue;

                append_move (board, moves, make_regular_move (piece_row, piece_col, row, col));
            }
        }

        if (board.able_to_castle ( who, Castle_Queenside) && piece_col == King_Column)
        {
            Move queenside_castle = make_special_castling_move (piece_row, piece_col, piece_row, piece_col - 2);
            if (valid_castling_move (board, queenside_castle))
                append_move (board, moves, queenside_castle);
        }

        if (board.able_to_castle ( who, Castle_Kingside) && piece_col == King_Column)
        {
            Move kingside_castle = make_special_castling_move (piece_row, piece_col, piece_row, piece_col + 2);
            if (valid_castling_move (board, kingside_castle))
                append_move (board, moves, kingside_castle);
        }
    }

    void MoveGeneration::rook () const
    {
        int dir;
        int row, col;

        for (dir = -1; dir <= 1; dir += 2)
        {
            for (row = next_row (piece_row, dir); is_valid_row (row); row = next_row (row, dir))
            {
                ColoredPiece piece = board.piece_at (row, piece_col);

                append_move (board, moves,
                             make_regular_move (piece_row, piece_col, row, piece_col));

                if (piece_type (piece) != Piece::None)
                    break;
            }

            for (col = next_column (piece_col, dir); is_valid_column (col); col = next_column (col, dir))
            {
                ColoredPiece piece = board.piece_at (piece_row, col);

                append_move (board, moves,
                             make_regular_move (piece_row, piece_col, piece_row, col));

                if (piece_type (piece) != Piece::None)
                    break;
            }
        }
    }

    void MoveGeneration::bishop () const
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
                    ColoredPiece piece = board.piece_at (row, col);

                    append_move (board, moves, make_regular_move (piece_row, piece_col, row, col));

                    if (piece != Piece_And_Color_None)
                        break;
                }
            }
        }
    }

    void MoveGeneration::queen () const
    {
        bishop ();
        rook ();
    }

    void MoveGeneration::knight () const
    {
        const auto& kt_moves = generator.generate_knight_moves (piece_row, piece_col);

        for (const auto& knight_move : kt_moves)
            append_move (board, moves, knight_move);
    }

    // Returns -1 if no column is eligible.
    auto eligible_en_passant_column (const Board& board, int row, int column, Color who)
        -> int
    {
        Color opponent = color_invert (who);

        if (!board.is_en_passant_vulnerable (opponent))
            return -1;

        // if WHITE rank 4, black rank 3
        if ((who == Color::White ? 3 : 4) != row)
            return -1;

        int left_column = column - 1;
        int right_column = column + 1;
        int target_column = Column<int> (board.get_en_passant_target (opponent));

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

    void MoveGeneration::pawn () const
    {
        int dir;
        int row;
        int take_col;
        int c_dir;

        dir = pawn_direction<int> (who);

        // row is _guaranteed_ to be on the board, because
        // a pawn on the eight rank can't remain a pawn, and that's
        // the only direction moved in
        assert (is_valid_row (piece_row));

        row = next_row (piece_row, dir);
        assert (is_valid_row (row));

        array<optional<Move>, 4> all_pawn_moves { nullopt, nullopt, nullopt, nullopt };

        // single move
        if (piece_type (board.piece_at (row, piece_col)) == Piece::None)
            all_pawn_moves[0] = make_regular_move (piece_row, piece_col, row, piece_col);

        // double move
        if (is_pawn_unmoved (board, piece_row, piece_col))
        {
            int double_row = next_row (row, dir);

            if (all_pawn_moves[0].has_value () &&
                board.piece_at (double_row, piece_col) == Piece_And_Color_None)
            {
                all_pawn_moves[1] = make_regular_move (piece_row, piece_col, double_row, piece_col);
            }
        }

        // take pieces
        for (c_dir = -1; c_dir <= 1; c_dir += 2)
        {
            take_col = next_column (piece_col, c_dir);

            if (!is_valid_column (take_col))
                continue;

            ColoredPiece target_piece = board.piece_at (row, take_col);

            if (target_piece != Piece_And_Color_None &&
                piece_color (target_piece) != who)
            {
                if (c_dir == -1)
                    all_pawn_moves[2] = make_normal_capturing_move (piece_row, piece_col, row, take_col);
                else
                    all_pawn_moves[3] = make_normal_capturing_move (piece_row, piece_col, row, take_col);
            }
        }

        // promotion
        if (need_pawn_promotion (row, who))
        {
            for (auto promotable_piece_type : All_Promotable_Piece_Types)
            {
                auto promoted_piece = make_piece (who, promotable_piece_type);

                // promotion moves dont include en passant
                for (auto& optional_move: all_pawn_moves)
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
            en_passant (en_passant_column);

        for (auto& check_pawn_move : all_pawn_moves)
            if (check_pawn_move.has_value ())
                append_move (board, moves, *check_pawn_move);
    }

    // put en passant in a separate handler
    // in order to not pollute instruction cache with it
    void MoveGeneration::en_passant (int en_passant_column) const
    {
        int direction;
        int take_row, take_col;

        direction = pawn_direction<int> (who);

        take_row = next_row (piece_row, direction);
        take_col = en_passant_column;

        [[maybe_unused]]
        ColoredPiece take_piece = board.piece_at (piece_row, take_col);

        assert (piece_type (take_piece) == Piece::Pawn);
        assert (piece_color (take_piece) == color_invert (who));

        Move new_move = make_special_en_passant_move (piece_row, piece_col, take_row, take_col);

        append_move (board, moves, new_move);
    }

    auto MoveGenerator::generate_legal_moves (Board& board, Color who) -> MoveList
    {
        MoveList non_checks { my_move_list_allocator.get () };

        MoveList all_moves = generate_all_potential_moves (board, who);
        for (auto move : all_moves)
        {
            UndoMove undo_state = board.make_move (who, move);

            if (was_legal_move (board, who, move))
                non_checks.push_back (move);

            board.take_back (who, move, undo_state);
        }

        return non_checks;
    }

    void MoveGeneration::generate (ColoredPiece piece, Coord coord)
    {
        this->piece_row = Row<int> (coord);
        this->piece_col = Column<int> (coord);

        switch (piece_type (piece))
        {
            case Piece::None:
                none ();
                return;
            case Piece::Pawn:
                pawn ();
                return;
            case Piece::Knight:
                knight ();
                return;
            case Piece::Bishop:
                bishop ();
                return;
            case Piece::Rook:
                rook ();
                return;
            case Piece::Queen:
                queen ();
                return;
            case Piece::King:
                king ();
                return;
        }
    }

    static int material_diff (const Board& board, Move move)
    {
        assert (is_any_capturing_move (move));

        if (is_special_en_passant_move (move))
        {
            return 0;
        }
        else
        {
            int a_material_src = Material::weight (
                piece_type (board.piece_at (move_src (move)))
            );
            int a_material_dst = Material::weight (
                piece_type (board.piece_at (move_dst (move)))
            );
            return a_material_dst - a_material_src;
        }
    }

    static constexpr auto promoting_or_coord_compare (const Move& a, const Move& b) -> bool
    {
        bool a_is_promoting = is_promoting_move (a);
        bool b_is_promoting = is_promoting_move (b);

        if (a_is_promoting && b_is_promoting)
        {
            return Material::weight (piece_type (move_get_promoted_piece (a))) >
                Material::weight (piece_type (move_get_promoted_piece (b)));
        }
        else if (a_is_promoting && !b_is_promoting)
        {
            return true;
        }
        else if (b_is_promoting && !a_is_promoting)
        {
            return false;
        }

        // return coordinate diff so order is consistent:
        Coord a_coord = move_src (a);
        Coord b_coord = move_src (b);

        if (a_coord != b_coord)
            return coord_index (a_coord) < coord_index (b_coord);
        else
            return coord_index (move_dst (a)) < coord_index (move_dst (b));
    }

    auto MoveGeneration::compare_moves (const Move& a, const Move& b) const -> bool
    {
        bool a_is_capturing = is_any_capturing_move (a);
        bool b_is_capturing = is_any_capturing_move (b);

        if (!a_is_capturing && !b_is_capturing)
        {
            return promoting_or_coord_compare (a, b);
        }

        if (a_is_capturing && !b_is_capturing)
        {
            return true;
        }
        else if (b_is_capturing && !a_is_capturing)
        {
            return false;
        }

        // both are capturing: return the biggest diff between source piece and dst piece:
        auto material_diff_a = material_diff (board, a);
        auto material_diff_b = material_diff (board, b);

        if (material_diff_a != material_diff_b)
            return material_diff (board, a) > material_diff (board, b);
        else
            return promoting_or_coord_compare (a, b);
    }

    auto MoveGenerator::generate_all_potential_moves (const Board& board, Color who)
        -> MoveList
    {
        MoveList result { my_move_list_allocator.get () };
        MoveGeneration generation { board, result, 0, 0, who, *this };

        for (auto coord : board.all_coords ())
        {
            ColoredPiece piece = board.piece_at (coord);

            if (piece_color (piece) != who)
                continue;

            generation.generate (piece, coord);
        }

        std::sort (result.begin (),
                   result.end (),
                   [generation](const Move& a, const Move& b) {
                        return generation.compare_moves (a, b);
                   });

        return result;
    }
}
