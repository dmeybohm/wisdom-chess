#include "wisdom-chess/engine/generate.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/threats.hpp"
#include "wisdom-chess/engine/coord.hpp"

namespace wisdom
{
    struct KnightMoveList
    {
        size_t size;
        array<Move, 8> moves;
    };

    using KnightMoveLists = array<KnightMoveList, Num_Squares>;

    namespace
    {
        consteval auto
        absoluteValue (auto integer)
            -> decltype (integer)
        {
            static_assert (std::is_integral_v<decltype (integer)>);
            return integer < 0 ? integer * -1 : integer;
        }

        consteval auto
        knightMoveListInit()
            -> KnightMoveLists
        {
            KnightMoveLists result {};

            for (auto coord : Board::allCoords())
            {
                auto row = coord.row<int>();
                auto col = coord.column<int>();

                for (int k_row = -2; k_row <= 2; k_row++)
                {
                    if (!k_row)
                        continue;

                    if (!isValidRow (k_row + row))
                        continue;

                    for (auto k_col = 3 - absoluteValue (k_row); k_col >= -2;
                         k_col -= 2 * absoluteValue (k_col))
                    {
                        if (!isValidColumn (k_col + col))
                            continue;

                        Move knight_move = Move::make (k_row + row, k_col + col, row, col);
                        auto index = knight_move.getSrc().index();

                        auto& size_ref = result[index].size;
                        auto& array_ref = result[index].moves;
                        array_ref[size_ref] = knight_move;
                        size_ref++;
                    }
                }
            }
            return result;
        }

        // Store a list of knight moves and their sizes, generated at
        // compile-time:
        constexpr KnightMoveLists Knight_Moves =
            knightMoveListInit();
    }

    struct MoveGeneration
    {
        const Board& board;
        nonnull<MoveList> moves;
        int piece_row;
        int piece_col;
        const Color who;
        MoveOrdering ordering;

        // Generate only captures and promotions to a queen.
        bool captures_only = false;

        void generate (ColoredPiece piece, Coord coord) noexcept;

        // The sort key of a move: a lower key is tried first.
        [[nodiscard]] auto
        sortKey (Move move) const noexcept
            -> uint64_t;

        // The killer slot a quiet move is in, or the slot count when it is
        // in none, so that a lower rank sorts first.
        [[nodiscard]] auto
        killerRank (const Move& move) const noexcept
            -> size_t;

        void pawn() noexcept;
        void knight() noexcept;
        void bishop() noexcept;
        void rook() noexcept;
        void slide (int row_direction, int col_direction) noexcept;
        void queen() noexcept;
        void king() noexcept;

        void enPassant (int en_passant_column) noexcept;

        // Get a std::span of the knight move list and the compile-time
        // calculated length:
        [[nodiscard]] static auto
        getKnightMoveList (int row, int col) noexcept
            -> span<const Move>
        {
            auto coord = Coord::make (row, col);
            const auto square = coord.index();

            const auto& list = Knight_Moves[square];
            return { list.moves.data(), list.size };
        }

        [[nodiscard]] static auto
        transformMove (ColoredPiece dst_piece, Move move) noexcept
            -> Move;

        void appendMove (Move move) noexcept;
    };

    namespace
    {
        auto isPawnUnmoved (const Board& board, int row, int col) noexcept -> bool
        {
            ColoredPiece piece = board.pieceAt (row, col);

            if (pieceColor (piece) == Color::White)
                return row == White_Pawn_Start_Row;
            else
                return row == Black_Pawn_Start_Row;
        }

        auto validCastlingMove (const Board& board, Move move) noexcept
            -> bool
        {
            // check for an intervening piece
            Coord src = move.getSrc();
            Coord dst = move.getDst();

            ColoredPiece piece3 = ColoredPiece::make (Color::None, Piece::None);

            // find which direction the king was castling in
            int direction = (dst.column() - src.column()) / 2;

            ColoredPiece piece1 = board.pieceAt (src.row(), dst.column() - direction);
            ColoredPiece piece2 = board.pieceAt (src.row(), dst.column());

            if (direction < 0)
            {
                // check for piece next to rook on queenside
                piece3 = board.pieceAt (src.row(), dst.column() - 1);
            }

            return pieceType (piece1) == Piece::None
                && pieceType (piece2) == Piece::None
                && pieceType (piece3) == Piece::None;
        }
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

