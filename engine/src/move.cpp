#include "move.hpp"
#include "board.hpp"
#include "generate.hpp"

#include <iostream>

namespace wisdom
{
    auto en_passant_taken_pawn_coord (Coord src, Coord dst) -> Coord
    {
        return make_coord (Row (src), Column (dst));
    }

    static void save_current_castle_state (UndoMove* undo_state, CastlingEligibility state)
    {
        undo_state->current_castle_state = pack_castle_state (state);
    }

    static void save_opponent_castle_state (UndoMove* undo_state, CastlingEligibility state)
    {
        undo_state->opponent_castle_state = pack_castle_state (state);
    }

    constexpr bool is_double_square_pawn_move (ColoredPiece src_piece, Move move)
    {
        Coord src = move.get_src ();
        Coord dst = move.get_dst ();
        return piece_type (src_piece) == Piece::Pawn && abs (Row (src) - Row (dst)) == 2;
    }

    // Returns the taken piece
    static auto apply_for_en_passant (Board* board, Color who, Coord src, Coord dst)
        -> ColoredPiece
    {
        Coord taken_pawn_pos = en_passant_taken_pawn_coord (src, dst);
        [[maybe_unused]] ColoredPiece taken_piece = board->piece_at (taken_pawn_pos);

        assert (piece_type (taken_piece) == Piece::Pawn);
        assert (piece_color (taken_piece) == color_invert (who));

        board->set_piece (taken_pawn_pos, Piece_And_Color_None);

        return ColoredPiece::make (color_invert (who), Piece::Pawn);
    }

    static void unapply_for_en_passant (Board* board, Color who, Coord src, Coord dst)
    {
        Coord taken_pawn_pos = en_passant_taken_pawn_coord (src, dst);

        ColoredPiece taken_pawn = ColoredPiece::make (color_invert (who), Piece::Pawn);
        board->set_piece (taken_pawn_pos, taken_pawn);
    }

    static auto get_castling_rook_move (const Board& board, Move move, Color who) -> Move
    {
        int src_row, src_col;
        int dst_row, dst_col;

        assert (move.is_castling ());

        Coord src = move.get_src ();
        Coord dst = move.get_dst ();

        src_row = Row<int> (src);
        dst_row = Row<int> (dst);

        if (Column (src) < Column (dst))
        {
            // castle to the right (kingside)
            src_col = Last_Column;
            dst_col = Column (dst) - 1;
        }
        else
        {
            // castle to the left (queenside)
            src_col = 0;
            dst_col = Column (dst) + 1;
        }

        if (!((piece_type (board.piece_at (src_row, src_col)) == Piece::Rook
               || piece_type (board.piece_at (dst_row, dst_col)) == Piece::Rook)))
        {
            throw MoveConsistencyProblem { "move considering: " + to_string (move) + "("
                                           + to_string (who) + " to move)" };
        }

        return Move::make (src_row, src_col, dst_row, dst_col);
    }

    static void apply_for_castling_move (Board* board, Color who, Move king_move,
                                         [[maybe_unused]] Coord src, [[maybe_unused]] Coord dst)
    {
        Move rook_move = get_castling_rook_move (*board, king_move, who);

        assert (piece_type (board->piece_at (src)) == Piece::King);
        assert (abs (Column (src) - Column (dst)) == 2);

        auto rook_src = rook_move.get_src ();
        auto rook_dst = rook_move.get_dst ();

        auto empty_piece = ColoredPiece::make (Color::None, Piece::None);

        auto rook = board->piece_at (rook_src);

        // do the rook move
        board->set_piece (rook_dst, rook);
        board->set_piece (rook_src, empty_piece);
    }

