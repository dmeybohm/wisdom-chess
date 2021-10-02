#include "move.hpp"
#include "board.hpp"
#include "validate.hpp"

#include <iostream>

namespace wisdom
{
    using std::optional;
    using std::nullopt;

    Coord en_passant_taken_pawn_coord (Coord src, Coord dst)
    {
        return make_coord (Row (src), Column (dst));
    }

    static void save_current_castle_state (UndoMove &undo_state, CastlingState state)
    {
        undo_state.current_castle_state = pack_castle_state (state);
    }

    static void save_opponent_castle_state (UndoMove &undo_state, CastlingState state)
    {
        undo_state.opponent_castle_state = pack_castle_state (state);
    }

    constexpr bool move_affects_current_castle_state (UndoMove move)
    {
        return move.current_castle_state != Castle_None;
    }

    constexpr bool move_affects_opponent_castle_state (UndoMove move)
    {
        return move.opponent_castle_state != Castle_None;
    }

    constexpr bool is_double_square_pawn_move (ColoredPiece src_piece, Move move)
    {
        Coord src = move_src (move);
        Coord dst = move_dst (move);
        return piece_type (src_piece) == Piece::Pawn && abs (Row (src) - Row (dst)) == 2;
    }

    ColoredPiece handle_en_passant (Board &board, Color who, Coord src, Coord dst, int undo)
    {
        Coord taken_pawn_pos = en_passant_taken_pawn_coord (src, dst);

        if (undo)
        {
            ColoredPiece taken_pawn = make_piece (color_invert (who), Piece::Pawn);
            board.set_piece (taken_pawn_pos, taken_pawn);

            return Piece_And_Color_None; // restore empty square where piece was replaced
        }
        else
        {
            ColoredPiece taken = piece_at (board, taken_pawn_pos);

            assert(piece_type (taken) == Piece::Pawn);
            assert(piece_color (taken) != who);
            board.set_piece (taken_pawn_pos, Piece_And_Color_None);

            return taken;
        }
    }

    static Move get_castling_rook_move (Board &board, Move move, Color who)
    {
        int src_row, src_col;
        int dst_row, dst_col;
        Coord src, dst;

        assert (is_castling_move (move));

        src = move_src (move);
        dst = move_dst (move);

        src_row = Row (src);
        dst_row = Row (dst);

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

        if (!((piece_type (piece_at (board, src_row, src_col)) == Piece::Rook
               || piece_type (piece_at (board, dst_row, dst_col)) == Piece::Rook)))
        {
            throw MoveConsistencyProblem {
                    "move considering: " + to_string (move) + "(" + to_string (who) + " to move)"
            };
        }

        return make_noncapture_move (src_row, src_col, dst_row, dst_col);
    }

    static void handle_castling (Board &board, Color who,
                                 Move king_move,
                                 [[maybe_unused]] Coord src,
                                 [[maybe_unused]] Coord dst,
                                 int undo)
    {
        Move rook_move = get_castling_rook_move (board, king_move, who);

        if (undo)
            assert (piece_type (piece_at (board, dst)) == Piece::King);
        else
            assert (piece_type (piece_at (board, src)) == Piece::King);

        assert (abs (Column (src) - Column (dst)) == 2);

        auto rook_src = move_src (rook_move);
        auto rook_dst = move_dst (rook_move);

        auto empty_piece = make_piece (Color::None, Piece::None);

        if (undo)
        {
            // undo the rook move
            auto rook = piece_at (board, rook_dst);

            // undo the rook move
            board.set_piece (rook_dst, empty_piece);
            board.set_piece (rook_src, rook);
        }
        else
        {
            auto rook = piece_at (board, rook_src);

            // do the rook move
            board.set_piece (rook_dst, rook);
            board.set_piece (rook_src, empty_piece);
        }
    }

    void update_king_position (Board &board, Color who, Move move,
                               UndoMove &undo_state, Coord src, Coord dst,
                               int undo)
    {
        if (undo)
        {
            UndoMove undo_state_value = undo_state;

            king_position_set (board, who, src);

            // retrieve the old board castle status
            if (move_affects_current_castle_state (undo_state_value))
            {
                board_undo_castle_change (board, who, current_castle_state (undo_state_value));
            }
        }
        else
        {
            king_position_set (board, who, dst);

            // set as not able to castle
            if (able_to_castle (board, who, (Castle_Kingside | Castle_Queenside)))
            {
                // save the old castle status
                CastlingState old_castle_state = board_get_castle_state (board, who);
                save_current_castle_state (undo_state, old_castle_state);

                if (is_castling_move (move))
                    old_castle_state = Castle_Castled;
                else
                    old_castle_state |= Castle_Kingside | Castle_Queenside;

                // set the new castle status
                board_apply_castle_change (board, who, old_castle_state);
            }
        }
    }