        ASSERT( pieceType (src_piece) != Piece::None );
        ASSERT( pieceColor (src_piece) != Color::None );

        if (pieceColor (src_piece) == pieceColor (dst_piece))
            return;

        if (captures_only && pieceType (dst_piece) == Piece::None
            && !move.isEnPassant() && !move.isPromoting())
        {
            return;
        }

        auto transformed_move = transformMove (dst_piece, move);
        moves->append (transformed_move);
    }

    void MoveGeneration::king() noexcept
    {
        for (int row = piece_row - 1; row <= piece_row + 1; row++)
        {
            if (!isValidRow (row))
                continue;

            for (int col = piece_col - 1; col <= piece_col + 1; col++)
            {
                if (!isValidColumn (col))
                    continue;

                appendMove (Move::make (piece_row, piece_col, row, col));
            }
        }

        if (captures_only)
            return;

        if (board.ableToCastle (who, CastlingRights::Queenside) &&
            piece_col == King_Column)
        {
            Move queenside_castle
                = Move::makeCastling (piece_row, piece_col, piece_row, piece_col - 2);
            if (validCastlingMove (board, queenside_castle))
                appendMove (queenside_castle);
        }

        if (board.ableToCastle (who, CastlingRights::Kingside) &&
            piece_col == King_Column)
        {
            Move kingside_castle
                = Move::makeCastling (piece_row, piece_col, piece_row, piece_col + 2);
            if (validCastlingMove (board, kingside_castle))
                appendMove (kingside_castle);
        }
    }

    void MoveGeneration::slide (int row_direction, int col_direction) noexcept
    {
        for (int row = nextRow (piece_row, row_direction), col = nextColumn (piece_col, col_direction);
             isValidRow (row) && isValidColumn (col);
             row = nextRow (row, row_direction), col = nextColumn (col, col_direction))
        {
            ColoredPiece piece = board.pieceAt (row, col);

            appendMove (Move::make (piece_row, piece_col, row, col));

            if (piece != Piece_And_Color_None)
                break;
        }
    }

    void MoveGeneration::rook() noexcept
    {
        slide (-1, 0);
        slide (0, -1);
        slide (+1, 0);
        slide (0, +1);
    }

    void MoveGeneration::bishop() noexcept
    {
        slide (-1, -1);
        slide (-1, +1);
        slide (+1, -1);
        slide (+1, +1);
    }

    void MoveGeneration::queen() noexcept
    {
        bishop();
        rook();
    }

    void MoveGeneration::knight() noexcept
    {
        const auto& kt_moves = getKnightMoveList (piece_row, piece_col);

        for (const auto& knight_move : kt_moves)
            appendMove (knight_move);
    }

    auto
    eligibleEnPassantColumn (const Board& board, int row, int column, Color who) noexcept
        -> optional<int>
    {
        Color opponent = colorInvert (who);

        auto en_passant_target = board.getAnyEnPassantTarget();
        if (!en_passant_target.has_value() || en_passant_target->vulnerable_color != opponent)
            return nullopt;

        Coord target_coord = en_passant_target->coord;

        auto capture_row = who == Color::White
            ? White_Pawn_En_Passant_Capture_Row
            : Black_Pawn_En_Passant_Capture_Row;
        if (row != capture_row)
            return nullopt;

        int left_column = column - 1;
        int right_column = column + 1;
        int target_column = target_coord.column<int>();

        if (left_column == target_column)
        {
            ASSERT( isValidColumn (left_column) );
            return left_column;
        }

        if (right_column == target_column)
        {
            ASSERT( isValidColumn (right_column) );
            return right_column;
        }

        return nullopt;
    }

