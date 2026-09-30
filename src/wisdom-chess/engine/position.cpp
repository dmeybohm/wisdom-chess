#include <iostream>

#include "wisdom-chess/engine/position.hpp"
#include "wisdom-chess/engine/board.hpp"

namespace wisdom
{
    // clang-format off
    constexpr int pawn_positions[Num_Rows][Num_Columns] = {
            {  0,  0,  0,  0,  0,  0,  0,  0 },
            { +9, +9, +9, +9, +9, +9, +9, +9 },
            { +2, +2, +4, +6, +6, +4, +2, +2 },
            { +1, +1, +2, +5, +5, +2, +1, +1 },
            {  0,  0,  0,  4, +4,  0,  0,  0 },
            { +1, -1, -2,  0,  0, -2, -1, +1 },
            { +1, +2, +2, -4, -4, +2, +2, +1 },
            {  0,  0,  0,  0,  0,  0,  0,  0 },
    };

    constexpr int king_positions[Num_Rows][Num_Columns] = {
            { -6, -8, -8, -9, -9, -8, -8, -6 },
            { -6, -8, -8, -9, -9, -8, -8, -6 },
            { -6, -8, -8, -9, -9, -8, -8, -6 },
            { -6, -8, -8, -9, -9, -8, -8, -6 },
            { -4, -6, -6, -8, -8, -6, -6, -4 },
            { -2, -4, -4, -4, -4, -4, -4, -2 },
            { +4, +4,  0,  0,  0,  0, +4, +4 },
            { +4, +6, +2,  0,  0, +2, +6, +4 },
    };

    constexpr int knight_positions[Num_Rows][Num_Columns] = {
            { -9, -8, -6, -6, -6, -6, -8, -9 },
            { -8, -4,  0,  0,  0,  0, -4, -8 },
            { -6,  0, +2, +3, +3, +2,  0, -6 },
            { -6, +1, +3, +4, +4, +3, +1, -6 },
            { -6,  0, +3, +4, +4, +3,  0, -6 },
            { -6, +1, +2, +3, +3, +2, +1, -6 },
            { -8, -4,  0, +1, +1,  0, -4, -8 },
            { -9, -8, -6, -6, -6, -6, -8, -9 },
    };

    constexpr int bishop_positions[Num_Rows][Num_Columns] = {
            { -4, -2, -2, -2, -2, -2, -2, -4 },
            { -2,  0,  0,  0,  0,  0,  0, -2 },
            { -2,  0, +1, +2, +2, +1,  0, -2 },
            { -2, +1, +1, +2, +2, +1, +1, -2 },
            { -2,  0, +2, +2, +2, +2,  0, -2 },
            { -2, +2, +2, +2, +2, +2, +2, -2 },
            { -2, +1,  0,  0,  0,  0, +1, -2 },
            { -4, -2, -2, -2, -2, -2, -2, -2 },
    };

    constexpr int rook_positions[Num_Rows][Num_Columns] = {
            {  0,  0,  0,  0,  0,  0,  0,  0 },
            { +1, +2, +2, +2, +2, +2, +2, +1 },
            { -1,  0,  0,  0,  0,  0,  0, -1 },
            { -1,  0,  0,  0,  0,  0,  0, -1 },
            { -1,  0,  0,  0,  0,  0,  0, -1 },
            { -1,  0,  0,  0,  0,  0,  0, -1 },
            { -1,  0,  0,  0,  0,  0,  0, -1 },
            {  0,  0,  0, +1, +1,  0,  0,  0 },
    };

    constexpr int queen_positions[Num_Rows][Num_Columns] = {
            { -4, -2, -2, -1, -1, -2, -2, -4 },
            { -2,  0,  0,  0,  0,  0,  0, -2 },
            { -2,  0, +1, +1, +1, +1,  0, -2 },
            { -1,  0, +1, +1, +1, +1,  0, -1 },
            {  0,  0, +1, +1, +1, +1,  0, -1 },
            { -2, +1, +1, +1, +1, +1,  0, -2 },
            { -2,  0, +1,  0,  0,  0,  0, -2 },
            { -4, -2, -2, -1, -1, -2, -2, -4 },
    };
    // clang-format on

