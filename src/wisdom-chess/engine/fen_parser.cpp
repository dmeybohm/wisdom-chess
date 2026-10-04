#include <sstream>

#include "wisdom-chess/engine/fen_parser.hpp"
#include "wisdom-chess/engine/str.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/coord.hpp"
#include "wisdom-chess/engine/game.hpp"

namespace wisdom
{
    using string_size_t = string::size_type;

    namespace
    {
        [[nodiscard]] auto
        parseError (string message)
            -> unexpected<ParseError>
        {
            return unexpected<ParseError> { ParseError { std::move (message) } };
        }
    }

    FenParser::FenParser (const string& input)
    {
        auto result = parseFields (input);
        if (!result.has_value())
        {
            terminateOnCheckFailure (
                "Precondition",
                "a valid FEN string: " + result.error().message,
                std::source_location::current()
            );
        }
    }

    auto
    FenParser::parse (const string& input)
        -> expected<FenParser, ParseError>
    {
        FenParser parser;
        auto result = parser.parseFields (input);
        if (!result.has_value())
            return unexpected<ParseError> { result.error() };

        return parser;
    }

    Game FenParser::build()
    {
        my_builder.setCurrentTurn (my_active_player);
        return Game::createGameFromBoard (my_builder);
    }

    auto
    FenParser::parsePiece (char ch)
        -> expected<ColoredPiece, ParseError>
    {
        char lower = toLower (ch);
        Color who = isLower (ch) ? Color::Black : Color::White;

        switch (lower)
        {
            case 'k':
                return ColoredPiece::make (who, Piece::King);
            case 'q':
                return ColoredPiece::make (who, Piece::Queen);
            case 'r':
                return ColoredPiece::make (who, Piece::Rook);
            case 'b':
                return ColoredPiece::make (who, Piece::Bishop);
            case 'n':
                return ColoredPiece::make (who, Piece::Knight);
            case 'p':
                return ColoredPiece::make (who, Piece::Pawn);
            default:
                return parseError ("Invalid piece type!");
        }
    }

    auto
    FenParser::parseActivePlayer (char ch)
        -> expected<Color, ParseError>
    {
        switch (ch)
        {
            case 'w':
                return Color::White;
            case 'b':
                return Color::Black;
            default:
                return parseError ("Invalid active color!");
        }
    }

    auto
    FenParser::parsePieces (string pieces_str)
        -> Result
    {
        // read pieces
        for (int row = 0, col = 0; !pieces_str.empty(); pieces_str = pieces_str.substr (1))
        {
            char ch = pieces_str[0];

            if (ch == '/')
            {
                row++;
                if (row >= Num_Rows)
                    return parseError ("Invalid row!");
                col = 0;
            }
            else if (ch == ' ')
            {
                break;
            }
            else if (isAlpha (ch))
            {
                if (col >= Num_Columns)
                    return parseError ("Invalid columns!");
                auto piece = parsePiece (ch);
                if (!piece.has_value())
                    return unexpected<ParseError> { piece.error() };
                my_builder.addPiece (row, col, pieceColor (*piece), pieceType (*piece));
                col++;
            }
            else if (isDigit (ch))
            {
                col += ch - '0';
                if (col > Num_Columns)
                    return parseError ("Invalid columns!");
            }
            else
            {
                return parseError ("Invalid character!");
            }
        }

        if (!my_builder.hasKingPositions())
            return parseError ("Each side needs a king!");

        return {};
    }

    // en passant target square:
    auto
    FenParser::parseEnPassant (string en_passant_str)
        -> Result
    {
        if (en_passant_str.empty())
            return {};

        if (en_passant_str[0] == '-')
            return {};

        string cstr { en_passant_str.substr (0, 2) };
        auto target = parseCoord (cstr);
        if (!target.has_value())
            return parseError ("Error parsing en passant coordinate: Invalid coordinate!");

        Color vulnerable_color = colorInvert (my_active_player);
        if (auto valid = validateEnPassantTarget (vulnerable_color, *target); !valid.has_value())
            return valid;

        my_builder.setEnPassantTarget (vulnerable_color, cstr);
        return {};
    }

    // Move generation trusts that an en passant target was left by a pawn
    // that just moved two squares, and never re-checks the squares itself.
    auto
    FenParser::validateEnPassantTarget (Color vulnerable_color, Coord target)
        -> Result
    {
        int target_row = vulnerable_color == Color::White
            ? White_En_Passant_Row
            : Black_En_Passant_Row;
        if (target.row<int>() != target_row)
            return parseError ("En passant target is on the wrong rank for the side to move!");

        int direction = pawnDirection<int> (vulnerable_color);
        int column = target.column<int>();
        auto pawn = my_builder.pieceAt (makeCoord (nextRow (target_row, direction), column));
        if (pawn != ColoredPiece::make (vulnerable_color, Piece::Pawn))
            return parseError ("En passant target requires a pawn that just moved two squares!");

        auto crossed = my_builder.pieceAt (target);
        auto origin = my_builder.pieceAt (makeCoord (nextRow (target_row, -direction), column));
        if (crossed != Piece_And_Color_None || origin != Piece_And_Color_None)
            return parseError ("En passant target requires empty squares behind the pawn!");

        return {};
    }