    void MoveGeneration::pawn() noexcept
    {
        int dir = pawnDirection<int> (who);

        // row is _guaranteed_ to be on the board, because
        // a pawn on the eight rank can't remain a pawn, and that's
        // the only direction moved in
        ASSERT( isValidRow (piece_row) );

        int row = nextRow (piece_row, dir);
        ASSERT( isValidRow (row) );

        array<optional<Move>, 4> all_pawn_moves { nullopt, nullopt, nullopt, nullopt };

        // single move
        if (pieceType (board.pieceAt (row, piece_col)) == Piece::None)
            all_pawn_moves[0] = Move::make (piece_row, piece_col, row, piece_col);

        // double move
        if (isPawnUnmoved (board, piece_row, piece_col))
        {
            int double_row = nextRow (row, dir);

            if (all_pawn_moves[0].has_value()
                && board.pieceAt (double_row, piece_col) == Piece_And_Color_None)
            {
                all_pawn_moves[1] = Move::make (piece_row, piece_col, double_row, piece_col);
            }
        }

        // take pieces
        for (int c_dir = -1; c_dir <= 1; c_dir += 2)
        {
            int take_col = nextColumn (piece_col, c_dir);

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
                if (captures_only && promotable_piece_type != Piece::Queen)
                    continue;

                // promotion moves dont include en passant
                for (auto& optional_move : all_pawn_moves)
                {
                    if (optional_move.has_value())
                    {
                        auto move = *optional_move;
                        move = move.withPromotion (promotable_piece_type);
                        appendMove (move);
                    }
                }
            }

            return;
        }

        // en passant, skipped when no capture of the target is legal
        if (board.getLegalEnPassantTarget().has_value())
        {
            optional<int> en_passant_column
                = eligibleEnPassantColumn (board, piece_row, piece_col, who);
            if (en_passant_column.has_value())
                enPassant (*en_passant_column);
        }

