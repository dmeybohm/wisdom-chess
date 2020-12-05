#include <iostream>
#include <algorithm>
#include <sstream>

#include "move.h"
#include "coord.h"
#include "board.h"
#include "validate.h"
#include "generate.h"

coord_t en_passant_taken_pawn_coord (coord_t src, coord_t dst)
{
    return make_coord (ROW (src), COLUMN (dst));
}

piece_t handle_en_passant (struct board &board, Color who,
                           coord_t src, coord_t dst, int undo)
{
    coord_t taken_pawn_pos = en_passant_taken_pawn_coord (src, dst);

    if (undo)
    {
        piece_t taken_pawn = make_piece (color_invert (who), Piece::Pawn);
        board_set_piece (board, taken_pawn_pos, taken_pawn);

        return piece_and_color_none; // restore empty square where piece was replaced
    }
    else
    {
        piece_t taken = piece_at (board, taken_pawn_pos);

        assert(piece_type (taken) == Piece::Pawn );
        assert(piece_color (taken) != who );
        board_set_piece (board, taken_pawn_pos, piece_and_color_none);

        return taken;
    }
}

static move_t get_castling_rook_move (struct board &board, move_t move,
                                      Color who)
{
    int8_t    src_row, src_col;
    int8_t    dst_row, dst_col;
    coord_t    src, dst;

    assert (is_castling_move (move));

    src = move_src (move);
    dst = move_dst (move);

    src_row = ROW(src);
    dst_row = ROW(dst);

    if (COLUMN(src) < COLUMN(dst))
    {
        // castle to the right (kingside)
        src_col = LAST_COLUMN;
        dst_col = COLUMN(dst) - 1;
    }
    else
    {
        // castle to the left (queenside)
        src_col = 0;
        dst_col = COLUMN (dst) + 1;
    }

    if (!((piece_type (piece_at (board, src_row, src_col)) == Piece::Rook
           || piece_type (piece_at (board, dst_row, dst_col)) == Piece::Rook)))
    {
        std::stringstream output;
        output << "move considering: " << to_string(move) << "(" << to_string(who) << " to move)\n";
        std::cerr << output.str();
        board.dump();
        assert (0);
    }

    return make_move (src_row, src_col, dst_row, dst_col);
}

static void handle_castling (struct board &board, Color who,
                             move_t king_move, coord_t src, coord_t dst, int undo)
{
    move_t  rook_move;
    coord_t rook_src, rook_dst;
    piece_t rook, empty_piece;

    rook_move = get_castling_rook_move (board, king_move, who);

    if (undo)
        assert (piece_type (piece_at (board, dst)) == Piece::King);
    else
        assert (piece_type (piece_at (board, src)) == Piece::King);

    assert (abs(COLUMN(src) - COLUMN(dst)) == 2);

    rook_src = move_src (rook_move);
    rook_dst = move_dst (rook_move);

    empty_piece = make_piece (Color::None, Piece::None);

    if (undo)
    {
        // undo the rook move
        rook = piece_at (board, rook_dst);

        // undo the rook move
        board_set_piece (board, rook_dst, empty_piece);
        board_set_piece (board, rook_src, rook);
    }
    else
    {
        rook = piece_at (board, rook_src);

        /* do the rook move */
        board_set_piece (board, rook_dst, rook);
        board_set_piece (board, rook_src, empty_piece);
    }
}

void update_king_position (struct board &board, Color who, move_t move,
                           undo_move_t *undo_state, coord_t src, coord_t dst,
                           int undo)
{
    if (undo)
    {
        undo_move_t undo_state_value = *undo_state;

        king_position_set (board, who, src);

        // retrieve the old board castle status
        if (move_affects_current_castle_state(undo_state_value)) {
            board_undo_castle_change (board, who, current_castle_state(undo_state_value));
        }
    }
    else
    {
        king_position_set (board, who, dst);

        // set as not able to castle
        if (able_to_castle (board, who, (CASTLE_KINGSIDE | CASTLE_QUEENSIDE)))
        {
            // save the old castle status
            castle_state_t old_castle_state = board_get_castle_state (board, who);
            save_current_castle_state (undo_state, old_castle_state);

            if (!is_castling_move(move))
                old_castle_state |= CASTLE_KINGSIDE | CASTLE_QUEENSIDE;
            else
                old_castle_state = CASTLE_CASTLED;

            // set the new castle status
            board_apply_castle_change (board, who, old_castle_state);
        }
    }
}