    static void
    update_opponent_rook_position (Board &board,
                                   Color opponent,
                                   [[maybe_unused]] ColoredPiece dst_piece,
                                   UndoMove &undo_state,
                                   [[maybe_unused]] Coord src,
                                   Coord dst,
                                   int undo)
    {
        assert (piece_color (dst_piece) == opponent && piece_type (dst_piece) == Piece::Rook);

        if (undo)
        {
            UndoMove undo_state_value = undo_state;

            // need to put castle status back...it's saved in the move
            // from do_move()...
            if (move_affects_opponent_castle_state (undo_state_value))
                board_undo_castle_change (board, opponent, opponent_castle_state (undo_state_value));
        }
        else
        {
            CastlingState castle_state;

            //
            // This needs distinguishes between captures that end
            // up on the rook and moves from the rook itself.
            //
            if (Column (dst) == 0)
                castle_state = Castle_Queenside;
            else
                castle_state = Castle_Kingside;

            //
            // Set inability to castle on one side. Note that
            // Castle_Queenside/KINGSIDE are _negative_ flags, indicating the
            // my_computer_player cannot castle.  This is a bit confusing, not sure why I did
            // this.
            //
            if (able_to_castle (board, opponent, castle_state))
            {
                // save the current castle state
                CastlingState orig_castle_state = board_get_castle_state (board, opponent);
                save_opponent_castle_state (undo_state, orig_castle_state);

                castle_state |= orig_castle_state;
                board_apply_castle_change (board, opponent, castle_state);
            }
        }
    }

    static void update_current_rook_position (Board &board, Color player,
                                              ColoredPiece src_piece, Move move,
                                              UndoMove &undo_state,
                                              [[maybe_unused]] Coord src, [[maybe_unused]] Coord dst,
                                              int undo)
    {
        if (!(piece_color (src_piece) == player &&
              piece_type (src_piece) == Piece::Rook))
        {
            throw MoveConsistencyProblem {
                "update_current_rook_position failed: move " + to_string (move)
            };
        }

        assert (piece_color (src_piece) == player && piece_type (src_piece) == Piece::Rook);

        if (undo)
        {
            UndoMove undo_state_value = undo_state;
            // need to put castle status back...it's saved in the move
            // from do_move()...
            if (move_affects_current_castle_state (undo_state_value))
                board_undo_castle_change (board, player, current_castle_state (undo_state_value));
        }
        else
        {
            CastlingState affects_castle_state = 0;
            int castle_src_row = player == Color::White ? Last_Row : First_Row;

            //
            // This needs distinguishes between captures that end
            // up on the rook and moves from the rook itself.
            //
            if (Row (src) == castle_src_row)
            {
                if (Column (src) == Queen_Rook_Column)
                    affects_castle_state = Castle_Queenside;
                else if (Column (src) == King_Rook_Column)
                    affects_castle_state = Castle_Kingside;
            }

            //
            // Set inability to castle on one side. Note that
            // Castle_Queenside/KINGSIDE are _negative_ flags, indicating the
            // Computer player cannot castle.  This is a bit confusing, not sure why I did
            // this.
            //
            if (affects_castle_state > 0 && able_to_castle (board, player, affects_castle_state))
            {
                // save the current castle state
                CastlingState orig_castle_state = board_get_castle_state (board, player);
                save_current_castle_state (undo_state, orig_castle_state);

                affects_castle_state |= orig_castle_state;
                board_apply_castle_change (board, player, affects_castle_state);
            }
        }
    }

    static void handle_en_passant_eligibility (Board &board, Color who, ColoredPiece src_piece,
                                               Move move, UndoMove *undo_state, int undo)
    {
        ColorIndex c_index = color_index (who);
        ColorIndex o_index = color_index (color_invert (who));

        if (undo)
        {
            board.en_passant_target[c_index] = undo_state->en_passant_target[c_index];
            board.en_passant_target[o_index] = undo_state->en_passant_target[o_index];
        }
        else
        {
            int direction = pawn_direction (who);
            Coord new_state = No_En_Passant_Coord;
            if (is_double_square_pawn_move (src_piece, move))
            {
                Coord src = move_src (move);
                int prev_row = next_row (Row (src), direction);
                new_state = make_coord (prev_row, Column (src));
            }
            undo_state->en_passant_target[c_index] = board.en_passant_target[c_index];
            undo_state->en_passant_target[o_index] = board.en_passant_target[o_index];
            board.en_passant_target[c_index] = new_state;
            board.en_passant_target[o_index] = No_En_Passant_Coord;
        }
    }

