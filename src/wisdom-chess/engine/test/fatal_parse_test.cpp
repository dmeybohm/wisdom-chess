#include "wisdom-chess/engine/coord.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"
#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/move_list.hpp"

#include "fatal_test.hpp"

using namespace wisdom;

namespace
{
    FATAL_CASE(
        "coord-parse-invalid",
        "Precondition failed at .*coord\\.hpp:[0-9]+: result\\.has_value\\(\\)"
    )
    {
        [[maybe_unused]] Coord coord = toCoord ("z9");
    }

    FATAL_CASE(
        "move-parse-invalid",
        "Precondition failed at .*move\\.cpp:[0-9]+: optional_result\\.has_value\\(\\)"
    )
    {
        [[maybe_unused]] Move move = toMove ("invalid");
    }

    FATAL_CASE(
        "move-parse-castling-without-color",
        "Precondition failed at .*move\\.cpp:[0-9]+: optional_result\\.has_value\\(\\)"
    )
    {
        [[maybe_unused]] Move move = toMove ("o-o");
    }

    FATAL_CASE(
        "move-parse-en-passant-without-color",
        "Precondition failed at .*move\\.cpp:[0-9]+: color != Color::None"
    )
    {
        [[maybe_unused]] Move move = toMove ("e5 d6 ep");
    }

    FATAL_CASE(
        "move-list-invalid-move",
        "Precondition failed at .*move\\.cpp:[0-9]+: optional_result\\.has_value\\(\\)"
    )
    {
        [[maybe_unused]] MoveList list { Color::White, { "e2 e4", "not a move" } };
    }

    FATAL_CASE(
        "fen-parser-invalid",
        "Precondition failed at .*fen_parser\\.cpp:[0-9]+: a valid FEN string: Invalid piece type!"
    )
    {
        [[maybe_unused]] FenParser parser { "not a fen" };
    }

    FATAL_CASE(
        "create-game-from-invalid-fen",
        "Precondition failed at .*fen_parser\\.cpp:[0-9]+: a valid FEN string: Invalid piece type!"
    )
    {
        [[maybe_unused]] Game game = Game::createGameFromFen ("not a fen");
    }
}
