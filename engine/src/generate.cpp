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

        [[nodiscard]] auto compareMoves (const Move& a, const Move& b) const -> bool;

        void none();
        void pawn();
        void knight();
        void bishop();
        void rook();
        void queen();
        void king();

        void enPassant (int en_passant_column);

        [[nodiscard]] static auto transformMove (ColoredPiece dst_piece, Move move) noexcept
            -> Move;
        void appendMove (Move move) noexcept;
    };

    auto needPawnPromotion (int row, Color who) -> bool
    {
        assert (isColorValid (who));
        switch (who)
        {
            case Color::White: return 0 == row;
            case Color::Black: return 7 == row;
            default: throw Error { "Invalid color in needPawnPromotion()" };
        }
    }

    void MoveGenerator::knightMoveListInit() const
    {
        for (auto coord : CoordIterator {})
        {
            int row = Row<int> (coord);
            int col = Column<int> (coord);

            for (int k_row = -2; k_row <= 2; k_row++)
            {
                if (!k_row)
                    continue;

                if (!isValidRow (k_row + row))
                    continue;

                for (int k_col = 3 - abs (k_row); k_col >= -2; k_col -= 2 * abs (k_col))
                {
                    if (!isValidColumn (k_col + col))
                        continue;

                    Move knight_move = Move::make (k_row + row, k_col + col, row, col);
                    int dst_row = k_row + row;
                    int dst_col = k_col + col;
                    auto index = coordIndex (dst_row, dst_col);
                    if (my_knight_moves[index] == nullptr) {
                        my_knight_moves[index] = make_unique<MoveList> (my_move_list_allocator.get());
                    }
                    my_knight_moves[index]->pushBack (knight_move);
                }
            }
        }
    }

    auto MoveGenerator::generateKnightMoves (int row, int col) const -> const MoveList&
    {
        auto index = coordIndex (row, col);

        if (my_knight_moves[0] == nullptr)
            knightMoveListInit();

        return *my_knight_moves[index];
    }

    static auto isPawnUnmoved (const Board &board, int row, int col) -> bool
    {
        ColoredPiece piece = board.pieceAt (row, col);

        if (pieceColor (piece) == Color::White)
            return row == 6;
        else
            return row == 1;
    }

    static auto validCastlingMove (const Board &board, Move move) noexcept
        -> bool
    {
        // check for an intervening piece
        int direction;

        Coord src = move.getSrc();
        Coord dst = move.getDst();

        ColoredPiece piece3 = ColoredPiece::make (Color::None, Piece::None);

        // find which direction the king was castling in
        direction = (Column (dst) - Column (src)) / 2;

        ColoredPiece piece1 = board.pieceAt (Row (src), Column (dst) - direction);
        ColoredPiece piece2 = board.pieceAt (Row (src), Column (dst));

        if (direction < 0)
        {
            // check for piece next to rook on queenside
            piece3 = board.pieceAt (Row (src), Column (dst) - 1);
        }

        return pieceType (piece1) == Piece::None && pieceType (piece2) == Piece::None && pieceType (piece3) == Piece::None;
    }

    auto MoveGeneration::transformMove (ColoredPiece dst_piece, Move move) noexcept
        -> Move
    {
        bool is_capture = (pieceType (dst_piece) != Piece::None);
        if (is_capture && !move.isEnPassant() && !move.isNormalCapturing())
            move = move.withCapture();

        return move;
    }

    void MoveGeneration::appendMove (Move move) noexcept
    {
        Coord src = move.getSrc();
        Coord dst = move.getDst();

        ColoredPiece src_piece = board.pieceAt (src);
        ColoredPiece dst_piece = board.pieceAt (dst);

        assert (pieceType (src_piece) != Piece::None);
        assert (pieceColor (src_piece) != Color::None);

        if (pieceColor (src_piece) == pieceColor (dst_piece))
            return;

        auto transformed_move = transformMove (dst_piece, move);
        moves.pushBack (transformed_move);
    }

    void MoveGeneration::none()
    {
    }

    void MoveGeneration::king()
    {
        for (int row = piece_row - 1; row < 8 && row <= piece_row + 1; row++)
        {
            if (!isValidRow (row))
                continue;

            for (int col = piece_col - 1; col < 8 && col <= piece_col + 1; col++)
            {
                if (!isValidColumn (col))
                    continue;

                appendMove (Move::make (piece_row, piece_col, row, col));
            }
        }

        if (board.ableToCastle (who, CastlingEligible::QueensideIneligible) && piece_col == King_Column)
        {
            Move queenside_castle = Move::makeCastling (piece_row, piece_col, piece_row, piece_col - 2);
            if (validCastlingMove (board, queenside_castle))
                appendMove (queenside_castle);
        }

        if (board.ableToCastle (who, CastlingEligible::KingsideIneligible) && piece_col == King_Column)
        {
            Move kingside_castle = Move::makeCastling (piece_row, piece_col, piece_row, piece_col + 2);
            if (validCastlingMove (board, kingside_castle))
                appendMove (kingside_castle);
        }
    }

    void MoveGeneration::rook()
    {
        int dir;
        int row, col;

        for (dir = -1; dir <= 1; dir += 2)
        {
            for (row = nextRow (piece_row, dir); isValidRow (row); row = nextRow (row, dir))
            {
                ColoredPiece piece = board.pieceAt (row, piece_col);

                appendMove (Move::make (piece_row, piece_col, row, piece_col));

                if (pieceType (piece) != Piece::None)
                    break;
            }

            for (col = nextColumn (piece_col, dir); isValidColumn (col); col = nextColumn (col, dir))
            {
                ColoredPiece piece = board.pieceAt (piece_row, col);

                appendMove (Move::make (piece_row, piece_col, piece_row, col));

                if (pieceType (piece) != Piece::None)
                    break;
            }
        }
    }

    void MoveGeneration::bishop()
    {
        int r_dir, c_dir;
        int row, col;

        for (r_dir = -1; r_dir <= 1; r_dir += 2)
        {
            for (c_dir = -1; c_dir <= 1; c_dir += 2)
            {
                for (row = nextRow (piece_row, r_dir), col = nextColumn (piece_col, c_dir);
                     isValidRow (row) && isValidColumn (col);
                     row = nextRow (row, r_dir), col = nextColumn (col, c_dir))
                {
                    ColoredPiece piece = board.pieceAt (row, col);

                    appendMove (Move::make (piece_row, piece_col, row, col));

                    if (piece != Piece_And_Color_None)
                        break;
                }
            }
        }
    }

    void MoveGeneration::queen()
    {
        bishop();
        rook();
    }

    void MoveGeneration::knight()
    {
        const auto& kt_moves = generator.generateKnightMoves (piece_row, piece_col);

        for (const auto& knight_move : kt_moves)
            appendMove (knight_move);
    }

    // Returns -1 if no column is eligible.
    auto eligibleEnPassantColumn (const Board& board, int row, int column, Color who)
        -> optional<int>
    {
        Color opponent = colorInvert (who);

        if (!board.isEnPassantVulnerable (opponent))
            return nullopt;

        // if WHITE rank 4, black rank 3
        if ((who == Color::White ? 3 : 4) != row)
            return nullopt;

        int left_column = column - 1;
        int right_column = column + 1;
        int target_column = Column<int> (board.getEnPassantTarget (opponent));

        if (left_column == target_column)
        {
            assert (isValidColumn (left_column));
            return left_column;
        }

        if (right_column == target_column)
        {
            assert (isValidColumn (right_column));
            return right_column;
        }

        return nullopt;
    }

    void MoveGeneration::pawn()
    {
        int dir;
        int row;
        int take_col;
        int c_dir;

        dir = pawnDirection<int> (who);

        // row is _guaranteed_ to be on the board, because
        // a pawn on the eight rank can't remain a pawn, and that's
        // the only direction moved in
        assert (isValidRow (piece_row));

        row = nextRow (piece_row, dir);
        assert (isValidRow (row));

        array<optional<Move>, 4> all_pawn_moves { nullopt, nullopt, nullopt, nullopt };

        // single move
        if (pieceType (board.pieceAt (row, piece_col)) == Piece::None)
            all_pawn_moves[0] = Move::make (piece_row, piece_col, row, piece_col);

        // double move
        if (isPawnUnmoved (board, piece_row, piece_col))
        {
            int double_row = nextRow (row, dir);

            if (all_pawn_moves[0].has_value() && board.pieceAt (double_row, piece_col) == Piece_And_Color_None)
            {
                all_pawn_moves[1] = Move::make (piece_row, piece_col, double_row, piece_col);
            }
        }

        // take pieces
        for (c_dir = -1; c_dir <= 1; c_dir += 2)
        {
            take_col = nextColumn (piece_col, c_dir);

            if (!isValidColumn (take_col))
                continue;

            ColoredPiece target_piece = board.pieceAt (row, take_col);

            if (target_piece != Piece_And_Color_None && pieceColor (target_piece) != who)
            {
                if (c_dir == -1)
                    all_pawn_moves[2] = Move::makeNormalCapturing (piece_row, piece_col, row, take_col);
                else
                    all_pawn_moves[3] = Move::makeNormalCapturing (piece_row, piece_col, row, take_col);
            }
        }

        // promotion
        if (needPawnPromotion (row, who))
        {
            for (auto promotable_piece_type : All_Promotable_Piece_Types)
            {
                auto promoted_piece = ColoredPiece::make (who, promotable_piece_type);

                // promotion moves dont include en passant
                for (auto& optional_move: all_pawn_moves)
                {
                    if (optional_move.has_value())
                    {
                        auto move = *optional_move;
                        move = move.withPromotion (promoted_piece);
                        appendMove (move);
                    }
                }
            }

            return;
        }

        // en passant
        optional<int> en_passant_column
            = eligibleEnPassantColumn (board, piece_row, piece_col, who);
        if (en_passant_column.has_value())
            enPassant (*en_passant_column);

        for (const auto& check_pawn_move : all_pawn_moves)
            if (check_pawn_move.has_value())
                appendMove (*check_pawn_move);
    }

    // put en passant in a separate handler
    // in order to not pollute instruction cache with it
    void MoveGeneration::enPassant (int en_passant_column)
    {
        int direction;
        int take_row, take_col;

        direction = pawnDirection<int> (who);

        take_row = nextRow (piece_row, direction);
        take_col = en_passant_column;

        [[maybe_unused]]
        ColoredPiece take_piece = board.pieceAt (piece_row, take_col);

        assert (pieceType (take_piece) == Piece::Pawn);
        assert (pieceColor (take_piece) == colorInvert (who));

        Move new_move = Move::makeEnPassant (piece_row, piece_col, take_row, take_col);

        appendMove (new_move);
    }

    auto MoveGenerator::generateLegalMoves (const Board& board, Color who) const -> MoveList
    {
        MoveList non_checks { my_move_list_allocator.get() };

        MoveList all_moves = generateAllPotentialMoves (board, who);
        for (auto move : all_moves)
        {
            Board new_board = board.withMove (who, move);

            if (isLegalPositionAfterMove (new_board, who, move))
                non_checks.pushBack (move);
        }

        return non_checks;
    }

    void MoveGeneration::generate (ColoredPiece piece, Coord coord)
    {
        this->piece_row = Row<int> (coord);
        this->piece_col = Column<int> (coord);

        switch (pieceType (piece))
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

    static int materialDiff (const Board& board, Move move)
    {
        assert (move.isAnyCapturing());

        if (move.isEnPassant())
        {
            return 0;
        }
        else
        {
            int a_material_src = Material::weight (pieceType (board.pieceAt (move.getSrc())));
            int a_material_dst = Material::weight (pieceType (board.pieceAt (move.getDst())));
            return a_material_dst - a_material_src;
        }
    }

    static constexpr auto promotingOrCoordCompare (const Move& a, const Move& b) -> bool
    {
        bool a_is_promoting = a.isPromoting();
        bool b_is_promoting = b.isPromoting();

        if (a_is_promoting && b_is_promoting)
        {
            return Material::weight (pieceType (a.getPromotedPiece())) >
                Material::weight (pieceType (b.getPromotedPiece()));
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
        Coord a_coord = a.getSrc();
        Coord b_coord = b.getSrc();

        if (a_coord != b_coord)
            return coordIndex (a_coord) < coordIndex (b_coord);
        else
            return coordIndex (a.getDst()) < coordIndex (b.getDst());
    }

    auto MoveGeneration::compareMoves (const Move& a, const Move& b) const -> bool
    {
        bool a_is_capturing = a.isAnyCapturing();
        bool b_is_capturing = b.isAnyCapturing();

        if (!a_is_capturing && !b_is_capturing)
        {
            return promotingOrCoordCompare (a, b);
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
        auto material_diff_a = materialDiff (board, a);
        auto material_diff_b = materialDiff (board, b);

        if (material_diff_a != material_diff_b)
            return materialDiff (board, a) > materialDiff (board, b);
        else
            return promotingOrCoordCompare (a, b);
    }

    auto MoveGenerator::generateAllPotentialMoves (const Board& board, Color who) const
        -> MoveList
    {
        MoveList result { my_move_list_allocator.get() };
        MoveGeneration generation { board, result, 0, 0, who, *this };

        for (auto coord : board.allCoords())
        {
            ColoredPiece piece = board.pieceAt (coord);

            if (pieceColor (piece) != who)
                continue;

            generation.generate (piece, coord);
        }

        std::sort (result.begin(),
                   result.end(),
                   [generation](const Move& a, const Move& b) {
                        return generation.compareMoves (a, b);
                   });

        return result;
    }
}