    UndoMove Board::make_move (Color who, Move move)
    {
        UndoMove undo_state = Empty_Undo_State;
        Color opponent = color_invert (who);

        Coord src = move_src (move);
        Coord dst = move_dst (move);

        auto src_piece = this->piece_at (src);
        auto orig_src_piece = src_piece;
        auto dst_piece = this->piece_at (dst);

        assert (piece_type (src_piece) != Piece::None);
        if (piece_type (dst_piece) != Piece::None)
        {
            assert (is_normal_capture_move (move));
            undo_state.category = MoveCategory::NormalCapture;
            undo_state.taken_piece_type = piece_type (dst_piece);
        }

        if (piece_type (src_piece) != Piece::None &&
            piece_type (dst_piece) != Piece::None)
        {
            assert (piece_color (src_piece) != piece_color (dst_piece));
        }

        // check for promotion
        if (is_promoting_move (move))
        {
            src_piece = move_get_promoted_piece (move);
            this->material.add (src_piece);
            this->material.remove (make_piece (who, Piece::Pawn));
        }

        // check for en passant
        if (is_en_passant_move (move))
        {
            dst_piece = handle_en_passant (*this, who, src, dst, 0);
            undo_state.category = MoveCategory::EnPassant;
        }

        // check for castling
        if (is_castling_move (move))
        {
            handle_castling (*this, who, move, src, dst, 0);
            undo_state.category = MoveCategory::Castling;
        }

        handle_en_passant_eligibility (*this, who, src_piece, move, &undo_state, 0);

        this->code.apply_move (*this, move);

        this->set_piece (src, Piece_And_Color_None);
        this->set_piece (dst, src_piece);

        // update king position
        if (piece_type (src_piece) == Piece::King)
            update_king_position (*this, who, move, undo_state, src, dst, 0);

        // update rook position -- for castling
        if (piece_type (orig_src_piece) == Piece::Rook)
        {
            update_current_rook_position (*this, who, orig_src_piece,
                                          move, undo_state, src, dst, 0);
        }

        ColoredPiece captured_piece = captured_material (undo_state, opponent);
        if (piece_type (captured_piece) != Piece::None)
        {
            // update material estimate
            this->material.remove (captured_piece);

            // update castle state if somebody takes the rook
            if (piece_type (captured_piece) == Piece::Rook)
            {
                update_opponent_rook_position (*this, color_invert (who), dst_piece,
                                               undo_state, src, dst, 0);
            }
        }

        this->position.apply_move (who, orig_src_piece, move, undo_state);
        validate_castle_state (*this, move);

        this->update_move_clock (who, piece_type (orig_src_piece), move, undo_state);
        return undo_state;
    }

    void Board::take_back (Color who, Move move, UndoMove undo_state)
    {
        Color opponent = color_invert (who);

        Coord src = move_src (move);
        Coord dst = move_dst (move);

        auto dst_piece_type = undo_state.taken_piece_type;
        auto dst_piece = Piece_And_Color_None;
        auto src_piece = this->piece_at (dst);
        auto orig_src_piece = src_piece;

        assert (piece_type (src_piece) != Piece::None);
        assert (piece_color (src_piece) == who);
        if (dst_piece_type != Piece::None)
            dst_piece = make_piece (opponent, dst_piece_type);

        // check for promotion
        if (is_promoting_move (move))
        {
            src_piece = make_piece (piece_color (src_piece), Piece::Pawn);
            this->material.remove (orig_src_piece);
            this->material.add (src_piece);
        }

        // check for castling
        if (is_castling_move (move))
            handle_castling (*this, who, move, src, dst, 1);

        // Update en passant eligibility:
        handle_en_passant_eligibility (*this, who, src_piece, move, &undo_state, 1);

        // check for en passant
        if (is_en_passant_move (move))
            dst_piece = handle_en_passant (*this, who, src, dst, 1);

        // Update the code:
        this->code.unapply_move (*this, move, undo_state);

        // put the pieces back
        this->set_piece (dst, dst_piece);
        this->set_piece (src, src_piece);

        // update king position
        if (piece_type (src_piece) == Piece::King)
            update_king_position (*this, who, move, undo_state, src, dst, 1);

        if (piece_type (orig_src_piece) == Piece::Rook)
        {
            update_current_rook_position (*this, who, orig_src_piece, move, undo_state,
                                          src, dst, 1);
        }

        ColoredPiece captured_piece = captured_material (undo_state, opponent);
        if (piece_type (captured_piece) != Piece::None)
        {
            // NOTE: we reload from the move in case of en-passant, since dst_piece
            // could be none.
            this->material.add (captured_piece);

            if (piece_type (dst_piece) == Piece::Rook)
            {
                update_opponent_rook_position (*this, color_invert (who), dst_piece,
                                               undo_state, src, dst, 1);
            }
        }

        this->position.unapply_move (who, src_piece, move, undo_state);
        validate_castle_state (*this, move);
        this->restore_move_clock (undo_state);
    }

