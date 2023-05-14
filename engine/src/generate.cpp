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
        const MoveGenerator& generator;

        void generate (ColoredPiece piece, Coord coord);

        [[nodiscard]] auto compare_moves (const Move& a, const Move& b) const -> bool;

        void none();
        void pawn();
        void knight();
        void bishop();
        void rook();
        void queen();
        void king();

        void en_passant (int en_passant_column);

        [[nodiscard]] static auto transform_move (ColoredPiece dst_piece, Move move) noexcept
            -> Move;
        void append_move (Move move) noexcept;
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

    void MoveGenerator::knight_move_list_init () const
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

                    Move knight_move = Move::make (k_row + row, k_col + col, row, col);
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

    auto MoveGenerator::generate_knight_moves (int row, int col) const -> const MoveList&
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

        Coord src = move.get_src ();
        Coord dst = move.get_dst ();

        ColoredPiece piece3 = ColoredPiece::make (Color::None, Piece::None);

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

    auto MoveGeneration::transform_move (ColoredPiece dst_piece, Move move) noexcept
        -> Move
    {
        bool is_capture = (piece_type (dst_piece) != Piece::None);
        if (is_capture && !move.is_en_passant () && !move.is_normal_capturing ())
            move = move.with_capture ();

        return move;
    }

    void MoveGeneration::append_move (Move move) noexcept
    {
        Coord src = move.get_src ();
        Coord dst = move.get_dst ();

        ColoredPiece src_piece = board.piece_at (src);
        ColoredPiece dst_piece = board.piece_at (dst);

        assert (piece_type (src_piece) != Piece::None);
        assert (piece_color (src_piece) != Color::None);

        if (piece_color (src_piece) == piece_color (dst_piece))
            return;

        auto transformed_move = transform_move (dst_piece, move);
        moves.push_back (transformed_move);
    }

    void MoveGeneration::none ()
    {
    }

    void MoveGeneration::king ()
    {
        for (int row = piece_row - 1; row < 8 && row <= piece_row + 1; row++)
        {
            if (!is_valid_row (row))
                continue;

            for (int col = piece_col - 1; col < 8 && col <= piece_col + 1; col++)
            {
                if (!is_valid_column (col))
                    continue;

                append_move (Move::make (piece_row, piece_col, row, col));
            }
        }

        if (board.able_to_castle (who, CastlingEligible::QueensideIneligible) && piece_col == King_Column)
        {
            Move queenside_castle = Move::make_castling (piece_row, piece_col, piece_row, piece_col - 2);
            if (valid_castling_move (board, queenside_castle))
                append_move (queenside_castle);
        }

        if (board.able_to_castle (who, CastlingEligible::KingsideIneligible) && piece_col == King_Column)
        {
            Move kingside_castle = Move::make_castling (piece_row, piece_col, piece_row, piece_col + 2);
            if (valid_castling_move (board, kingside_castle))
                append_move (kingside_castle);
        }
    }

    void MoveGeneration::rook ()
    {
        int dir;
        int row, col;

        for (dir = -1; dir <= 1; dir += 2)
        {
            for (row = next_row (piece_row, dir); is_valid_row (row); row = next_row (row, dir))
            {
                ColoredPiece piece = board.piece_at (row, piece_col);

                append_move (Move::make (piece_row, piece_col, row, piece_col));

                if (piece_type (piece) != Piece::None)
                    break;
            }

            for (col = next_column (piece_col, dir); is_valid_column (col); col = next_column (col, dir))
            {
                ColoredPiece piece = board.piece_at (piece_row, col);

                append_move (Move::make (piece_row, piece_col, piece_row, col));

                if (piece_type (piece) != Piece::None)
                    break;
            }
        }
    }

    void MoveGeneration::bishop ()
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

                    append_move (Move::make (piece_row, piece_col, row, col));

                    if (piece != Piece_And_Color_None)
                        break;
                }
            }
        }
    }

    void MoveGeneration::queen ()
    {
        bishop ();
        rook ();
    }

    void MoveGeneration::knight ()
    {
        const auto& kt_moves = generator.generate_knight_moves (piece_row, piece_col);

        for (const auto& knight_move : kt_moves)
            append_move (knight_move);
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

    void MoveGeneration::pawn ()
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
            all_pawn_moves[0] = Move::make (piece_row, piece_col, row, piece_col);

        // double move
        if (is_pawn_unmoved (board, piece_row, piece_col))
        {
            int double_row = next_row (row, dir);

            if (all_pawn_moves[0].has_value () &&
                board.piece_at (double_row, piece_col) == Piece_And_Color_None)
            {
                all_pawn_moves[1] = Move::make (piece_row, piece_col, double_row, piece_col);
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
                    all_pawn_moves[2] = Move::make_normal_capturing (piece_row, piece_col, row, take_col);
                else
                    all_pawn_moves[3] = Move::make_normal_capturing (piece_row, piece_col, row, take_col);
            }
        }

        // promotion
        if (need_pawn_promotion (row, who))
        {
            for (auto promotable_piece_type : All_Promotable_Piece_Types)
            {
                auto promoted_piece = ColoredPiece::make (who, promotable_piece_type);

                // promotion moves dont include en passant
                for (auto& optional_move: all_pawn_moves)
                {
                    if (optional_move.has_value ())
                    {
                        auto move = *optional_move;
                        move = move.with_promotion (promoted_piece);
                        append_move (move);
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
                append_move (*check_pawn_move);
    }

    // put en passant in a separate handler
    // in order to not pollute instruction cache with it
    void MoveGeneration::en_passant (int en_passant_column)
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

        Move new_move = Move::make_en_passant (piece_row, piece_col, take_row, take_col);

        append_move (new_move);
    }

    auto MoveGenerator::generate_legal_moves (const Board& board, Color who) const -> MoveList
    {
        MoveList non_checks { my_move_list_allocator.get () };

        MoveList all_moves = generate_all_potential_moves (board, who);
        for (auto move : all_moves)
        {
            Board new_board = board.with_move (who, move);

            if (is_legal_position_after_move (new_board, who, move))
                non_checks.push_back (move);
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
                none();
                return;
            case Piece::Pawn:
                pawn();
                return;
            case Piece::Knight:
                knight();
                return;
            case Piece::Bishop:
                bishop();
                return;
            case Piece::Rook:
                rook();
                return;
            case Piece::Queen:
                queen();
                return;
            case Piece::King:
                king();
                return;
        }
    }

    static int material_diff (const Board& board, Move move)
    {
        assert (move.is_any_capturing ());

        if (move.is_en_passant ())
        {
            return 0;
        }
        else
        {
            int a_material_src = Material::weight (
                piece_type (board.piece_at (move.get_src ()))
            );
            int a_material_dst = Material::weight (
                piece_type (board.piece_at (move.get_dst ()))
            );
            return a_material_dst - a_material_src;
        }
    }

    static constexpr auto promoting_or_coord_compare (const Move& a, const Move& b) -> bool
    {
        bool a_is_promoting = a.is_promoting ();
        bool b_is_promoting = b.is_promoting ();

        if (a_is_promoting && b_is_promoting)
        {
            return Material::weight (piece_type (a.get_promoted_piece ())) >
                Material::weight (piece_type (b.get_promoted_piece ()));
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
        Coord a_coord = a.get_src ();
        Coord b_coord = b.get_src ();

        if (a_coord != b_coord)
            return coord_index (a_coord) < coord_index (b_coord);
        else
            return coord_index (a.get_dst ()) < coord_index (b.get_dst ());
    }

    auto MoveGeneration::compare_moves (const Move& a, const Move& b) const -> bool
    {
        bool a_is_capturing = a.is_any_capturing ();
        bool b_is_capturing = b.is_any_capturing ();

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

    auto MoveGenerator::generate_all_potential_moves (const Board& board, Color who) const
        -> MoveList
    {
        MoveList result { my_move_list_allocator.get () };
        MoveGeneration generation { board, result, 0, 0, who, *this };

        for (auto coord : Board::all_coords())
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
