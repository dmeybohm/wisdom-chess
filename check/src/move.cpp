#include <cstdio>
#include <cstring>
#include <mutex>
#include <iostream>
#include <algorithm>

#include "move.h"
#include "coord.h"
#include "board.h"
#include "validate.h"
#include "generate.h"

extern char col_to_char (int col);
extern char row_to_char (int row);

static std::mutex output_mutex;

coord_t en_passant_taken_pawn_coord (coord_t src, coord_t dst)
{
    return coord_create (ROW(src), COLUMN(dst));
}

piece_t handle_en_passant (struct board &board, enum color who,
                           coord_t src, coord_t dst, int undo)
{
    coord_t taken_pawn_pos = en_passant_taken_pawn_coord (src, dst);

    if (undo)
    {
        piece_t taken_pawn = MAKE_PIECE( color_invert(who), PIECE_PAWN );
        board_set_piece (board, taken_pawn_pos, taken_pawn);

        return PIECE_AND_COLOR_NONE; // restore empty square where piece was replaced
    }
    else
    {
        piece_t taken = PIECE_AT (board, taken_pawn_pos);

        assert( PIECE_TYPE(taken) == PIECE_PAWN );
        assert( PIECE_COLOR(taken) != who );
        board_set_piece (board, taken_pawn_pos, PIECE_AND_COLOR_NONE);

        return taken;
    }
}

static move_t get_castling_rook_move (struct board &board, move_t move,
                                      enum color who)
{
    int8_t    src_row, src_col;
    int8_t    dst_row, dst_col;
    coord_t    src, dst;

    assert (is_castling_move (move));

    src = MOVE_SRC(move);
    dst = MOVE_DST(move);

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

    if (!((PIECE_TYPE (PIECE_AT (board, src_row, src_col)) == PIECE_ROOK
           || PIECE_TYPE (PIECE_AT (board, dst_row, dst_col)) == PIECE_ROOK)))
    {
        printf ("move considering: %s (%s to move)\n", to_string(move).c_str(),
                who == COLOR_WHITE ? "White" : "Black");
        board.dump();
        assert (0);
    }

    return move_create (src_row, src_col, dst_row, dst_col);
}

static void handle_castling (struct board &board, enum color who,
                             move_t king_move, coord_t src, coord_t dst, int undo)
{
    move_t  rook_move;
    coord_t rook_src, rook_dst;
    piece_t rook, empty_piece;

    rook_move = get_castling_rook_move (board, king_move, who);

    if (undo)
        assert (PIECE_TYPE (PIECE_AT (board, dst)) == PIECE_KING);
    else
        assert (PIECE_TYPE (PIECE_AT (board, src)) == PIECE_KING);

    assert (abs(COLUMN(src) - COLUMN(dst)) == 2);

    rook_src = MOVE_SRC(rook_move);
    rook_dst = MOVE_DST(rook_move);

    empty_piece = MAKE_PIECE (COLOR_NONE, PIECE_NONE);

    if (undo)
    {
        // undo the rook move
        rook = PIECE_AT (board, rook_dst);

        // undo the rook move
        board_set_piece (board, rook_dst, empty_piece);
        board_set_piece (board, rook_src, rook);
    }
    else
    {
        rook = PIECE_AT (board, rook_src);

        /* do the rook move */
        board_set_piece (board, rook_dst, rook);
        board_set_piece (board, rook_src, empty_piece);
    }
}

