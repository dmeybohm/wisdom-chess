
#include "fen.hpp"
#include "board.h"
#include "game.h"

using string_size_t = std::string::size_type;

struct game *fen::build ()
{
    struct game *game = game_new (active_player, color_invert(active_player));

    board_free (game->board);
    game->board = builder.build();
//    game->board->half_move_clock = half_move_clock;
//    game->board->full_moves = full_moves;

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

// en passant target square:
std::string_view fen::parse_en_passant (std::string_view str)
{

}

std::string_view fen::parse_castling (std::string_view str)
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
std::string_view fen::parse_halfmove (std::string_view str)
{

}

// fullmove number:
std::string_view fen::parse_fullmove (std::string_view str)
{

}

void fen::parse (const std::string &source)
{
    uint8_t row = 0, col = 0;
    std::string_view str { source };

    // read pieces
    for (; str.size() > 0; str = str.substr(1))
    {
        char c = str[0];

        if (c == '/')
        {
            row++;
            if (row > NR_ROWS)
                throw fen_exception("Invalid row!");
            col = 0;
        }
        else if (c == ' ')
        {
            break;
        }
        else if (isalpha(c))
        {
            piece_t piece = parse_piece (c);
            builder.add_piece (row, col, PIECE_COLOR(piece), PIECE_TYPE(piece));
            col++;
            if (col > NR_COLUMNS)
                throw fen_exception("Invalid columns!");
        }
        else if (isdigit(c))
        {
            col += c - '0';
            if (col > NR_COLUMNS)
                throw fen_exception("Invalid columns!");
        }
        else
        {
            throw fen_exception("Invalid character!");
        }
    }

    // skip space:
    str = str.substr (1);

    // read active player:
    active_player = parse_active_player (str[0]);
    str = str.substr (2);

    // castling:
    str = parse_castling (str);

    // en passant target square:
    str = parse_en_passant (str);

    // halfmove clock:
    str = parse_halfmove (str);

    // fullmove number:
    str = parse_fullmove (str);
}