    static optional<Move> castle_parse (const std::string &str, Color who)
    {
        int src_row, dst_col;

        if (who == Color::White)
            src_row = Last_Row;
        else if (who == Color::Black)
            src_row = First_Row;
        else
            throw ParseMoveException { "Invalid color parsing castling move." };

        std::string transformed { str };
        std::transform (transformed.begin (), transformed.end (), transformed.begin (),
                        [] (auto c) -> auto
                        { return ::toupper (c); });

        if (transformed == "O-O-O")
            dst_col = King_Column - 2;
        else if (transformed == "O-O")
            dst_col = King_Column + 2;
        else
            return nullopt;

        return make_castling_move (src_row, King_Column, src_row, dst_col);
    }

    optional<Move> move_parse_optional (const std::string &str, Color who)
    {
        bool en_passant = false;
        bool is_capturing = false;

        // allow any number of spaces/tabs before the two coordinates
        std::string tmp { str };

        if (tmp.empty ())
            return nullopt;

        tmp.erase (std::remove_if (tmp.begin (), tmp.end (), isspace), tmp.end ());
        std::transform (tmp.begin (), tmp.end (), tmp.begin (),
                        [] (auto c) -> auto
                        { return ::toupper (c); });

        if (tmp.empty ())
            return nullopt;

        if (tolower (tmp[0]) == 'o')
            return castle_parse (tmp, who);

        if (tmp.size () < 4)
            return nullopt;

        Coord src;
        int offset = 0;
        try
        {
            src = coord_parse (tmp.substr (0, 2));
            offset += 2;
        }
        catch ([[maybe_unused]] const CoordParseError &e)
        {
            return nullopt;
        }

        // allow an 'x' between coordinates, which is used to indicate a capture
        if (tmp[offset] == 'X')
        {
            offset++;
            is_capturing = true;
        }

        std::string dst_coord { tmp.substr (offset, 2) };
        offset += 2;
        if (dst_coord.empty ())
            return nullopt;

        Coord dst;
        try
        {
            dst = coord_parse (dst_coord);
        }
        catch ([[maybe_unused]] const CoordParseError &e)
        {
            return nullopt;
        }

        std::string rest { tmp.substr (offset) };
        Move move = make_noncapture_move (src, dst);
        if (is_capturing)
        {
            move = copy_move_with_capture (move);
        }

        // grab extra identifiers describing the move
        ColoredPiece promoted = make_piece (Color::None, Piece::None);
        if (rest == "EP")
        {
            en_passant = true;
        }
        else if (rest == "(Q)")
        {
            promoted = make_piece (who, Piece::Queen);
        }
        else if (rest == "(N)")
        {
            promoted = make_piece (who, Piece::Knight);
        }
        else if (rest == "(B)")
        {
            promoted = make_piece (who, Piece::Bishop);
        }
        else if (rest == "(R)")
        {
            promoted = make_piece (who, Piece::Rook);
        }

        if (piece_type (promoted) != Piece::None)
        {
            move = copy_move_with_promotion (move, promoted);
        }

        if (en_passant)
        {
            move = make_en_passant_move (Row (src), Column (src), Row (dst), Column (dst));
        }

        return move;
    }

    Move move_parse (const std::string &str, Color color)
    {
        if (tolower (str[0]) == 'o' && color == Color::None)
            throw ParseMoveException ("Move requires color, but no color provided");

        auto optional_result = move_parse_optional (str, color);
        if (!optional_result.has_value ())
            throw ParseMoveException ("Error parsing move: " + str);

        auto result = *optional_result;
        if (color == Color::None &&
            result.move_category != MoveCategory::NormalCapture &&
            result.move_category != MoveCategory::NonCapture)
        {
            throw ParseMoveException ("Invalid type of move in parse_simple_move");
        }

        return result;
    }

    std::string to_string (const Move &move)
    {
        Coord src, dst;

        src = move_src (move);
        dst = move_dst (move);

        if (is_castling_move (move))
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

        std::string result;
        result += to_string (src);

        if (is_normal_capture_move (move))
            result += "x";
        else
            result += " ";

        result += to_string (dst);

        if (is_en_passant_move (move))
        {
            result += " ep";
        }

        if (is_promoting_move (move))
        {
            std::string promoted_piece { piece_char (move_get_promoted_piece (move)) };
            result += "(" + promoted_piece + ")";
        }

        return result;
    }

    std::ostream &operator<< (std::ostream &os, const Move &value)
    {
        os << to_string (value);
        return os;
    }
}