void update_king_position (struct board &board, enum color who, move_t move,
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
update_opponent_rook_position (struct board &board, enum color opponent,
                               piece_t dst_piece, undo_move_t *undo_state,
                               coord_t src, coord_t dst, int undo)
{
    assert( PIECE_COLOR(dst_piece) == opponent && PIECE_TYPE(dst_piece) == PIECE_ROOK );

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
        ///
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

static void update_current_rook_position (struct board &board, enum color player,
                                          piece_t src_piece, move_t move,
                                          undo_move_t *undo_state,
                                          coord_t src, coord_t dst, int undo)
{
    if (!( PIECE_COLOR(src_piece) == player &&
           PIECE_TYPE(src_piece) == PIECE_ROOK))
    {
        std::cout << "update_current_rook_position failed: move " << to_string(move) << "\n";
        board.dump();
        abort ();
    }

    assert( PIECE_COLOR(src_piece) == player && PIECE_TYPE(src_piece) == PIECE_ROOK );

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

static void handle_en_passant_eligibility (struct board &board, enum color who, piece_t src_piece,
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
        int direction = PAWN_DIRECTION(who);
        coord_t new_state = no_en_passant_coord;
        if (is_double_square_pawn_move (src_piece, move))
        {
            coord_t src = MOVE_SRC(move);
            int8_t prev_row = NEXT (ROW(src), direction);
            new_state = coord_create (prev_row, COLUMN(src));
        }
        undo_state->en_passant_target[c_index] = board.en_passant_target[c_index];
        undo_state->en_passant_target[o_index] = board.en_passant_target[o_index];
        board.en_passant_target[c_index] = new_state;
        board.en_passant_target[o_index] = no_en_passant_coord;
    }
}

undo_move_t do_move (struct board &board, enum color who, move_t move)
{
    piece_t      orig_src_piece, src_piece, dst_piece;
    coord_t      src, dst;
    undo_move_t  undo_state = empty_undo_state;
    enum color   opponent = color_invert(who);

    src = MOVE_SRC(move);
    dst = MOVE_DST(move);

    orig_src_piece = src_piece = PIECE_AT (board, src);
    dst_piece = PIECE_AT (board, dst);

    if (PIECE_TYPE(dst_piece) != PIECE_NONE)
    {
        assert( is_capture_move(move) );
        undo_state.category = MOVE_CATEGORY_NORMAL_CAPTURE;
        undo_state.taken_piece_type = PIECE_TYPE(dst_piece);
    }

    if (PIECE_TYPE(src_piece) != PIECE_NONE &&
        PIECE_TYPE(dst_piece) != PIECE_NONE)
    {
        assert (PIECE_COLOR(src_piece) != PIECE_COLOR(dst_piece));
    }

    // check for promotion
	if (is_promoting_move(move))
	{
		src_piece = move_get_promoted_piece(move);
		board.material.add (src_piece);
		board.material.remove (MAKE_PIECE(who, PIECE_PAWN));
	}

    // check for en passant
    if (is_en_passant_move(move))
    {
        dst_piece = handle_en_passant (board, who, src, dst, 0);
        undo_state.category = MOVE_CATEGORY_EN_PASSANT;
    }

    // check for castling
    if (is_castling_move(move))
    {
        handle_castling (board, who, move, src, dst, 0);
        undo_state.category = MOVE_CATEGORY_CASTLING;
    }

    handle_en_passant_eligibility (board, who, src_piece, move, &undo_state, 0);

    board_set_piece (board, src, PIECE_AND_COLOR_NONE);
    board_set_piece (board, dst, src_piece);

    // update king position
    if (PIECE_TYPE(src_piece) == PIECE_KING)
        update_king_position (board, who, move, &undo_state, src, dst, 0);

    // update rook position -- for castling
    if (PIECE_TYPE(orig_src_piece) == PIECE_ROOK)
    {
        update_current_rook_position (board, who, orig_src_piece,
                                      move, &undo_state, src, dst, 0);
    }

    piece_t captured_piece = captured_material (undo_state, opponent);
    if (PIECE_TYPE(captured_piece) != PIECE_NONE)
    {
        // update material estimate
        board.material.remove (captured_piece);

        // update castle state if somebody takes the rook
        if (PIECE_TYPE(captured_piece) == PIECE_ROOK)
        {
            update_opponent_rook_position (board, color_invert (who), dst_piece,
                                           &undo_state, src, dst, 0);
        }
    }

    position_do_move (&board.position, who, orig_src_piece, move, undo_state);
    validate_castle_state (board, move);

    return undo_state;
}

void undo_move (struct board &board, enum color who,
                move_t move, undo_move_t undo_state)
{
    piece_t         orig_src_piece, src_piece, dst_piece = PIECE_AND_COLOR_NONE;
    enum piece_type dst_piece_type;
    coord_t         src, dst;
    enum color      opponent = color_invert(who);

    src = MOVE_SRC(move);
    dst = MOVE_DST(move);

    dst_piece_type = undo_state.taken_piece_type;
    orig_src_piece = src_piece = PIECE_AT (board, dst);

    assert( PIECE_TYPE(src_piece) != PIECE_NONE );
    assert( PIECE_COLOR(src_piece) == who );
    if (dst_piece_type != PIECE_NONE)
        dst_piece = MAKE_PIECE( opponent, dst_piece_type );

    // check for promotion
	if (is_promoting_move(move))
	{
		src_piece = MAKE_PIECE(PIECE_COLOR(src_piece), PIECE_PAWN);
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
    if (PIECE_TYPE(src_piece) == PIECE_KING)
        update_king_position (board, who, move, &undo_state, src, dst, 1);

    if (PIECE_TYPE(orig_src_piece) == PIECE_ROOK)
    {
        update_current_rook_position (board, who, orig_src_piece, move, &undo_state,
                                      src, dst, 1);
    }

    piece_t captured_piece = captured_material (undo_state, opponent);
    if (PIECE_TYPE(captured_piece) != PIECE_NONE)
    {
        // NOTE: we reload from the move in case of en-passant, since dst_piece
        // could be none.
        board.material.add (captured_piece);

        if (PIECE_TYPE(dst_piece) == PIECE_ROOK)
        {
            update_opponent_rook_position (board, color_invert (who), dst_piece,
                                           &undo_state, src, dst, 1);
        }
    }

    position_undo_move (&board.position, who, src_piece, move, undo_state);

    validate_castle_state (board, move);
}

static const char *move_str (move_t move)
{
	coord_t src, dst;
	static char buf[256];
	char tmp[32];

	src = MOVE_SRC (move);
	dst = MOVE_DST (move);

	if (is_castling_move(move))
	{
		if (COLUMN(dst) - COLUMN(src) < 0)
		{
			// queenside
			strcpy (buf, "O-O-O");
		}
		else
		{
			// kingside
			strcpy (buf, "O-O");
		}

		return buf;
	}
		
	buf[0] = col_to_char(COLUMN(src));
	buf[1] = row_to_char(ROW(src));
	
	if (is_capture_move(move))
		buf[2] = 'x';
	else
		buf[2] = ' ';

	buf[3] = col_to_char (COLUMN (dst));
	buf[4] = row_to_char (ROW (dst));
	buf[5] = 0;

	if (is_en_passant_move(move))
	{
		snprintf (tmp, sizeof(tmp)-1, " ep");
		strcat (buf, tmp);
	}

	if (is_promoting_move(move))
	{
		snprintf (tmp, sizeof(tmp)-1, "(%c)", 
				  piece_chr(move_get_promoted_piece(move)));
		strcat (buf, tmp);
	}

	return buf;
}

static move_t castle_parse (std::string_view str, enum color who)
{
	int8_t src_row, dst_col;

	if (who == COLOR_WHITE)
		src_row = LAST_ROW;
	else if (who == COLOR_BLACK)
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

	return move_create_castling (src_row, KING_COLUMN, src_row, dst_col);
}

static const char *skip_whitespace (const char *p)
{
	while (*p == ' ' || *p == '\t')
		p++;

	return p;
}

move_t move_parse (std::string_view str, enum color who)
{
	bool en_passant   = false;
    bool is_capturing = false;

	if (str.empty())
		return null_move;

	if (tolower(str[0]) == 'o')
		return castle_parse (str, who);

	if (str.size() < 4)
	    return null_move;

	// allow any number of spaces/tabs before the two coordinates
	std::string tmp { str };

    tmp.erase(std::remove_if(tmp.begin(), tmp.end(), isspace), tmp.end());

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

    std::string_view next { tmp.substr(2) };
	// allow an 'x' between coordinates, which is used to indicate a capture
	if (tmp[offset] == 'x')
    {
        offset++;
        is_capturing = true;
    }

	std::string_view rest { tmp.substr(offset) };
	if (rest.empty())
		return null_move;

	coord_t dst;
    try
    {
        dst = coord_parse (rest.substr(0, 2));
        rest = rest.substr(2);
    }
    catch (const coord_parse_exception &e)
    {
        return null_move;
    }

    move_t move = move_create (src, dst);
    if (is_capturing)
        move = move_with_capture(move);

	// grab extra identifiers describing the move
	piece_t promoted = MAKE_PIECE (COLOR_NONE, PIECE_NONE);
    if (rest == "ep")
        en_passant = true;
    else if (rest == "(Q)")
        promoted = MAKE_PIECE (who, PIECE_QUEEN);
    else if (rest ==  "(N)")
        promoted = MAKE_PIECE (who, PIECE_KNIGHT);
    else if (rest == "(B)")
        promoted = MAKE_PIECE (who, PIECE_BISHOP);
    else if (rest == "(R)")
        promoted = MAKE_PIECE (who, PIECE_ROOK);

    if (PIECE_TYPE(promoted) != PIECE_NONE)
        move = move_with_promotion (move, promoted);

    if (en_passant)
        move = move_create_en_passant (ROW(src), COLUMN(src), ROW(dst), COLUMN(dst));

	return move;
}

char row_to_char (int row)
{
    return 8-row + '0';
}

char col_to_char (int col)
{
    return col + 'a';
}

move_t parse_move (std::string_view str, enum color color)
{
    if (tolower(str[0]) == 'o' && color == COLOR_NONE)
        throw parse_move_exception("Move requires color, but no color provided");

    move_t result = move_parse (str, color);
    if (color == COLOR_NONE &&
        result.move_category != MOVE_CATEGORY_NORMAL_CAPTURE &&
        result.move_category != MOVE_CATEGORY_NON_CAPTURE)
    {
        throw parse_move_exception("Invalid type of move in parse_simple_move");
    }

    if (is_null_move(result))
        throw parse_move_exception ("Error parsing move");

    return result;
}

std::string to_string (const move_t &move)
{
    std::lock_guard lock_guard { output_mutex };
    const char *str = move_str (move);
    return { str };
}