    auto
    FenParser::parseCastling (string castling_str)
        -> Result
    {
        CastlingEligibility white_castle = CastlingEligibility::Neither_Side;
        CastlingEligibility black_castle = CastlingEligibility::Neither_Side;

        if (castling_str != "-")
        {
            for (char ch : castling_str)
            {
                switch (ch)
                {
                    case 'K':
                        white_castle |= CastlingRights::Kingside;
                        break;
                    case 'Q':
                        white_castle |= CastlingRights::Queenside;
                        break;
                    case 'k':
                        black_castle |= CastlingRights::Kingside;
                        break;
                    case 'q':
                        black_castle |= CastlingRights::Queenside;
                        break;
                    default:
                        return parseError ("Invalid castling character!");
                }
            }
        }

        if (auto valid = validateCastlingPieces (Color::White, white_castle); !valid.has_value())
            return valid;
        if (auto valid = validateCastlingPieces (Color::Black, black_castle); !valid.has_value())
            return valid;

        my_builder.setCastling (Color::White, white_castle);
        my_builder.setCastling (Color::Black, black_castle);
        return {};
    }

    // Move generation trusts that a castling-eligibility bit is only set
    // when the king and the corresponding rook still sit on their home
    // squares; it never re-checks the squares itself before applying a
    // castling move.
    auto
    FenParser::validateCastlingPieces (Color who, CastlingEligibility eligibility)
        -> Result
    {
        auto row = castlingRowForColor<int> (who);

        auto has_rook_at = [&] (int col)
        {
            auto piece = my_builder.pieceAt (makeCoord (row, col));
            return pieceType (piece) == Piece::Rook && pieceColor (piece) == who;
        };

        auto king = my_builder.pieceAt (makeCoord (row, King_Column));
        if (eligibility != CastlingEligibility::Neither_Side
            && king != ColoredPiece::make (who, Piece::King))
        {
            return parseError ("Castling rights require the king on its home square!");
        }

        if (eligibility.canCastleKingside() && !has_rook_at (King_Rook_Column))
            return parseError ("Castling rights require a rook on its home square!");

        if (eligibility.canCastleQueenside() && !has_rook_at (Queen_Rook_Column))
            return parseError ("Castling rights require a rook on its home square!");

        return {};
    }

    // halfmove clock:
    auto
    FenParser::parseHalfMove (int half_moves)
        -> Result
    {
        if (half_moves < 0 || half_moves > Max_Half_Move_Clock)
            return parseError ("Half move clock out of range parsing FEN string");

        my_builder.setHalfMovesClock (half_moves);
        return {};
    }

    // fullmove number:
    auto
    FenParser::parseFullMove (int full_moves)
        -> Result
    {
        if (full_moves < 0 || full_moves > Max_Full_Move_Number)
            return parseError ("Full move number out of range parsing FEN string");

        // The number starts at 1, but some programs write 0.
        my_builder.setFullMoves (full_moves == 0 ? 1 : full_moves);
        return {};
    }

    auto
    FenParser::parseFields (const string& source)
        -> Result
    {
        std::stringstream input { source };

        // Read the pieces:
        string pieces_str;
        input >> pieces_str;
        if (input.fail())
            return parseError ("Missing pieces declaration parsing FEN string");
        if (auto result = parsePieces (pieces_str); !result.has_value())
            return result;

        // read active computer_player:
        string active_player_str;
        input >> active_player_str;
        if (input.fail())
            return parseError ("Missing active player parsing FEN string");
        auto active_player = parseActivePlayer (active_player_str[0]);
        if (!active_player.has_value())
            return unexpected<ParseError> { active_player.error() };
        my_active_player = *active_player;

        // castling:
        string castling_str;
        input >> castling_str;
        if (input.fail())
            return parseError ("Missing castling string FEN string");
        if (auto result = parseCastling (castling_str); !result.has_value())
            return result;

        // en passant target square:
        string en_passant_str;
        input >> en_passant_str;
        if (input.fail())
            return parseError ("Missing en passant square parsing FEN string");
        if (auto result = parseEnPassant (en_passant_str); !result.has_value())
            return result;

        // halfmove clock:
        int half_moves;
        input >> half_moves;
        if (input.fail())
            return parseError ("Missing half move number parsing FEN string");
        if (auto result = parseHalfMove (half_moves); !result.has_value())
            return result;

        // fullmove number:
        int full_moves;
        input >> full_moves;
        if (input.fail())
            return parseError ("Missing full move number parsing FEN string");
        return parseFullMove (full_moves);
    }

    auto
    FenParser::buildBoard()
        -> Board
    {
        my_builder.setCurrentTurn (my_active_player);
        return Board { my_builder };
    }
}