static void
update_opponent_rook_position (struct board &board, Color opponent,
                               piece_t dst_piece, undo_move_t *undo_state,
                               coord_t src, coord_t dst, int undo)
{
    assert (piece_color(dst_piece) == opponent && piece_type(dst_piece) == Piece::Rook);

    if (undo)
    {
        undo_move_t undo_state_value = *undo_state;

        // need to put castle status back...its saved in the move
        // from do_move()...
        if (move_affects_opponent_castle_state(undo_state_value))
            board_undo_castle_change (board, opponent, opponent_castle_state(undo_state_value));
    }
    else
    {
        castle_state_t castle_state;

        //
        // This needs distinguishes between captures that end
        // up on the rook and moves from the rook itself.
        //
        if (COLUMN(dst) == 0)
            castle_state = CASTLE_QUEENSIDE;
        else
            castle_state = CASTLE_KINGSIDE;

        //
        // Set inability to castle on one side. Note that
        // CASTLE_QUEENSIDE/KINGSIDE are _negative_ flags, indicating the
        // player cannot castle.  This is a bit confusing, not sure why i did
        // this.
        //
        if (able_to_castle (board, opponent, castle_state))
        {
            // save the current castle state
            castle_state_t orig_castle_state = board_get_castle_state (board, opponent);
            save_opponent_castle_state (undo_state, orig_castle_state);

            castle_state |= orig_castle_state;
            board_apply_castle_change (board, opponent, castle_state);
        }
    }
}

static void update_current_rook_position (struct board &board, Color player,
                                          piece_t src_piece, move_t move,
                                          undo_move_t *undo_state,
                                          coord_t src, coord_t dst, int undo)
{
    if (!(piece_color(src_piece) == player &&
            piece_type(src_piece) == Piece::Rook))
    {
        std::cout << "update_current_rook_position failed: move " << to_string(move) << "\n";
        board.dump();
        abort ();
    }

    assert (piece_color(src_piece) == player && piece_type(src_piece) == Piece::Rook );

    if (undo)
    {
        undo_move_t undo_state_value = *undo_state;
        // need to put castle status back...its saved in the move
        // from do_move()...
        if (move_affects_current_castle_state (undo_state_value))
            board_undo_castle_change (board, player, current_castle_state(undo_state_value));
    }
    else
    {
        castle_state_t castle_state;

        //
        // This needs distinguishes between captures that end
        // up on the rook and moves from the rook itself.
        //
        if (COLUMN(src) == 0)
            castle_state = CASTLE_QUEENSIDE;
        else
            castle_state = CASTLE_KINGSIDE;

        //
        // Set inability to castle on one side. Note that
        // CASTLE_QUEENSIDE/KINGSIDE are _negative_ flags, indicating the
        // player cannot castle.  This is a bit confusing, not sure why i did
        // this.
        //
        if (able_to_castle (board, player, castle_state))
        {
            // save the current castle state
            castle_state_t orig_castle_state = board_get_castle_state (board, player);
            save_current_castle_state (undo_state, orig_castle_state);

            castle_state |= orig_castle_state;
            board_apply_castle_change (board, player, castle_state);
        }
    }
}

static void handle_en_passant_eligibility (struct board &board, Color who, piece_t src_piece,
                                           move_t move, undo_move_t *undo_state, int undo)
{
    color_index_t c_index = color_index(who);
    color_index_t o_index = color_index(color_invert(who));

    if (undo)
    {
        board.en_passant_target[c_index] = undo_state->en_passant_target[c_index];
        board.en_passant_target[o_index] = undo_state->en_passant_target[o_index];
    }
    else
    {
        int direction = pawn_direction (who);
        coord_t new_state = no_en_passant_coord;
        if (is_double_square_pawn_move (src_piece, move))
        {
            coord_t src = move_src (move);
            int8_t prev_row = next_row (ROW(src), direction);
            new_state = make_coord (prev_row, COLUMN(src));
        }
        undo_state->en_passant_target[c_index] = board.en_passant_target[c_index];
        undo_state->en_passant_target[o_index] = board.en_passant_target[o_index];
        board.en_passant_target[c_index] = new_state;
        board.en_passant_target[o_index] = no_en_passant_coord;
    }
}