    static void unapply_for_castling (Board* board, Color who, Move king_move,
                                      [[maybe_unused]] Coord src, [[maybe_unused]] Coord dst)
    {
        Move rook_move = get_castling_rook_move (*board, king_move, who);

        assert (piece_type (board->piece_at (dst)) == Piece::King);
        assert (abs (Column (src) - Column (dst)) == 2);

        auto rook_src = rook_move.get_src ();
        auto rook_dst = rook_move.get_dst ();

        // undo the rook move
        auto rook = board->piece_at (rook_dst);

        // do the rook move
        board->set_piece (rook_dst, rook);
        board->set_piece (rook_src, Piece_And_Color_None);

        // undo the rook move
        board->set_piece (rook_dst, Piece_And_Color_None);
        board->set_piece (rook_src, rook);
    }

    static void apply_for_king_move (Board* board, Color who, UndoMove* state,
                                     [[maybe_unused]] Coord src, Coord dst)
    {
        board->set_king_position (who, dst);

        // set as not able to castle
        if (board->able_to_castle (
                who, CastlingEligible::KingsideIneligible | CastlingEligible::QueensideIneligible))
        {
            // set the new castle status
            board->remove_castling_eligibility (
                who, CastlingEligible::KingsideIneligible | CastlingEligible::QueensideIneligible);
        }
    }

    static void unapply_for_king_move (Board* board, Color who, Coord src,
                                       [[maybe_unused]] Coord dst)
    {
        board->set_king_position (who, src);
    }

    static void apply_for_rook_capture (Board* board, Color opponent, ColoredPiece dst_piece,
                                         Coord src, Coord dst)
    {
        assert (piece_color (dst_piece) == opponent && piece_type (dst_piece) == Piece::Rook);

        CastlingEligibility castle_state = CastlingEligible::EitherSideEligible;

        int castle_rook_row = opponent == Color::White ? Last_Row : First_Row;

        //
        // This needs to distinguish between captures that end
        // up on the rook and moves from the rook itself.
        //
        if (Column (dst) == First_Column && Row (dst) == castle_rook_row)
            castle_state = CastlingEligible::QueensideIneligible;
        else if (Column (dst) == Last_Column && Row (dst) == castle_rook_row)
            castle_state = CastlingEligible::KingsideIneligible;

        //
        // Set inability to castle on one side.
        //
        if (board->able_to_castle (opponent, castle_state))
            board->remove_castling_eligibility (opponent, castle_state);
    }

    static void apply_for_rook_move (Board* board, Color player, ColoredPiece src_piece,
                                     Move move, Coord src, Coord dst)
    {
        if (!(piece_color (src_piece) == player && piece_type (src_piece) == Piece::Rook))
        {
            throw MoveConsistencyProblem { "apply_for_rook_move failed: move "
                                           + to_string (move) };
        }

        assert (piece_color (src_piece) == player && piece_type (src_piece) == Piece::Rook);

        CastlingEligibility affects_castle_state = CastlingEligible::EitherSideEligible;
        int castle_src_row = player == Color::White ? Last_Row : First_Row;

        //
        // This needs to distinguish between captures that end
        // up on the rook and moves from the rook itself.
        //
        if (Row (src) == castle_src_row)
        {
            if (Column (src) == Queen_Rook_Column)
                affects_castle_state = CastlingEligible::QueensideIneligible;
            else if (Column (src) == King_Rook_Column)
                affects_castle_state = CastlingEligible::KingsideIneligible;
        }

        // Set inability to castle on one side.
        if (affects_castle_state != CastlingEligible::EitherSideEligible
            && board->able_to_castle (player, affects_castle_state))
        {
            board->remove_castling_eligibility (player, affects_castle_state);
        }
    }

    static void update_en_passant_eligibility (Board* board, Color who, ColoredPiece src_piece,
                                               Move move, UndoMove* undo_state)
    {
        ColorIndex c_index = color_index (who);
        ColorIndex o_index = color_index (color_invert (who));

        int direction = pawn_direction<int> (who);
        Coord new_state = No_En_Passant_Coord;
        if (is_double_square_pawn_move (src_piece, move))
        {
            Coord src = move.get_src ();
            int prev_row = next_row (Row<int> (src), direction);
            new_state = make_coord (prev_row, Column (src));
        }
        undo_state->en_passant_targets[o_index] = board->get_en_passant_target (o_index);
        undo_state->en_passant_targets[c_index] = board->get_en_passant_target (c_index);
        board->set_en_passant_target (c_index, new_state);
    }

