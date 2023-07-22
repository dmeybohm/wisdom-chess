#include "move.hpp"
#include "board.hpp"
#include "generate.hpp"

#include <iostream>

namespace wisdom
{
    auto enPassantTakenPawnCoord (Coord src, Coord dst) -> Coord
    {
        return makeCoord (Row (src), Column (dst));
    }

    constexpr bool isDoubleSquarePawnMove (ColoredPiece src_piece, Move move)
    {
        Coord src = move.getSrc();
        Coord dst = move.getDst();
        return pieceType (src_piece) == Piece::Pawn && abs (Row (src) - Row (dst)) == 2;
    }

    // Returns the taken piece
    auto Board::applyForEnPassant (Color who, Coord src, Coord dst)
        -> ColoredPiece
    {
        Coord taken_pawn_pos = enPassantTakenPawnCoord (src, dst);
        [[maybe_unused]] ColoredPiece taken_piece = pieceAt (taken_pawn_pos);

        assert (pieceType (taken_piece) == Piece::Pawn);
        assert (pieceColor (taken_piece) == colorInvert (who));

        setPiece (taken_pawn_pos, Piece_And_Color_None);

        return ColoredPiece::make (colorInvert (who), Piece::Pawn);
    }

    auto Board::getCastlingRookMove (Move move, Color who) -> Move
    {
        int src_row, src_col;
        int dst_row, dst_col;

        assert (move.isCastling());

        Coord src = move.getSrc();
        Coord dst = move.getDst();

        src_row = Row<int> (src);
        dst_row = Row<int> (dst);

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

        if (!((pieceType (pieceAt (src_row, src_col)) == Piece::Rook
               || pieceType (pieceAt (dst_row, dst_col)) == Piece::Rook)))
        {
            throw MoveConsistencyProblem { "move considering: " + wisdom::asString (move) + "("
                                           + wisdom::asString (who) + " to move)" };
        }

        return Move::make (src_row, src_col, dst_row, dst_col);
    }

    void Board::applyForCastlingMove (Color who, Move king_move,
                                      [[maybe_unused]] Coord src, [[maybe_unused]] Coord dst)
    {
        Move rook_move = getCastlingRookMove (king_move, who);

        assert (pieceType (pieceAt (src)) == Piece::King);
        assert (abs (Column (src) - Column (dst)) == 2);

        auto rook_src = rook_move.getSrc();
        auto rook_dst = rook_move.getDst();

        auto empty_piece = ColoredPiece::make (Color::None, Piece::None);

        auto rook = pieceAt (rook_src);

        // do the rook move
        setPiece (rook_dst, rook);
        setPiece (rook_src, empty_piece);
    }

    void Board::applyForKingMove (Color who,
                                     [[maybe_unused]] Coord src, Coord dst)
    {
        setKingPosition (who, dst);

        // set as not able to castle
        if (ableToCastle (
                who, CastlingEligible::KingsideIneligible | CastlingEligible::QueensideIneligible))
        {
            // set the new castle status
            removeCastlingEligibility (
                who, CastlingEligible::KingsideIneligible | CastlingEligible::QueensideIneligible);
        }
    }

    void Board::applyForRookCapture (Color opponent, ColoredPiece dst_piece,
                                     Coord src, Coord dst)
    {
        assert (pieceColor (dst_piece) == opponent && pieceType (dst_piece) == Piece::Rook);

        CastlingEligibility castle_state = CastlingEligible::EitherSideEligible;

        int castle_rook_row = opponent == Color::White ? Last_Row : First_Row;

        //
        // This needs to distinguish between captures that end
        // up on the rook and moves from the rook itself.
        //
        if (Column (dst) == First_Column && Row (dst) == castle_rook_row)
            castle_state = CastlingEligible::QueensideIneligible;
        else if (Column (dst) == Last_Column && Row (dst) == castle_rook_row)
            castle_state = CastlingEligible::KingsideIneligible;

        //
        // Set inability to castle on one side.
        //
        if (ableToCastle (opponent, castle_state))
            removeCastlingEligibility (opponent, castle_state);
    }

    void Board::applyForRookMove (Color player, ColoredPiece src_piece,
                                  Move move, Coord src, Coord dst)
    {
        if (!(pieceColor (src_piece) == player && pieceType (src_piece) == Piece::Rook))
        {
            throw MoveConsistencyProblem { "applyForRookMove failed: move "
                                           + wisdom::asString (move) };
        }

        assert (pieceColor (src_piece) == player && pieceType (src_piece) == Piece::Rook);

        CastlingEligibility affects_castle_state = CastlingEligible::EitherSideEligible;
        int castle_src_row = player == Color::White ? Last_Row : First_Row;

        //
        // This needs to distinguish between captures that end
        // up on the rook and moves from the rook itself.
        //
        if (Row (src) == castle_src_row)
        {
            if (Column (src) == Queen_Rook_Column)
                affects_castle_state = CastlingEligible::QueensideIneligible;
            else if (Column (src) == King_Rook_Column)
                affects_castle_state = CastlingEligible::KingsideIneligible;
        }

        // Set inability to castle on one side.
        if (affects_castle_state != CastlingEligible::EitherSideEligible
            && ableToCastle (player, affects_castle_state))
        {
            removeCastlingEligibility (player, affects_castle_state);
        }
    }

