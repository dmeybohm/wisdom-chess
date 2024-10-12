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
            { +1, -1, -2,  0,  0,  2, -1, +1 },
            { +1, +2, +2, -4, -4, +2, +2, +1 },
            {  0,  0,  0,  0,  0,  0,  0,  0 },
    };

    constexpr int king_positions[Num_Rows][Num_Columns] = {
            { -6, -8, -8, -9, -9, -4, -4, -6 },
            { -6, -8, -8, -9, -9, -4, -4, -6 },
            { -6, -8, -8, -9, -9, -4, -4, -6 },
            { -6, -8, -8, -9, -9, -8, -8, -6 },
            { -4, -6, -6, -8, -8, -6, -6, -2 },
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
            { -4, -2, -2, -2, -2, -2, -2, -2 },
            { -2,  0,  0,  0,  0,  0,  0, -2 },
            { -2,  0, +1, +2, +2, +1,  0, -2 },
            { -2,  0, +1, +2, +2, +1, +1, -2 },
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
            { -2,  0, +1, +1, +1, +1,  0, -2 },
            { -2,  0, +1,  0,  0,  0,  0, -2 },
            { -4, -2, -2, -1, -1, -2, -2, -4 },
    };
    // clang-format on

    static auto 
    translatePosition (Coord coord, Color who)
        -> Coord
    {
        if (who == Color::White)
            return coord;

        int8_t row = coord.row();
        int8_t col = coord.column();

        return makeCoord (
            narrow_cast<int8_t> (Last_Row - row),
            narrow_cast<int8_t> (Last_Column - col)
        );
    }

    static auto 
    castlingRowFromColor (Color who)
        -> int8_t
    {
        switch (who)
        {
            case Color::White:
                return 7;
            case Color::Black:
                return 0;
            default:
                throw Error { "Invalid color in castlingRowFromColor()" };
        }
    }

    static auto 
    change (Coord coord, Color who, ColoredPiece piece)
        -> int
    {
        Coord translated_pos = translatePosition (coord, who);
        int8_t row = translated_pos.row();
        int8_t col = translated_pos.column();

        // todo convert enum to integer index instead of using switch.
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
                throw Error ("Invalid position of piece to change.");
        }
    }

    auto 
    Position::overallScore (Color who) const
        -> int
    {
        ColorIndex index = colorIndex (who);
        ColorIndex inverted = colorIndex (colorInvert (who));
        assert (this->my_score[index] < 3000 && this->my_score[index] > -3000);
        assert (this->my_score[inverted] < 3000 && this->my_score[inverted] > -3000);
        int result = this->my_score[index] - this->my_score[inverted];
        assert (result < 3000);
        return result * Position_Score_Scale;
    }

    void Position::add (Color who, Coord coord, ColoredPiece piece)
    {
        ColorIndex index = colorIndex (who);
        this->my_score[index] += change (coord, who, piece);
    }

    void Position::remove (Color who, Coord coord, ColoredPiece piece)
    {
        ColorIndex index = colorIndex (who);
        this->my_score[index] -= change (coord, who, piece);
    }

    void Position::applyMove (Color who, ColoredPiece src_piece, Move move, ColoredPiece dst_piece)
    {
        Color opponent = colorInvert (who);

        Coord src = move.getSrc();
        Coord dst = move.getDst();

        this->remove (who, src, src_piece);

        switch (move.getMoveCategory())
        {
            case MoveCategory::Default:
                break;

            case MoveCategory::NormalCapturing:
                {
                    Coord taken_piece_coord = dst;
                    this->remove (opponent, taken_piece_coord, dst_piece);
                }
                break;

            case MoveCategory::EnPassant:
                {
                    Coord taken_pawn_coord = enPassantTakenPawnCoord (src, dst);
                    this->remove (
                        opponent,
                        taken_pawn_coord,
                        ColoredPiece::make (opponent, Piece::Pawn)
                    );
                }
                break;

            case MoveCategory::Castling:
                {
                    int8_t rook_src_row = castlingRowFromColor (who);
                    auto rook_src_col = narrow_cast<int8_t> (
                        move.isCastlingOnKingside()
                            ? King_Rook_Column : Queen_Rook_Column
                    );
                    auto rook_dst_col = narrow_cast<int8_t> (
                        move.isCastlingOnKingside()
                            ? Kingside_Castled_Rook_Column : Queenside_Castled_Rook_Column
                    );

                    Coord src_rook_coord = makeCoord (rook_src_row, rook_src_col);
                    Coord dst_rook_coord = makeCoord (rook_src_row, rook_dst_col);
                    ColoredPiece rook = ColoredPiece::make (who, Piece::Rook);

                    this->remove (who, src_rook_coord, rook);
                    this->add (who, dst_rook_coord, rook);
                }
                break;

            default:
                throw Error { "Invalid move type." };
        }

        ColoredPiece new_piece = move.isPromoting() ? move.getPromotedPiece() : src_piece;

        this->add (who, dst, new_piece);
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
    operator<< (std::ostream& ostream, Position& position) 
        -> std::ostream&
    {
        return ostream << "{ " << position.my_score[0] << ", " << position.my_score[1] << "}";
    }
}
