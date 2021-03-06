#include "fen_parser.hpp"
#include "board.hpp"
#include "game.hpp"

#include <sstream>

namespace wisdom
{
    using string_size_t = string::size_type;

    Game FenParser::build ()
    {
        builder.set_current_turn (active_player);
        return Game { builder };
    }

    auto FenParser::parse_piece (char ch) -> ColoredPiece
    {
        int lower = tolower (ch);
        Color who = islower (ch) ? Color::Black : Color::White;

        switch (lower)
        {
            case 'k':
                return make_piece (who, Piece::King);
            case 'q':
                return make_piece (who, Piece::Queen);
            case 'r':
                return make_piece (who, Piece::Rook);
            case 'b':
                return make_piece (who, Piece::Bishop);
            case 'n':
                return make_piece (who, Piece::Knight);
            case 'p':
                return make_piece (who, Piece::Pawn);
            default:
                throw FenParserError ("Invalid piece type!");
        }
    }

    auto FenParser::parse_active_player (char ch) -> Color
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

    void FenParser::parse_pieces (string str)
    {
        char ch;

        // read pieces
        for (int row = 0, col = 0; !str.empty (); str = str.substr (1))
        {
            ch = str[0];

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
                ColoredPiece piece = parse_piece (ch);
                builder.add_piece (row, col, piece_color (piece), piece_type (piece));
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
    void FenParser::parse_en_passant (string str)
    {
        if (str.empty ())
            return;

        if (str[0] == '-')
            return;

        try
        {
            string cstr { str.substr (0, 2) };
            builder.set_en_passant_target (color_invert (active_player), cstr);
        }
        catch ([[maybe_unused]] const BoardBuilderError& e)
        {
            throw FenParserError ("Error parsing en passant coordinate!");
        }
    }

    void FenParser::parse_castling (string str)
    {
        CastlingState white_castle = Castle_Queenside | Castle_Kingside;
        CastlingState black_castle = Castle_Queenside | Castle_Kingside;

        for (; !str.empty () && isalpha (str[0]); str = str.substr (1))
        {
            char ch = str[0];

            if (ch == ' ' || ch == '-')
                break;

            Color who = islower (ch) ? Color::Black : Color::White;
            CastlingState new_state = Castle_Queenside | Castle_Kingside;

            switch (tolower (ch))
            {
                case 'k':
                    new_state &= ~Castle_Kingside;
                    break;
                case 'q':
                    new_state &= ~Castle_Queenside;
                    break;
            }

            if (who == Color::Black)
                black_castle &= new_state;
            else
                white_castle &= new_state;
        }

        builder.set_castling (Color::White, white_castle);
        builder.set_castling (Color::Black, black_castle);
    }

    // halfmove clock:
    void FenParser::parse_half_move (int half_moves)
    {
        builder.set_half_moves (half_moves);
    }

    // fullmove number:
    void FenParser::parse_full_move (int full_moves)
    {
        builder.set_full_moves (full_moves);
    }

    void FenParser::parse (const string& source)
    {
        std::stringstream input { source };

        // Read the pieces:
        string pieces_str;
        input >> pieces_str;
        if (input.fail ())
            throw FenParserError { "Missing pieces declaration parsing FEN string" };
        parse_pieces (pieces_str);

        // read active computer_player:
        string active_player_str;
        input >> active_player_str;
        if (input.fail ())
            throw FenParserError { "Missing active player parsing FEN string" };
        active_player = parse_active_player (active_player_str[0]);

        // castling:
        string castling_str;
        input >> castling_str;
        if (input.fail ())
            throw FenParserError { "Missing castling string FEN string" };
        parse_castling (castling_str);

        // en passant target square:
        string en_passant_str;
        input >> en_passant_str;
        if (input.fail ())
            throw FenParserError { "Missing en passant square parsing FEN string" };
        parse_en_passant (en_passant_str);

        // halfmove clock:
        int half_moves;
        input >> half_moves;
        if (input.fail ())
            throw FenParserError { "Missing half move number parsing FEN string" };
        parse_half_move (half_moves);

        // fullmove number:
        int full_moves;
        input >> full_moves;
        if (input.fail ())
            throw FenParserError { "Missing full move number parsing FEN string" };
        parse_full_move (full_moves);
    }

    auto FenParser::build_board () -> Board
    {
        Board board = *builder.build ();
        return board;
    }
}