        for (const auto& check_pawn_move : all_pawn_moves)
            if (check_pawn_move.has_value())
                appendMove (*check_pawn_move);
    }

    void MoveGeneration::enPassant (int en_passant_column) noexcept
    {
        int direction = pawnDirection<int> (who);

        int take_row = nextRow (piece_row, direction);
        int take_col = en_passant_column;

        [[maybe_unused]] ColoredPiece take_piece = board.pieceAt (piece_row, take_col);

        ASSERT( pieceType (take_piece) == Piece::Pawn );
        ASSERT( pieceColor (take_piece) == colorInvert (who) );

        Move new_move = Move::makeEnPassant (piece_row, piece_col, take_row, take_col);

        appendMove (new_move);
    }

    void MoveGeneration::generate (ColoredPiece piece, Coord coord) noexcept
    {
        piece_row = coord.row<int>();
        piece_col = coord.column<int>();

        switch (pieceType (piece))
        {
            case Piece::None:
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

    namespace
    {
        auto
        materialDiff (const Board& board, Move move) noexcept
            -> int
        {
            ASSERT( move.isAnyCapturing() );

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

        // Ranks a promotion by its piece, highest first, and puts any
        // promotion ahead of a move that is not one.
        constexpr auto
        promotionRank (Move move) noexcept
            -> uint64_t
        {
            switch (move.getPromotedPiece())
            {
                case Piece::Queen: return 0;
                case Piece::Rook: return 1;
                case Piece::Bishop: return 2;
                case Piece::Knight: return 3;
                default: return 4;
            }
        }

        // The fields of a sort key, from the most significant: the kind of
        // move, then a score within the kind, then the promotion, then the
        // source and destination squares, which make the order total.
        constexpr int Sort_Key_Kind_Shift = 48;
        constexpr int Sort_Key_Score_Shift = 16;
        constexpr int Sort_Key_Promotion_Shift = 12;

        enum class SortKind : uint64_t
        {
            Priority,
            Capture,
            Promotion,
            FirstKiller,
            SecondKiller,
            Quiet,
        };

        // Keeps a capture's material difference, which can be negative,
        // within the unsigned score field.
        constexpr int Material_Diff_Offset = 2 * Weight_King;

        static_assert (CutoffHistory::Max_Score < (int64_t { 1 } << (Sort_Key_Kind_Shift - Sort_Key_Score_Shift)));

        constexpr auto
        makeSortKey (SortKind kind, uint64_t score, Move move) noexcept
            -> uint64_t
        {
            auto squares = to_unsigned_cast<uint64_t> (
                move.getSrc().index() * Num_Squares + move.getDst().index()
            );
            return (static_cast<uint64_t> (kind) << Sort_Key_Kind_Shift)
                | (score << Sort_Key_Score_Shift)
                | (promotionRank (move) << Sort_Key_Promotion_Shift)
                | squares;
        }
    }

    auto
    MoveGeneration::killerRank (const Move& move) const noexcept
        -> size_t
    {
        for (size_t slot = 0; slot < ordering.killers.size(); slot++)
        {
            if (ordering.killers[slot] == move)
                return slot;
        }
        return ordering.killers.size();
    }

    auto
    MoveGeneration::sortKey (Move move) const noexcept
        -> uint64_t
    {
        if (move == ordering.priority_move)
            return makeSortKey (SortKind::Priority, 0, move);

        if (move.isAnyCapturing())
        {
            auto score = to_unsigned_cast<uint64_t> (Material_Diff_Offset - materialDiff (board, move));
            return makeSortKey (SortKind::Capture, score, move);
        }

        if (move.isPromoting())
            return makeSortKey (SortKind::Promotion, 0, move);

        switch (killerRank (move))
        {
            case 0: return makeSortKey (SortKind::FirstKiller, 0, move);
            case 1: return makeSortKey (SortKind::SecondKiller, 0, move);
            default: break;
        }

        uint64_t score = 0;
        if (ordering.history)
        {
            score = to_unsigned_cast<uint64_t> (
                CutoffHistory::Max_Score - ordering.history.value()->getScore (who, move)
            );
        }
        return makeSortKey (SortKind::Quiet, score, move);
    }

    namespace
    {
        auto
        generateSortedMoves (
            const Board& board,
            Color who,
            const MoveOrdering& ordering,
            bool captures_only
        ) noexcept
            -> MoveList
        {
            MoveList result;
            MoveGeneration generation {
                board, &result, 0, 0, who, ordering, captures_only
            };

            for (auto coord : Board::allCoords())
            {
                ColoredPiece piece = board.pieceAt (coord);

                if (pieceColor (piece) != who)
                    continue;

                generation.generate (piece, coord);
            }

            struct KeyedMove
            {
                uint64_t key;
                Move move;
            };

            array<KeyedMove, Max_Move_List_Size> keyed; // NOLINT(*-pro-type-member-init)
            auto count = narrow_cast<size_t> (result.size());

            for (size_t i = 0; i < count; i++)
            {
                auto move = *(result.begin() + i);
                keyed[i] = KeyedMove { generation.sortKey (move), move };
            }

            std::sort (
                keyed.begin(),
                keyed.begin() + count,
                [] (const KeyedMove& a, const KeyedMove& b) { return a.key < b.key; }
            );

            for (size_t i = 0; i < count; i++)
                *(result.begin() + i) = keyed[i].move;

            return result;
        }
    }

    auto
    generateAllPotentialMoves (const Board& board, Color who, const MoveOrdering& ordering) noexcept
        -> MoveList
    {
        return generateSortedMoves (board, who, ordering, false);
    }

    auto
    generateAllPotentialMoves (const Board& board, Color who) noexcept
        -> MoveList
    {
        return generateAllPotentialMoves (board, who, MoveOrdering {});
    }

    auto
    generateCaptures (const Board& board, Color who) noexcept
        -> MoveList
    {
        return generateSortedMoves (board, who, MoveOrdering {}, true);
    }

    auto
    generateLegalMoves (const Board& board, Color who) noexcept
        -> MoveList
    {
        MoveList non_checks;

        MoveList all_moves = generateAllPotentialMoves (board, who);
        for (auto move : all_moves)
        {
            Board new_board = board.withMove (who, move);

            if (isLegalPositionAfterMove (new_board, who, move))
                non_checks.append (move);
        }

        return non_checks;
    }

    auto
    generateLegalEnPassantMoves (const Board& board) noexcept
        -> MoveList
    {
        MoveList result;

        Color who = board.getCurrentTurn();
        auto target = board.getAnyEnPassantTarget();
        if (!target.has_value() || target->vulnerable_color == who)
            return result;

        int capture_row = who == Color::White
            ? White_Pawn_En_Passant_Capture_Row
            : Black_Pawn_En_Passant_Capture_Row;
        int target_row = target->coord.row<int>();
        int target_column = target->coord.column<int>();

        // A target read from a FEN may have no pawn to capture.
        auto taken_pawn = ColoredPiece::make (target->vulnerable_color, Piece::Pawn);
        if (board.pieceAt (capture_row, target_column) != taken_pawn
            || board.pieceAt (target->coord) != Piece_And_Color_None)
        {
            return result;
        }

        auto capturing_pawn = ColoredPiece::make (who, Piece::Pawn);
        for (int column : { target_column - 1, target_column + 1 })
        {
            if (!isValidColumn (column))
                continue;

            if (board.pieceAt (capture_row, column) != capturing_pawn)
                continue;

            Move move = Move::makeEnPassant (capture_row, column, target_row, target_column);
            Board new_board = board.withMove (who, move);

            if (isLegalPositionAfterMove (new_board, who, move))
                result.append (move);
        }

        return result;
    }

    namespace
    {
        auto
        generatePieceMoves (const Board& board, Color who, Coord coord) noexcept
            -> MoveList
        {
            MoveList result;
            MoveGeneration generation { board, &result, 0, 0, who, MoveOrdering {} };

            generation.generate (board.pieceAt (coord), coord);

            return result;
        }

        auto
        pieceHasLegalMove (const Board& board, Color who, Coord coord) noexcept
            -> bool
        {
            for (auto move : generatePieceMoves (board, who, coord))
            {
                Board new_board = board.withMove (who, move);

                if (isLegalPositionAfterMove (new_board, who, move))
                    return true;
            }

            return false;
        }

        auto
        sharesLine (Coord a, Coord b) noexcept
            -> bool
        {
            int row_diff = a.row<int>() - b.row<int>();
            int col_diff = a.column<int>() - b.column<int>();

            return row_diff == 0 || col_diff == 0
                || row_diff == col_diff || row_diff == -col_diff;
        }

        // When the king is not in check, a move can only leave it attacked by
        // opening one of its lines: the piece leaves a square on a line, or an
        // en passant capture takes the pawn off one. Any other move is legal
        // without a test.
        auto
        hasMoveThatCannotExposeKing (const Board& board, Color who, Coord king_coord) noexcept
            -> bool
        {
            for (auto coord : Board::allCoords())
            {
                if (pieceColor (board.pieceAt (coord)) != who || sharesLine (coord, king_coord))
                    continue;

                for (auto move : generatePieceMoves (board, who, coord))
                {
                    if (!move.isEnPassant())
                        return true;
                }
            }

            return false;
        }
    }

    auto
    hasLegalMove (const Board& board, bool in_check) noexcept
        -> bool
    {
        Color who = board.getCurrentTurn();
        Coord king_coord = board.getKingPosition (who);

        ASSERT( in_check == isKingThreatened (board, who, king_coord) );

        if (!in_check && hasMoveThatCannotExposeKing (board, who, king_coord))
            return true;

        if (pieceHasLegalMove (board, who, king_coord))
            return true;

        for (auto coord : Board::allCoords())
        {
            if (pieceColor (board.pieceAt (coord)) != who || coord == king_coord)
                continue;

            if (pieceHasLegalMove (board, who, coord))
                return true;
        }

        return false;
    }

    auto
    hasLegalMove (const Board& board) noexcept
        -> bool
    {
        Color who = board.getCurrentTurn();
        bool in_check = isKingThreatened (board, who, board.getKingPosition (who));

        return hasLegalMove (board, in_check);
    }

    auto
    isLegalPositionAfterMove (const Board& board, Color who, Move mv) noexcept
        -> bool
    {
        auto king_coord = board.getKingPosition (who);

        if (isKingThreatened (board, who, king_coord))
            return false;

        if (mv.isCastling())
        {
            Coord castled_pos = mv.getDst();
            auto castled_row = castled_pos.row();
            auto castled_col = castled_pos.column();

            ASSERT( king_coord.row() == castled_row );
            ASSERT( king_coord.column() == castled_col );

            int8_t direction = mv.isCastlingOnKingside() ? -1 : 1;

            int8_t plus_one_column = nextColumn (castled_col, direction);
            int8_t plus_two_column = nextColumn (plus_one_column, direction);

            if (isKingThreatened (board, who, castled_row, plus_one_column)
                || isKingThreatened (board, who, castled_row, plus_two_column))
            {
                return false;
            }
        }

        return true;
    }

    auto isCheckmated (const Board& board) noexcept -> bool
    {
        auto who = board.getCurrentTurn();
        auto coord = board.getKingPosition (who);

        return isKingThreatened (board, who, coord) && !hasLegalMove (board);
    }

    auto isStalemated (const Board& board) noexcept -> bool
    {
        auto who = board.getCurrentTurn();
        auto coord = board.getKingPosition (who);

        return !isKingThreatened (board, who, coord) && !hasLegalMove (board);
    }

    auto
    needPawnPromotion (int row, Color who) noexcept
        -> bool
    {
        EXPECTS_NOEXCEPT( isColorValid (who) );
        return who == Color::White ? row == First_Row : row == Last_Row;
    }
}
