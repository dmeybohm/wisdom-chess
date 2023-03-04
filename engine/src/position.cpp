#include "position.hpp"
#include "board.hpp"

namespace wisdom
{
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

    static Coord translate_position (Coord coord, Color who)
    {
        if (who == Color::White)
            return coord;

        int8_t row = Row (coord);
        int8_t col = Column (coord);

        return make_coord (
            gsl::narrow_cast<int8_t>(Last_Row - row),
            gsl::narrow_cast<int8_t>(Last_Column - col)
        );
    }

    static int8_t castling_row_from_color (Color who)
    {
        switch (who)
        {
            case Color::White: return 7;
            case Color::Black: return 0;
            default: throw Error {
              "Invalid color in castling_row_from_color()"
            };
        }
    }

    static int change (Coord coord, Color who, ColoredPiece piece)
    {
        Coord translated_pos = translate_position (coord, who);
        int8_t row = Row (translated_pos);
        int8_t col = Column (translated_pos);

        // todo convert enum to integer index instead of using switch.
        switch (piece_type (piece))
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

    int Position::overall_score (Color who) const
    {
        ColorIndex index = color_index (who);
        ColorIndex inverted = color_index (color_invert (who));
        assert (this->my_score[index] < 3000 && this->my_score[index] > -3000);
        assert (this->my_score[inverted] < 3000 && this->my_score[inverted] > -3000);
        int result = this->my_score[index] - this->my_score[inverted];
        assert (result < 3000);
        return result * Position_Score_Scale;
    }

    void Position::add (Color who, Coord coord, ColoredPiece piece)
    {
        ColorIndex index = color_index (who);
        this->my_score[index] += change (coord, who, piece);
    }

    void Position::remove (Color who, Coord coord, ColoredPiece piece)
    {
        ColorIndex index = color_index (who);
        this->my_score[index] -= change (coord, who, piece);
    }

    void Position::apply_move (Color who, ColoredPiece src_piece, Move move, ColoredPiece dst_piece)
    {
        Color opponent = color_invert (who);

        Coord src = move.get_src();
        Coord dst = move.get_dst();

        this->remove (who, src, src_piece);

        switch (move.get_move_category())
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
                    Coord taken_pawn_coord = en_passant_taken_pawn_coord (src, dst);
                    this->remove (
                        opponent,
                        taken_pawn_coord,
                        ColoredPiece::make (opponent, Piece::Pawn)
                    );
                }
                break;

            case MoveCategory::Castling:
                {
                    int8_t rook_src_row = castling_row_from_color (who);
                    auto rook_src_col = gsl::narrow_cast<int8_t> (
                        move.is_castling_on_kingside ()
                            ? King_Rook_Column : Queen_Rook_Column
                    );
                    auto rook_dst_col = gsl::narrow_cast<int8_t> (
                        move.is_castling_on_kingside ()
                            ? Kingside_Castled_Rook_Column : Queenside_Castled_Rook_Column
                    );

                    Coord src_rook_coord = make_coord (rook_src_row, rook_src_col);
                    Coord dst_rook_coord = make_coord (rook_src_row, rook_dst_col);
                    ColoredPiece rook = ColoredPiece::make (who, Piece::Rook);

                    this->remove (who, src_rook_coord, rook);
                    this->add (who, dst_rook_coord, rook);
                }
                break;

            default:
                throw Error { "Invalid move type." };
        }

        ColoredPiece new_piece = move.is_promoting() ? move.get_promoted_piece() : src_piece;

        this->add (who, dst, new_piece);
    }

    int Position::individual_score (Color who) const
    {
        return my_score[color_index (who)];
    }

    Position::Position (const Board& board)
    {
        for (auto coord : board.all_coords ())
        {
            auto piece = board.piece_at (coord);
            if (piece != Piece_And_Color_None)
                add (piece_color (piece), coord, piece);
        }
    }

    auto operator<< (std::ostream& ostream, Position& position) -> std::ostream&
    {
        return ostream << "{ " << position.my_score[0] << ", " << position.my_score[1] << "}";
    }
}
