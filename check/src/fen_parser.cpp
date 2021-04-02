#include "fen_parser.hpp"
#include "board.hpp"
#include "game.hpp"

namespace wisdom
{
    using string_size_t = std::string::size_type;

    Game FenParser::build ()
    {
        struct Game result {
                active_player,
                color_invert (active_player),
                builder
        };

        return result;
    }

    ColoredPiece FenParser::parse_piece (char ch)
    {
        int lower = tolower (ch);
        Color who = islower (ch) ? Color::Black : Color::White;

        switch (lower)
        {
            case 'k': return make_piece (who, Piece::King);
            case 'q': return make_piece (who, Piece::Queen);
            case 'r': return make_piece (who, Piece::Rook);
            case 'b': return make_piece (who, Piece::Bishop);
            case 'n': return make_piece (who, Piece::Knight);
            case 'p': return make_piece (who, Piece::Pawn);
            default: throw FenParserError ("Invalid piece type!");
        }
    }

    Color FenParser::parse_active_player (char ch)
    {
        switch (ch)
        {
            case 'w': return Color::White;
            case 'b': return Color::Black;
            default: throw FenParserError ("Invalid active color!");
        }
    }

    void FenParser::parse_pieces (std::string str)
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
    void FenParser::parse_en_passant (std::string str)
    {
        if (str.empty ())
            return;

        if (str[0] == '-')
            return;

        try
        {
            std::string cstr { str.substr (0, 2) };
            builder.set_en_passant_target (color_invert (active_player), cstr.c_str ());
        } catch ([[maybe_unused]] const BoardBuilderError &e)
        {
            throw FenParserError ("Error parsing en passant coordinate!");
        }
    }

    void FenParser::parse_castling (std::string str)
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
                case 'k':new_state &= ~Castle_Kingside;
                    break;
                case 'q':new_state &= ~Castle_Queenside;
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
    void FenParser::parse_halfmove (int half_moves)
    {
        builder.set_half_moves (half_moves);
    }

    // fullmove number:
    void FenParser::parse_fullmove (int full_moves)
    {
        builder.set_full_moves (full_moves);
    }

    void FenParser::parse (const std::string &source)
    {
        std::stringstream input { source };

        // Read the pieces:
        std::string pieces_str;
        input >> pieces_str;
        parse_pieces (pieces_str);

        // read active player:
        std::string active_player_str;
        input >> active_player_str;
        active_player = parse_active_player (active_player_str[0]);

        // castling:
        std::string castling_str;
        input >> castling_str;
        parse_castling (castling_str);

        // en passant target square:
        std::string en_passant_str;
        input >> en_passant_str;
        parse_en_passant (en_passant_str);

        // halfmove clock:
        int half_moves;
        input >> half_moves;
        parse_halfmove (half_moves);

        // fullmove number:
        int full_moves;
        input >> full_moves;
        parse_fullmove (full_moves);
    }
}