    namespace
    {
        auto
        translatePosition (Coord coord, Color who)
            -> Coord
        {
            if (who == Color::White)
                return coord;

            return makeCoord (narrow_cast<int8_t> (Last_Row - coord.row()), coord.column());
        }

        auto
        change (Coord coord, Color who, ColoredPiece piece)
            -> int
        {
            Coord translated_pos = translatePosition (coord, who);
            int8_t row = translated_pos.row();
            int8_t col = translated_pos.column();

            switch (pieceType (piece))
            {
                case Piece::Pawn:
                    return pawn_positions[row][col];
                case Piece::Knight:
                    return knight_positions[row][col];
                case Piece::Bishop:
                    return bishop_positions[row][col];
                case Piece::Rook:
                    return rook_positions[row][col];
                case Piece::Queen:
                    return queen_positions[row][col];
                case Piece::King:
                    return king_positions[row][col];
                default:
                    terminateOnCheckFailure ("Precondition", "a piece type", std::source_location::current());
            }
        }
    }

    auto
    Position::overallScore (Color who) const
        -> int
    {
        ColorIndex index = colorIndex (who);
        ColorIndex inverted = colorIndex (colorInvert (who));
        ASSERT( my_score[index] < 3000 && my_score[index] > -3000 );
        ASSERT( my_score[inverted] < 3000 && my_score[inverted] > -3000 );
        int result = my_score[index] - my_score[inverted];
        ASSERT( result < 3000 );
        return result * Position_Score_Scale;
    }

    void Position::add (Color who, Coord coord, ColoredPiece piece)
    {
        ColorIndex index = colorIndex (who);
        my_score[index] += change (coord, who, piece);
    }

    void Position::remove (Color who, Coord coord, ColoredPiece piece)
    {
        ColorIndex index = colorIndex (who);
        my_score[index] -= change (coord, who, piece);
    }

    void Position::applyMove (Color who, ColoredPiece src_piece, Move move, ColoredPiece dst_piece)
    {
        Color opponent = colorInvert (who);

        Coord src = move.getSrc();
        Coord dst = move.getDst();

        remove (who, src, src_piece);

        switch (move.getMoveCategory())
        {
            case MoveCategory::Default:
                break;

            case MoveCategory::NormalCapturing:
                {
                    Coord taken_piece_coord = dst;
                    remove (opponent, taken_piece_coord, dst_piece);
                }
                break;

            case MoveCategory::EnPassant:
                {
                    Coord taken_pawn_coord = enPassantTakenPawnCoord (src, dst);
                    remove (
                        opponent,
                        taken_pawn_coord,
                        ColoredPiece::make (opponent, Piece::Pawn)
                    );
                }
                break;

            case MoveCategory::Castling:
                {
                    Move rook_move = castlingRookMove (move);
                    ColoredPiece rook = ColoredPiece::make (who, Piece::Rook);

                    remove (who, rook_move.getSrc(), rook);
                    add (who, rook_move.getDst(), rook);
                }
                break;
        }

        ColoredPiece new_piece = move.isPromoting()
            ? ColoredPiece::make (who, move.getPromotedPiece())
            : src_piece;

        add (who, dst, new_piece);
    }

    auto
    Position::individualScore (Color who) const
        -> int
    {
        return my_score[colorIndex (who)];
    }

    Position::Position (const Board& board)
    {
        for (auto coord : Board::allCoords())
        {
            auto piece = board.pieceAt (coord);
            if (piece != Piece_And_Color_None)
                add (pieceColor (piece), coord, piece);
        }
    }

    auto
    operator<< (std::ostream& ostream, const Position& position)
        -> std::ostream&
    {
        return ostream << "{ " << position.my_score[0] << ", " << position.my_score[1] << "}";
    }
}
