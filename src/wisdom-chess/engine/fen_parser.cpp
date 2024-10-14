#include <sstream>

#include "wisdom-chess/engine/fen_parser.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/game.hpp"

namespace wisdom
{
    using string_size_t = string::size_type;

    Game FenParser::build()
    {
        builder.setCurrentTurn (active_player);
        return Game { builder };
    }

    auto 
    FenParser::parsePiece (char ch) 
        -> ColoredPiece
    {
        int lower = tolower (ch);
        Color who = islower (ch) ? Color::Black : Color::White;

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
                throw FenParserError ("Invalid piece type!");
        }
    }

    auto 
    FenParser::parseActivePlayer (char ch) 
        -> Color
    {
        switch (ch)
        {
            case 'w':
                return Color::White;
            case 'b':
                return Color::Black;
            default:
                throw FenParserError ("Invalid active color!");
        }
    }

    void FenParser::parsePieces (string pieces_str)
    {
        char ch;

        // read pieces
        for (int row = 0, col = 0; !pieces_str.empty(); pieces_str = pieces_str.substr (1))
        {
            ch = pieces_str[0];

            if (ch == '/')
            {
                row++;
                if (row > Num_Rows)
                    throw FenParserError ("Invalid row!");
                col = 0;
            }
            else if (ch == ' ')
            {
                break;
            }
            else if (isalpha (ch))
            {
                ColoredPiece piece = parsePiece (ch);
                builder.addPiece (row, col, pieceColor (piece), pieceType (piece));
                col++;
                if (col > Num_Columns)
                    throw FenParserError ("Invalid columns!");
            }
            else if (isdigit (ch))
            {
                col += ch - '0';
                if (col > Num_Columns)
                    throw FenParserError ("Invalid columns!");
            }
            else
            {
                throw FenParserError ("Invalid character!");
            }
        }
    }

    // en passant target square:
    void FenParser::parseEnPassant (string en_passant_str)
    {
        if (en_passant_str.empty())
            return;

        if (en_passant_str[0] == '-')
            return;

        try
        {
            string cstr { en_passant_str.substr (0, 2) };
            builder.setEnPassantTarget (colorInvert (active_player), cstr);
        }
        catch ([[maybe_unused]] const BoardBuilderError& e)
        {
            throw FenParserError ("Error parsing en passant coordinate!");
        }
    }

    void FenParser::parseCastling (string castling_str)
    {
        CastlingEligibility white_castle = Neither_Side_Eligible;
        CastlingEligibility black_castle = Neither_Side_Eligible;

        for (; !castling_str.empty() && isalpha (castling_str[0]);
             castling_str = castling_str.substr (1))
        {
            char ch = castling_str[0];

            if (ch == ' ' || ch == '-')
                break;

            Color who = islower (ch) ? Color::Black : Color::White;
            CastlingEligibility new_ineligible = Either_Side_Eligible;

            switch (tolower (ch))
            {
                case 'k':
                    new_ineligible ^= CastlingIneligible::Kingside;
                    break;
                case 'q':
                    new_ineligible ^= CastlingIneligible::Queenside;
                    break;
            }

            if (who == Color::Black)
                black_castle ^= type_safe::combo<CastlingIneligible> (new_ineligible);
            else
                white_castle ^= type_safe::combo<CastlingIneligible> (new_ineligible);
        }

        builder.setCastling (Color::White, white_castle);
        builder.setCastling (Color::Black, black_castle);
    }

    // halfmove clock:
    void FenParser::parseHalfMove (int half_moves)
    {
        builder.setHalfMovesClock (half_moves);
    }

    // fullmove number:
    void FenParser::parseFullMove (int full_moves)
    {
        builder.setFullMoves (full_moves);
    }

    void FenParser::parse (const string& source)
    {
        std::stringstream input { source };

        // Read the pieces:
        string pieces_str;
        input >> pieces_str;
        if (input.fail())
            throw FenParserError { "Missing pieces declaration parsing FEN string" };
        parsePieces (pieces_str);

        // read active computer_player:
        string active_player_str;
        input >> active_player_str;
        if (input.fail())
            throw FenParserError { "Missing active player parsing FEN string" };
        active_player = parseActivePlayer (active_player_str[0]);

        // castling:
        string castling_str;
        input >> castling_str;
        if (input.fail())
            throw FenParserError { "Missing castling string FEN string" };
        parseCastling (castling_str);

        // en passant target square:
        string en_passant_str;
        input >> en_passant_str;
        if (input.fail())
            throw FenParserError { "Missing en passant square parsing FEN string" };
        parseEnPassant (en_passant_str);

        // halfmove clock:
        int half_moves;
        input >> half_moves;
        if (input.fail())
            throw FenParserError { "Missing half move number parsing FEN string" };
        parseHalfMove (half_moves);

        // fullmove number:
        int full_moves;
        input >> full_moves;
        if (input.fail())
            throw FenParserError { "Missing full move number parsing FEN string" };
        parseFullMove (full_moves);
    }

    auto 
    FenParser::buildBoard() 
        -> Board
    {
        return Board { builder };
    }
}
