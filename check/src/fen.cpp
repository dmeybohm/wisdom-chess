
#include "fen.hpp"
#include "board.h"
#include "game.h"

using string_size_t = std::string::size_type;

std::unique_ptr<game> fen::build ()
{
    auto game = std::make_unique<struct game> (
            active_player,
            color_invert(active_player)
    );

    // todo pass the new board into the game constructor instead.
    board_free (game->board);
    game->board = builder.build();

    return game;
}

piece_t fen::parse_piece (char ch)
{
    int lower = tolower(ch);
    enum color who = islower(ch) ? COLOR_BLACK : COLOR_WHITE;

    switch (lower)
    {
        case 'k':
            return MAKE_PIECE (who, PIECE_KING);
        case 'q':
            return MAKE_PIECE (who, PIECE_QUEEN);
        case 'r':
            return MAKE_PIECE (who, PIECE_ROOK);
        case 'b':
            return MAKE_PIECE (who, PIECE_BISHOP);
        case 'n':
            return MAKE_PIECE (who, PIECE_KNIGHT);
        case 'p':
            return MAKE_PIECE (who, PIECE_PAWN);
        default:
            throw fen_exception("Invalid piece type!");
    }
}

enum color fen::parse_active_player (char ch)
{
    switch (ch)
    {
        case 'w': return COLOR_WHITE;
        case 'b': return COLOR_BLACK;
        default: throw fen_exception ("Invalid active color!");
    }
}

void fen::parse_pieces (std::string_view str)
{
    char ch;

    // read pieces
    for (uint8_t row = 0, col = 0; !str.empty(); str = str.substr(1))
    {
        ch = str[0];

        if (ch == '/')
        {
            row++;
            if (row > NR_ROWS)
                throw fen_exception("Invalid row!");
            col = 0;
        }
        else if (ch == ' ')
        {
            break;
        }
        else if (isalpha(ch))
        {
            piece_t piece = parse_piece (ch);
            builder.add_piece (row, col, PIECE_COLOR(piece), PIECE_TYPE(piece));
            col++;
            if (col > NR_COLUMNS)
                throw fen_exception("Invalid columns!");
        }
        else if (isdigit(ch))
        {
            col += ch - '0';
            if (col > NR_COLUMNS)
                throw fen_exception("Invalid columns!");
        }
        else
        {
            throw fen_exception("Invalid character!");
        }
    }

}

// en passant target square:
void fen::parse_en_passant (std::string_view str)
{
    if (str.empty())
        return;

    if (str[0] == '-')
        return;

    try {
        std::string cstr { str.substr(0, 2) };
        builder.set_en_passant_target (color_invert(active_player), cstr.c_str());
    } catch (const board_builder_exception &e) {
        throw fen_exception("Error parsing en passant coordinate!");
    }
}

void fen::parse_castling (std::string_view str)
{
    castle_state_t white_castle = CASTLE_QUEENSIDE | CASTLE_KINGSIDE;
    castle_state_t black_castle = CASTLE_QUEENSIDE | CASTLE_KINGSIDE;

    for (;!str.empty() && isalpha(str[0]); str = str.substr(1))
    {
        char ch = str[0];

        if (ch == ' ' || ch == '-')
            break;

        enum color who = islower(ch) ? COLOR_BLACK : COLOR_WHITE;
        castle_state_t new_state = CASTLE_QUEENSIDE | CASTLE_KINGSIDE;

        switch (tolower(ch))
        {
            case 'k':
                new_state &= ~CASTLE_KINGSIDE;
                break;
            case 'q':
                new_state &= ~CASTLE_QUEENSIDE;
                break;
        }

        if (who == COLOR_BLACK)
            black_castle &= new_state;
        else
            white_castle &= new_state;

    }

    builder.set_castling (COLOR_WHITE, white_castle);
    builder.set_castling (COLOR_BLACK, black_castle);
}

// halfmove clock:
void fen::parse_halfmove (int half_moves)
{
    builder.set_half_moves (half_moves);
}

// fullmove number:
void fen::parse_fullmove (int full_moves)
{
    builder.set_full_moves (full_moves);
}

void fen::parse (const std::string &source)
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