    static void restore_en_passant_eligibility (Board* board, Color who, const UndoMove& undo_state)
    {
        ColorIndex o_index = color_index (color_invert (who));

        board->set_en_passant_target (o_index, undo_state.en_passant_targets[o_index]);
    }

    auto Board::make_move (Color who, Move move) -> UndoMove
    {
        assert (who == my_code.current_turn ());

        UndoMove undo_state = Empty_Undo_State;
        Color opponent = color_invert (who);

        Coord src = move.get_src ();
        Coord dst = move.get_dst ();

        auto src_piece = piece_at (src);
        auto orig_src_piece = src_piece;
        auto dst_piece = piece_at (dst);

        assert (piece_type (src_piece) != Piece::None);
        assert (piece_color (src_piece) == who);

        // Save the current castling state, regardless of whether it will be affected.
        save_current_castle_state (&undo_state, get_castling_eligibility (who));
        save_opponent_castle_state (&undo_state, get_castling_eligibility (opponent));

        // check for promotion
        if (move.is_promoting ())
        {
            src_piece = move.get_promoted_piece ();
            my_material.add (src_piece);
            my_material.remove (ColoredPiece::make (who, Piece::Pawn));
        }

        switch (move.get_move_category())
        {
            case MoveCategory::Default:
                assert (piece_type (dst_piece) == Piece::None);
                break;

            case MoveCategory::NormalCapturing:
                assert (move.is_normal_capturing ());
                assert (piece_color (src_piece) != piece_color (dst_piece));
                undo_state.category = MoveCategory::NormalCapturing;
                undo_state.taken_piece_type = piece_type (dst_piece);
                break;

            case MoveCategory::EnPassant:
                dst_piece = apply_for_en_passant (this, who, src, dst);
                undo_state.category = MoveCategory::EnPassant;
                break;

            case MoveCategory::Castling:
                apply_for_castling_move (this, who, move, src, dst);
                undo_state.category = MoveCategory::Castling;
                break;

            default:
                throw Error {
                    "Invalid move category: " + std::to_string (static_cast<int>(move.get_move_category()))
                };
        }

        update_en_passant_eligibility (this, who, src_piece, move, &undo_state);

        my_code.apply_move (*this, move);

        set_piece (src, Piece_And_Color_None);
        set_piece (dst, src_piece);

        // update king position
        if (piece_type (src_piece) == Piece::King)
            apply_for_king_move (this, who, &undo_state, src, dst);

        // update rook position -- for castling
        if (piece_type (orig_src_piece) == Piece::Rook)
        {
            apply_for_rook_move (this, who, orig_src_piece, move, src, dst);
        }

        ColoredPiece captured_piece = captured_material (undo_state, opponent);
        if (piece_type (captured_piece) != Piece::None)
        {
            // update material estimate
            my_material.remove (captured_piece);

            auto captured_piece_type = piece_type (captured_piece);

            // update castle state if somebody takes the rook
            if (captured_piece_type == Piece::Rook)
            {
                apply_for_rook_capture (this, color_invert (who), dst_piece, src, dst);
            }
        }

        my_position.apply_move (who, orig_src_piece, move, &undo_state);

        update_move_clock (who, piece_type (orig_src_piece), move, undo_state);
        set_current_turn (color_invert (who));
        return undo_state;
    }

