#include "board.hpp"
#include "move.hpp"
#include "validate.hpp"

#include <iostream>

namespace wisdom
{
    static void check_it (Board &board, Color who, Move mv, int expr)
    {
        if (!expr)
        {
            std::cerr << "move considering: " << to_string (mv) << "\n";
            board.dump ();
            abort ();
        }
    }

    static void validate_castle (Board &board, CastlingState state, Color who, Move mv)
    {
        // check positions of the pieces:
        int8_t row = who == Color::White ? 7 : 0;
        int8_t col = state == Castle_Queenside ? 0 : 7;
        ColoredPiece supposed_king = piece_at (board, row, 4);
        ColoredPiece supposed_rook = piece_at (board, row, col);
        if (able_to_castle (board, who, state))
        {
            check_it (board, who, mv, piece_type (supposed_king) == Piece::King);
            check_it (board, who, mv, piece_type (supposed_rook) == Piece::Rook);
            check_it (board, who, mv, piece_color (supposed_king) == who);
            check_it (board, who, mv, piece_color (supposed_rook) == who);
            check_it (board, who, mv, coord_equals (king_position (board, who), make_coord (row, 4)));
        }
    }

    void validate_castle_state (Board &board, Move mv)
    {
#ifdef VALIDATE_CASTLE_STATE
        validate_castle (board, Castle_Queenside, Color::White, mv);
        validate_castle (board, Castle_Kingside, Color::White, mv);
        validate_castle (board, Castle_Queenside, Color::Black, mv);
        validate_castle (board, Castle_Kingside, Color::Black, mv);
#endif
    }
}