#include "validate.hpp"
#include "board.hpp"
#include "move.hpp"
#include "validate.hpp"

#include <iostream>

namespace wisdom
{
    class CastleConsistencyProblem : public Error
    {
    public:
        CastleConsistencyProblem() : Error("Castling consistency problem.")
        {}
    };

    static void check_it ([[maybe_unused]] Board &board, [[maybe_unused]] Color who,
                          [[maybe_unused]] Move mv, [[maybe_unused]] int expr)
    {
        if (!expr)
        {
            std::cerr << "move considering: " << to_string (mv) << "\n";
            throw CastleConsistencyProblem {};
        }
    }

    [[maybe_unused]]
    static void validate_castle (Board &board, CastlingState state, Color who, Move mv)
    {
        // check positions of the pieces:
        int row = who == Color::White ? 7 : 0;
        int col = state == Castle_Queenside ? 0 : 7;
        ColoredPiece supposed_king = piece_at (board, row, 4);
        ColoredPiece supposed_rook = piece_at (board, row, col);
        if (board.able_to_castle ( who, state))
        {
            check_it (board, who, mv, piece_type (supposed_king) == Piece::King);
            check_it (board, who, mv, piece_type (supposed_rook) == Piece::Rook);
            check_it (board, who, mv, piece_color (supposed_king) == who);
            check_it (board, who, mv, piece_color (supposed_rook) == who);
            check_it (board, who, mv, coord_equals (board.get_king_position (who), make_coord (row, 4)));
        }
    }

    [[maybe_unused]]
    void do_validate_castle_state ([[maybe_unused]] Board &board, [[maybe_unused]] Move mv)
    {
        validate_castle (board, Castle_Queenside, Color::White, mv);
        validate_castle (board, Castle_Kingside, Color::White, mv);
        validate_castle (board, Castle_Queenside, Color::Black, mv);
        validate_castle (board, Castle_Kingside, Color::Black, mv);
    }
}