    void Board::updateEnPassantEligibility (Color who, ColoredPiece src_piece, Move move)
    {
        ColorIndex c_index = colorIndex (who);
        ColorIndex o_index = colorIndex (colorInvert (who));

        int direction = pawnDirection<int> (who);
        Coord new_state = No_En_Passant_Coord;
        if (isDoubleSquarePawnMove (src_piece, move))
        {
            Coord src = move.getSrc();
            int prev_row = nextRow (Row<int> (src), direction);
            new_state = makeCoord (prev_row, Column (src));
        }
        setEnPassantTarget (c_index, new_state);
    }

    [[nodiscard]] auto Board::withMove (Color who, Move move) const -> Board
    {
        Board result = *this;
        result.makeMove (who, move);
        return result;
    }

    void Board::makeMove (Color who, Move move)
    {
        assert (who == my_code.currentTurn());

        Coord src = move.getSrc();
        Coord dst = move.getDst();

        auto src_piece = pieceAt (src);
        auto orig_src_piece = src_piece;
        auto dst_piece = pieceAt (dst);

        assert (pieceType (src_piece) != Piece::None);
        assert (pieceColor (src_piece) == who);
        if (pieceType (dst_piece) != Piece::None)
            assert (move.isNormalCapturing());

        if (pieceType (dst_piece) != Piece::None)
            assert (pieceColor (src_piece) != pieceColor (dst_piece));

        // check for promotion
        if (move.isPromoting())
        {
            src_piece = move.getPromotedPiece();
            my_material.add (src_piece);
            my_material.remove (ColoredPiece::make (who, Piece::Pawn));
        }

        switch (move.getMoveCategory())
        {
            case MoveCategory::Default:
                assert (pieceType (dst_piece) == Piece::None);
                break;

            case MoveCategory::NormalCapturing:
                assert (move.isNormalCapturing());
                assert (pieceColor (src_piece) != pieceColor (dst_piece));
                break;

            case MoveCategory::EnPassant:
                dst_piece = applyForEnPassant (who, src, dst);
                break;

            case MoveCategory::Castling:
                applyForCastlingMove (who, move, src, dst);
                break;

            default:
                throw Error {
                    "Invalid move category: " + std::to_string (static_cast<int>(move.getMoveCategory()))
                };
        }

        updateEnPassantEligibility (who, src_piece, move);

        my_code.applyMove (*this, move);

        setPiece (src, Piece_And_Color_None);
        setPiece (dst, src_piece);

        // update king position
        if (pieceType (src_piece) == Piece::King)
            applyForKingMove (who, src, dst);

        // update rook position -- for castling
        if (pieceType (orig_src_piece) == Piece::Rook)
        {
            applyForRookMove (who, orig_src_piece, move, src, dst);
        }

        if (pieceType (dst_piece) != Piece::None)
        {
            // update material estimate
            my_material.remove (dst_piece);

            auto captured_piece_type = pieceType (dst_piece);

            // update castle state if somebody takes the rook
            if (captured_piece_type == Piece::Rook)
                applyForRookCapture (colorInvert (who), dst_piece, src, dst);
        }

        my_position.applyMove (who, orig_src_piece, move, dst_piece);

        updateMoveClock (who, pieceType (orig_src_piece), move);
        setCurrentTurn (colorInvert (who));
    }

    static auto castleParse (const string& str, Color who) -> optional<Move>
    {
        int src_row, dst_col;

        if (who == Color::White)
            src_row = Last_Row;
        else if (who == Color::Black)
            src_row = First_Row;
        else
            throw ParseMoveException { "Invalid color parsing castling move." };

        string transformed { str };
        std::transform (transformed.begin (), transformed.end (), transformed.begin (),
                        [] (auto c) -> auto
                        {
                            return ::toupper (c);
                        });

        if (transformed == "O-O-O")
            dst_col = King_Column - 2;
        else if (transformed == "O-O")
            dst_col = King_Column + 2;
        else
            return nullopt;

        return Move::makeCastling (src_row, King_Column, src_row, dst_col);
    }