    void Board::take_back (Color who, Move move, const UndoMove& undo_state)
    {
        assert (my_code.current_turn () == color_invert (who));

        Color opponent = color_invert (who);

        Coord src = move.get_src ();
        Coord dst = move.get_dst ();

        auto dst_piece_type = undo_state.taken_piece_type;
        auto dst_piece = Piece_And_Color_None;
        auto src_piece = piece_at (dst);
        auto orig_src_piece = src_piece;

        // Restore the current castling state, regardless of whether it will be affected.
        set_castle_state (who, current_castle_state (undo_state));
        set_castle_state (opponent, opponent_castle_state (undo_state));

        assert (piece_type (src_piece) != Piece::None);
        assert (piece_color (src_piece) == who);
        if (dst_piece_type != Piece::None)
            dst_piece = ColoredPiece::make (opponent, dst_piece_type);

        // check for promotion
        if (move.is_promoting ())
        {
            src_piece = ColoredPiece::make (piece_color (src_piece), Piece::Pawn);
            my_material.remove (orig_src_piece);
            my_material.add (src_piece);
        }

        switch (move.get_move_category())
        {
            case MoveCategory::Default:
                assert (dst_piece_type == Piece::None);
                break;

            case MoveCategory::NormalCapturing:
                break;

            case MoveCategory::EnPassant:
                unapply_for_en_passant (this, who, src, dst);
                // restore empty square where piece was replaced:
                dst_piece = Piece_And_Color_None;
                break;

            case MoveCategory::Castling:
                unapply_for_castling (this, who, move, src, dst);
                break;

            default:
                throw Error {
                    "Invalid move category: " + std::to_string (static_cast<int>(move.get_move_category()))
                };
        }

        // Update en passant eligibility:
        restore_en_passant_eligibility (this, who, undo_state);

        // Update the code:
        my_code.unapply_move (*this, move, undo_state);

        // put the pieces back
        set_piece (dst, dst_piece);
        set_piece (src, src_piece);

        // update king position
        if (piece_type (src_piece) == Piece::King)
            unapply_for_king_move (this, who, src, dst);

        ColoredPiece captured_piece = captured_material (undo_state, opponent);
        auto captured_piece_type = piece_type (captured_piece);
        if (captured_piece_type != Piece::None)
        {
            // NOTE: we reload from the move in case of en-passant, since dst_piece
            // could be none.
            my_material.add (captured_piece);
        }

        my_position.unapply_move (who, undo_state);
        set_current_turn (who);
        restore_move_clock (undo_state);
    }

    static auto castle_parse (const string& str, Color who) -> optional<Move>
    {
        int src_row, dst_col;

        if (who == Color::White)
            src_row = Last_Row;
        else if (who == Color::Black)
            src_row = First_Row;
        else
            throw ParseMoveException { "Invalid color parsing castling move." };

        string transformed { str };
        std::transform (transformed.begin (), transformed.end (), transformed.begin (),
                        [] (auto c) -> auto
                        {
                            return ::toupper (c);
                        });

        if (transformed == "O-O-O")
            dst_col = King_Column - 2;
        else if (transformed == "O-O")
            dst_col = King_Column + 2;
        else
            return nullopt;

        return Move::make_castling (src_row, King_Column, src_row, dst_col);
    }

    auto move_parse_optional (const string& str, Color who) -> optional<Move>
    {
        bool en_passant = false;
        bool is_capturing = false;

        // allow any number of spaces/tabs before the two coordinates
        string tmp { str };

        if (tmp.empty ())
            return nullopt;

        tmp.erase (std::remove_if (tmp.begin (), tmp.end (), isspace), tmp.end ());
        std::transform (tmp.begin (), tmp.end (), tmp.begin (),
                        [] (auto c) -> auto
                        {
                            return ::toupper (c);
                        });

        if (tmp.empty ())
            return nullopt;

        if (tolower (tmp[0]) == 'o')
            return castle_parse (tmp, who);

        if (tmp.size () < 4)
            return nullopt;

        optional<Coord> src;
        int offset = 0;
        try
        {
            src = coord_parse (tmp.substr (0, 2));
            offset += 2;
        }
        catch ([[maybe_unused]] const CoordParseError& e)
        {
            return nullopt;
        }

        // allow an 'x' between coordinates, which is used to indicate a capture
        if (tmp[offset] == 'X')
        {
            offset++;
            is_capturing = true;
        }

        string dst_coord { tmp.substr (offset, 2) };
        offset += 2;
        if (dst_coord.empty ())
            return nullopt;

        optional<Coord> dst;
        try
        {
            dst = coord_parse (dst_coord);
        }
        catch ([[maybe_unused]] const CoordParseError& e)
        {
            return nullopt;
        }

        string rest { tmp.substr (offset) };
        Move move = Move::make (*src, *dst);
        if (is_capturing)
        {
            move = move.with_capture ();
        }

        // grab extra identifiers describing the move
        ColoredPiece promoted = ColoredPiece::make (Color::None, Piece::None);
        if (rest == "EP")
        {
            en_passant = true;
        }
        else if (rest == "(Q)")
        {
            promoted = ColoredPiece::make (who, Piece::Queen);
        }
        else if (rest == "(N)")
        {
            promoted = ColoredPiece::make (who, Piece::Knight);
        }
        else if (rest == "(B)")
        {
            promoted = ColoredPiece::make (who, Piece::Bishop);
        }
        else if (rest == "(R)")
        {
            promoted = ColoredPiece::make (who, Piece::Rook);
        }

        if (piece_type (promoted) != Piece::None)
        {
            move = move.with_promotion (promoted);
        }

        if (en_passant)
        {
            move = Move::make_en_passant (Row (*src), Column (*src), Row (*dst), Column (*dst));
        }

        return move;
    }