undo_move_t do_move (struct board &board, Color who, move_t move)
{
    piece_t      orig_src_piece, src_piece, dst_piece;
    coord_t      src, dst;
    undo_move_t  undo_state = empty_undo_state;
    Color        opponent = color_invert(who);

    src = move_src (move);
    dst = move_dst (move);

    orig_src_piece = src_piece = piece_at (board, src);
    dst_piece = piece_at (board, dst);

    if (piece_type(dst_piece) != Piece::None)
    {
        assert( is_capture_move(move) );
        undo_state.category = MoveCategory::NormalCapture;
        undo_state.taken_piece_type = piece_type (dst_piece);
    }

    if (piece_type(src_piece) != Piece::None &&
            piece_type(dst_piece) != Piece::None)
    {
        assert (piece_color (src_piece) != piece_color (dst_piece));
    }

    // check for promotion
	if (is_promoting_move(move))
	{
		src_piece = move_get_promoted_piece(move);
		board.material.add (src_piece);
		board.material.remove (make_piece (who, Piece::Pawn));
	}

    // check for en passant
    if (is_en_passant_move(move))
    {
        dst_piece = handle_en_passant (board, who, src, dst, 0);
        undo_state.category = MoveCategory::EnPassant;
    }

    // check for castling
    if (is_castling_move(move))
    {
        handle_castling (board, who, move, src, dst, 0);
        undo_state.category = MoveCategory::Castling;
    }

    handle_en_passant_eligibility (board, who, src_piece, move, &undo_state, 0);

    board_set_piece (board, src, piece_and_color_none);
    board_set_piece (board, dst, src_piece);

    // update king position
    if (piece_type (src_piece) == Piece::King)
        update_king_position (board, who, move, &undo_state, src, dst, 0);

    // update rook position -- for castling
    if (piece_type (orig_src_piece) == Piece::Rook)
    {
        update_current_rook_position (board, who, orig_src_piece,
                                      move, &undo_state, src, dst, 0);
    }

    piece_t captured_piece = captured_material (undo_state, opponent);
    if (piece_type (captured_piece) != Piece::None)
    {
        // update material estimate
        board.material.remove (captured_piece);

        // update castle state if somebody takes the rook
        if (piece_type (captured_piece) == Piece::Rook)
        {
            update_opponent_rook_position (board, color_invert (who), dst_piece,
                                           &undo_state, src, dst, 0);
        }
    }

    position_do_move (&board.position, who, orig_src_piece, move, undo_state);
    validate_castle_state (board, move);

    return undo_state;
}

void undo_move (struct board &board, Color who,
                move_t move, undo_move_t undo_state)
{
    piece_t    orig_src_piece, src_piece, dst_piece = piece_and_color_none;
    Piece      dst_piece_type;
    coord_t    src, dst;
    Color      opponent = color_invert(who);

    src = move_src (move);
    dst = move_dst (move);

    dst_piece_type = undo_state.taken_piece_type;
    orig_src_piece = src_piece = piece_at (board, dst);

    assert (piece_type(src_piece) != Piece::None );
    assert (piece_color(src_piece) == who );
    if (dst_piece_type != Piece::None)
        dst_piece = make_piece (opponent, dst_piece_type);

    // check for promotion
	if (is_promoting_move(move))
	{
		src_piece = make_piece (piece_color (src_piece), Piece::Pawn);
		board.material.remove (orig_src_piece);
		board.material.add (src_piece);
	}

    // check for castling
    if (is_castling_move(move))
        handle_castling (board, who, move, src, dst, 1);

    // Update en passant eligibility:
    handle_en_passant_eligibility (board, who, src_piece, move, &undo_state, 1);

    // check for en passant
    if (is_en_passant_move(move))
        dst_piece = handle_en_passant (board, who, src, dst, 1);

    // put the pieces back
    board_set_piece (board, dst, dst_piece);
    board_set_piece (board, src, src_piece);

    // update king position
    if (piece_type (src_piece) == Piece::King)
        update_king_position (board, who, move, &undo_state, src, dst, 1);

    if (piece_type (orig_src_piece) == Piece::Rook)
    {
        update_current_rook_position (board, who, orig_src_piece, move, &undo_state,
                                      src, dst, 1);
    }

    piece_t captured_piece = captured_material (undo_state, opponent);
    if (piece_type (captured_piece) != Piece::None)
    {
        // NOTE: we reload from the move in case of en-passant, since dst_piece
        // could be none.
        board.material.add (captured_piece);

        if (piece_type (dst_piece) == Piece::Rook)
        {
            update_opponent_rook_position (board, color_invert (who), dst_piece,
                                           &undo_state, src, dst, 1);
        }
    }

    position_undo_move (&board.position, who, src_piece, move, undo_state);

    validate_castle_state (board, move);
}

