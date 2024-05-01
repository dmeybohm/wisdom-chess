#include "board_builder.hpp"

#include "board.hpp"
#include "move.hpp"
#include "coord.hpp"

namespace wisdom
{
    auto BoardBuilder::fromDefaultPosition() -> BoardBuilder
    {
        auto result = BoardBuilder {};
        result.addRowOfSameColor (0, Color::Black, Default_Piece_Row);
        result.addRowOfSameColorAndPiece (1, Color::Black, Piece::Pawn);
        result.addRowOfSameColorAndPiece (6, Color::White, Piece::Pawn);
        result.addRowOfSameColor (7, Color::White, Default_Piece_Row);

        result.setCastling (Color::White, Either_Side_Eligible);
        result.setCastling (Color::Black, Either_Side_Eligible);

        return result;
    }

    void BoardBuilder::addPiece (const string& coord_str, Color who, Piece piece_type)
    {
        if (coord_str.size() != 2)
            throw BoardBuilderError ("Invalid coordinate string!");

        Coord algebraic = coordParse (coord_str);

        addPiece (algebraic.row(), algebraic.column(), who, piece_type);
    }

    auto BoardBuilder::calculateCastleStateFromPosition (Color who) const
        -> CastlingEligibility
    {
        auto row = castlingRowForColor (who);
        int8_t king_col = King_Column;

        CastlingEligibility state = Either_Side_Eligible;
        ColoredPiece prospective_king = pieceAt (makeCoord (row, king_col));
        ColoredPiece prospective_queen_rook = pieceAt (makeCoord (row, 0));
        ColoredPiece prospective_king_rook = pieceAt (makeCoord (row, 7));

        if (pieceType (prospective_king) != Piece::King || pieceColor (prospective_king) != who
            || pieceType (prospective_queen_rook) != Piece::Rook
            || pieceColor (prospective_queen_rook) != who)
        {
            state |= CastlingIneligible::Queenside;
        }
        if (pieceType (prospective_king) != Piece::King || pieceColor (prospective_king) != who
            || pieceType (prospective_king_rook) != Piece::Rook
            || pieceColor (prospective_king_rook) != who)
        {
            state |= CastlingIneligible::Kingside;
        }

        return state;
    }

    void BoardBuilder::addPiece (int row, int col, Color who, Piece piece_type)
    {
        if (row < 0 || row >= Num_Rows)
            throw BoardBuilderError ("Invalid row!");

        if (col < 0 || col >= Num_Columns)
            throw BoardBuilderError ("Invalid column!");

        if (piece_type == Piece::None)
            return;

        auto coord = Coord::make (row, col);
        my_squares[coord.index()] = ColoredPiece::make (who, piece_type);

        if (piece_type == Piece::King)
            my_king_positions[colorIndex (who)] = coord;
    }

    void BoardBuilder::addPieces (Color who, const vector<CoordAndPiece>& pieces)
    {
        for (auto&& it : pieces)
            addPiece (it.coord, who, it.piece_type);
    }

    void BoardBuilder::addRowOfSameColorAndPiece (int row, Color who, Piece piece_type)
    {
        for (int col = 0; col < Num_Columns; col++)
            addPiece (row, col, who, piece_type);
    }

    void
    BoardBuilder::addRowOfSameColorAndPiece (const string& coord_str, Color who, Piece piece_type)
    {
        Coord coord = coordParse (coord_str);

        for (int col = 0; col < Num_Columns; col++)
            addPiece (coord.row(), col, who, piece_type);
    }

    void BoardBuilder::addRowOfSameColor (int row, Color who, const PieceRow& piece_types)
    {
        for (auto col = 0; col < Num_Columns; col++)
            addPiece (row, col, who, piece_types[col]);
    }

    void BoardBuilder::addRowOfSameColor (
        const string& coord_str, 
        Color who, 
        const PieceRow& piece_types
    ) {
        Coord coord = coordParse (coord_str);

        for (auto col = 0; col < Num_Columns; col++)
            addPiece (coord.row(), col, who, piece_types[col]);
    }

    void BoardBuilder::setCurrentTurn (Color who)
    {
        my_current_turn = who;
    }

    void BoardBuilder::setEnPassantTarget (Color vulnerable_color, const string& coord_str)
    {
        my_en_passant_targets[colorIndex (vulnerable_color)] = coordParse (coord_str);
    }

    void BoardBuilder::setCastling (Color who, CastlingEligibility state)
    {
        my_castle_states[colorIndex (who)] = state;
    }

    void BoardBuilder::setHalfMovesClock (int new_half_moves_clock)
    {
        my_half_moves_clock = new_half_moves_clock;
    }

    void BoardBuilder::setFullMoves (int new_full_moves)
    {
        my_full_moves = new_full_moves;
    }
}
