#include "board_builder.hpp"

#include "board.hpp"
#include "move.hpp"
#include "coord.hpp"

namespace wisdom
{
    auto BoardBuilder::fromDefaultPosition() -> BoardBuilder
    {
        array<Piece, Num_Columns> back_rank = {
                Piece::Rook, Piece::Knight, Piece::Bishop, Piece::Queen, Piece::King,
                Piece::Bishop, Piece::Knight, Piece::Rook,
        };

        array<Piece, Num_Columns> pawns = {
                Piece::Pawn, Piece::Pawn, Piece::Pawn, Piece::Pawn, Piece::Pawn,
                Piece::Pawn, Piece::Pawn, Piece::Pawn,
        };

        auto result = BoardBuilder {};
        result.addRowOfSameColor (0, Color::Black, Default_Piece_Row);
        result.addRowOfSameColorAndPiece (1, Color::Black, Piece::Pawn);
        result.addRowOfSameColorAndPiece (6, Color::White, Piece::Pawn);
        result.addRowOfSameColor (7, Color::White, Default_Piece_Row);

        result.setCastling (Color::White, CastlingEligible::EitherSideEligible);
        result.setCastling (Color::Black, CastlingEligible::EitherSideEligible);

        return result;
    }

    void BoardBuilder::addPiece (const string& coord_str, Color who, Piece piece_type)
    {
        if (coord_str.size () != 2)
            throw BoardBuilderError ("Invalid coordinate string!");

        Coord algebraic = coord_parse (coord_str);

        addPiece (Row (algebraic), Column (algebraic), who, piece_type);
    }

    auto BoardBuilder::calculateCastleStateFromPosition (Color who) const
        -> CastlingEligibility
    {
        auto row = castling_row_for_color (who);
        int8_t king_col = King_Column;

        CastlingEligibility state = CastlingEligible::EitherSideEligible;
        ColoredPiece prospective_king = pieceAt (make_coord (row, king_col));
        ColoredPiece prospective_queen_rook = pieceAt (make_coord (row, 0));
        ColoredPiece prospective_king_rook = pieceAt (make_coord (row, 7));

        if (piece_type (prospective_king) != Piece::King ||
            piece_color (prospective_king) != who ||
            piece_type (prospective_queen_rook) != Piece::Rook ||
            piece_color (prospective_queen_rook) != who)
        {
            state |= CastlingEligible::QueensideIneligible;
        }
        if (piece_type (prospective_king) != Piece::King ||
            piece_color (prospective_king) != who ||
            piece_type (prospective_king_rook) != Piece::Rook ||
            piece_color (prospective_king_rook) != who)
        {
            state |= CastlingEligible::KingsideIneligible;
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

        auto coord = make_coord (row, col);
        my_squares[coord_index (coord)] = ColoredPiece::make (who, piece_type);

        if (piece_type == Piece::King)
            my_king_positions[color_index (who)] = coord;
    }

    void BoardBuilder::addPieces (Color who, const vector<CoordAndPiece> &pieces)
    {
        for (auto&& it : pieces)
            addPiece (it.coord, who, it.piece_type);
    }

    void BoardBuilder::addRowOfSameColorAndPiece (int row, Color who, Piece piece_type)
    {
        for (int col = 0; col < Num_Columns; col++)
            addPiece (row, col, who, piece_type);
    }

    void BoardBuilder::addRowOfSameColorAndPiece (const string& coord_str, Color who,
                                                        Piece piece_type)
    {
        Coord coord = coord_parse (coord_str);

        for (int col = 0; col < Num_Columns; col++)
            addPiece (Row (coord), col, who, piece_type);
    }

    void BoardBuilder::addRowOfSameColor (int row, Color who,
                                              const PieceRow& piece_types)
    {
        for (auto col = 0; col < Num_Columns; col++)
            addPiece (row, col, who, piece_types[col]);
    }

    void BoardBuilder::addRowOfSameColor (const string& coord_str, Color who,
                                              const PieceRow& piece_types)
    {
        Coord coord = coord_parse (coord_str);

        for (auto col = 0; col < Num_Columns; col++)
            addPiece (Row (coord), col, who, piece_types[col]);
    }

    void BoardBuilder::setCurrentTurn (Color who)
    {
        my_current_turn = who;
    }

    void BoardBuilder::setEnPassantTarget (Color vulnerable_color, const string& coord_str)
    {
        my_en_passant_targets[color_index (vulnerable_color)] = coord_parse (coord_str);
    }

    void BoardBuilder::setCastling (Color who, CastlingEligibility state)
    {
        my_castle_states[color_index (who)] = state;
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