static move_t castle_parse (const std::string &str, Color who)
{
	int8_t src_row, dst_col;

	if (who == Color::White)
		src_row = LAST_ROW;
	else if (who == Color::Black)
		src_row = FIRST_ROW;
	else
		assert (0);

	std::string transformed { str };
	std::transform (transformed.begin(), transformed.end(), transformed.begin(),
                    [](auto c) -> auto { return ::toupper(c); });

	if (transformed == "O-O-O")
		dst_col = KING_COLUMN - 2;
	else if (transformed == "O-O")
		dst_col = KING_COLUMN + 2;
	else
		return null_move;

	return make_castling_move (src_row, KING_COLUMN, src_row, dst_col);
}

move_t move_parse (const std::string &str, Color who)
{
	bool en_passant   = false;
    bool is_capturing = false;

	// allow any number of spaces/tabs before the two coordinates
	std::string tmp { str };

	if (tmp.empty())
		return null_move;

    tmp.erase(std::remove_if(tmp.begin(), tmp.end(), isspace), tmp.end());
    std::transform (tmp.begin(), tmp.end(), tmp.begin(),
                    [](auto c) -> auto { return ::toupper(c); });

    if (tmp.empty())
        return null_move;

	if (tolower(tmp[0]) == 'o')
		return castle_parse (tmp, who);

	if (tmp.size() < 4)
	    return null_move;

    coord_t src;
    int offset = 0;
    try
    {
        src = coord_parse (tmp.substr (0, 2));
        offset += 2;
    }
    catch (const coord_parse_exception &e)
    {
        return null_move;
    }

	// allow an 'x' between coordinates, which is used to indicate a capture
	if (tmp[offset] == 'X')
    {
        offset++;
        is_capturing = true;
    }

	std::string dst_coord { tmp.substr(offset, 2) };
	offset += 2;
	if (dst_coord.empty())
		return null_move;

	coord_t dst;
    try
    {
        dst = coord_parse (dst_coord);
    }
    catch (const coord_parse_exception &e)
    {
        return null_move;
    }

    std::string rest { tmp.substr(offset) };
    move_t move = make_move (src, dst);
    if (is_capturing)
        move = copy_move_with_capture (move);

	// grab extra identifiers describing the move
	piece_t promoted = make_piece (Color::None, Piece::None);
    if (rest == "EP")
        en_passant = true;
    else if (rest == "(Q)")
        promoted = make_piece (who, Piece::Queen);
    else if (rest == "(N)")
        promoted = make_piece (who, Piece::Knight);
    else if (rest == "(B)")
        promoted = make_piece (who, Piece::Bishop);
    else if (rest == "(R)")
        promoted = make_piece (who, Piece::Rook);

    if (piece_type (promoted) != Piece::None)
        move = copy_move_with_promotion (move, promoted);

    if (en_passant)
        move = make_en_passant_move (ROW (src), COLUMN (src), ROW (dst), COLUMN (dst));

	return move;
}


move_t parse_move (const std::string &str, Color color)
{
    if (tolower(str[0]) == 'o' && color == Color::None)
        throw parse_move_exception("Move requires color, but no color provided");

    move_t result = move_parse (str, color);
    if (color == Color::None &&
        result.move_category != MoveCategory::NormalCapture &&
        result.move_category != MoveCategory::NonCapture)
    {
        throw parse_move_exception("Invalid type of move in parse_simple_move");
    }

    if (is_null_move(result))
        throw parse_move_exception ("Error parsing move");

    return result;
}

std::string to_string (const move_t &move)
{
    coord_t src, dst;

    src = move_src (move);
    dst = move_dst (move);

    if (is_castling_move(move))
    {
        if (COLUMN(dst) - COLUMN(src) < 0)
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

    if (is_capture_move(move))
        result += "x";
    else
        result += " ";

    result += to_string (dst);

    if (is_en_passant_move(move))
    {
        result += " ep";
    }

    if (is_promoting_move(move))
    {
        std::string promoted_piece { piece_char (move_get_promoted_piece (move)) };
        result += "(" + promoted_piece + ")";
    }

    return result;
}