    auto move_parse (const string& str, Color color) -> Move
    {
        if (tolower (str[0]) == 'o' && color == Color::None)
            throw ParseMoveException ("Move requires color, but no color provided");

        auto optional_result = move_parse_optional (str, color);
        if (!optional_result.has_value ())
            throw ParseMoveException ("Error parsing move: " + str);

        auto result = *optional_result;
        auto move_category = result.get_move_category ();
        if (color == Color::None && move_category != MoveCategory::NormalCapturing
            && move_category != MoveCategory::Default)
        {
            throw ParseMoveException ("Invalid type of move in parse_simple_move");
        }

        return result;
    }

    string to_string (const Move& move)
    {
        Coord src = move.get_src ();
        Coord dst = move.get_dst ();

        if (move.is_castling ())
        {
            if (Column (dst) - Column (src) < 0)
            {
                // queenside
                return "O-O-O";
            }
            else
            {
                // kingside
                return "O-O";
            }
        }

        string result;
        result += to_string (src);

        if (move.is_normal_capturing ())
            result += "x";
        else
            result += " ";

        result += to_string (dst);

        if (move.is_en_passant ())
        {
            result += " ep";
        }

        if (move.is_promoting ())
        {
            string promoted_piece { piece_char (move.get_promoted_piece ()) };
            result += "(" + promoted_piece + ")";
        }

        return result;
    }

    auto map_coordinates_to_move (const Board& board, Color who, Coord src, Coord dst,
                                  optional<Piece> promoted_piece) -> optional<Move>
    {
        ColoredPiece src_piece = board.piece_at (src);
        ColoredPiece dst_piece = board.piece_at (dst);

        if (src_piece == Piece_And_Color_None)
            return {};

        if (piece_color (src_piece) != who)
            return {};

        // make capturing if dst piece is not none
        Move move = Move::make (src, dst);
        if (dst_piece != Piece_And_Color_None)
            move = move.with_capture ();

        // check for pawn special moves.
        switch (piece_type (src_piece))
        {
            case Piece::Pawn:
                // look for en passant:
                if (piece_type (src_piece) == Piece::Pawn)
                {
                    int eligible_column
                        = eligible_en_passant_column (board, Row (src), Column (src), who);
                    if (eligible_column == Column (dst))
                        return Move::make_en_passant (src, dst);

                    if (need_pawn_promotion (Row<int> (dst), who) && promoted_piece.has_value ())
                        return move.with_promotion (ColoredPiece::make (who, *promoted_piece));
                }
                break;

            // look for castling
            case Piece::King:
                if (Column (src) == King_Column)
                {
                    if (Column (dst) - Column (src) == 2 || Column (dst) - Column (src) == -2)
                    {
                        return Move::make_castling (src, dst);
                    }
                }
                break;

            default:
                break;
        }

        return move;
    }

    auto operator<< (std::ostream& os, const Move& value) -> std::ostream&
    {
        os << to_string (value);
        return os;
    }
}