    auto moveParseOptional (const string& str, Color who) -> optional<Move>
    {
        bool en_passant = false;
        bool is_capturing = false;

        // allow any number of spaces/tabs before the two coordinates
        string tmp { str };

        if (tmp.empty ())
            return nullopt;

        tmp.erase (std::remove_if (tmp.begin (), tmp.end (), isspace), tmp.end ());
        std::transform (tmp.begin (), tmp.end (), tmp.begin (),
                        [] (auto c) -> auto
                        {
                            return ::toupper (c);
                        });

        if (tmp.empty ())
            return nullopt;

        if (tolower (tmp[0]) == 'o')
            return castleParse (tmp, who);

        if (tmp.size () < 4)
            return nullopt;

        optional<Coord> src;
        int offset = 0;
        try
        {
            src = coordParse (tmp.substr (0, 2));
            offset += 2;
        }
        catch ([[maybe_unused]] const CoordParseError& e)
        {
            return nullopt;
        }

        // allow an 'x' between coordinates, which is used to indicate a capture
        if (tmp[offset] == 'X')
        {
            offset++;
            is_capturing = true;
        }

        string dst_coord { tmp.substr (offset, 2) };
        offset += 2;
        if (dst_coord.empty ())
            return nullopt;

        optional<Coord> dst;
        try
        {
            dst = coordParse (dst_coord);
        }
        catch ([[maybe_unused]] const CoordParseError& e)
        {
            return nullopt;
        }

        string rest { tmp.substr (offset) };
        Move move = Move::make (*src, *dst);
        if (is_capturing)
        {
            move = move.withCapture();
        }

        // grab extra identifiers describing the move
        ColoredPiece promoted = ColoredPiece::make (Color::None, Piece::None);
        if (rest == "EP")
        {
            en_passant = true;
        }
        else if (rest == "(Q)")
        {
            promoted = ColoredPiece::make (who, Piece::Queen);
        }
        else if (rest == "(N)")
        {
            promoted = ColoredPiece::make (who, Piece::Knight);
        }
        else if (rest == "(B)")
        {
            promoted = ColoredPiece::make (who, Piece::Bishop);
        }
        else if (rest == "(R)")
        {
            promoted = ColoredPiece::make (who, Piece::Rook);
        }

        if (pieceType (promoted) != Piece::None)
        {
            move = move.withPromotion (promoted);
        }

        if (en_passant)
        {
            move = Move::makeEnPassant (Row (*src), Column (*src), Row (*dst), Column (*dst));
        }

        return move;
    }

    auto moveParse (const string& str, Color color) -> Move
    {
        if (tolower (str[0]) == 'o' && color == Color::None)
            throw ParseMoveException ("Move requires color, but no color provided");

        auto optional_result = moveParseOptional (str, color);
        if (!optional_result.has_value ())
            throw ParseMoveException ("Error parsing move: " + str);

        auto result = *optional_result;
        auto move_category = result.getMoveCategory();
        if (color == Color::None && move_category != MoveCategory::NormalCapturing
            && move_category != MoveCategory::Default)
        {
            throw ParseMoveException ("Invalid type of move in parse_simple_move");
        }

        return result;
    }

    string asString (const Move& move)
    {
        Coord src = move.getSrc();
        Coord dst = move.getDst();

        if (move.isCastling())
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

        string result;
        result += asString (src);

        if (move.isNormalCapturing())
            result += "x";
        else
            result += " ";

        result += asString (dst);

        if (move.isEnPassant())
        {
            result += " ep";
        }

        if (move.isPromoting())
        {
            string promoted_piece { pieceChar (move.getPromotedPiece()) };
            result += "(" + promoted_piece + ")";
        }

        return result;
    }

    auto mapCoordinatesToMove (const Board& board, Color who, Coord src, Coord dst,
                               optional<Piece> promoted_piece) -> optional<Move>
    {
        ColoredPiece src_piece = board.pieceAt (src);
        ColoredPiece dst_piece = board.pieceAt (dst);

        if (src_piece == Piece_And_Color_None)
            return {};

        if (pieceColor (src_piece) != who)
            return {};

        // make capturing if dst piece is not none
        Move move = Move::make (src, dst);
        if (dst_piece != Piece_And_Color_None)
            move = move.withCapture();

        // check for pawn special moves.
        switch (pieceType (src_piece))
        {
            case Piece::Pawn:
                // look for en passant:
                if (pieceType (src_piece) == Piece::Pawn)
                {
                    int eligible_column
                        = eligibleEnPassantColumn (board, Row (src), Column (src), who);
                    if (eligible_column == Column (dst))
                        return Move::makeEnPassant (src, dst);

                    if (needPawnPromotion (Row<int> (dst), who) && promoted_piece.has_value ())
                        return move.withPromotion (ColoredPiece::make (who, *promoted_piece));
                }
                break;

            // look for castling
            case Piece::King:
                if (Column (src) == King_Column)
                {
                    if (Column (dst) - Column (src) == 2 || Column (dst) - Column (src) == -2)
                    {
                        return Move::makeCastling (src, dst);
                    }
                }
                break;

            default:
                break;
        }

        return move;
    }

    auto operator<< (std::ostream& os, const Move& value) -> std::ostream&
    {
        os << asString (value);
        return os;
    }
}
