#pragma once

#include <functional>
#include <optional>

#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/game_status.hpp"

namespace wisdom::ui
{
    enum class DrawByRepetitionStatus
    {
        NotReached,
        Proposed,
        Accepted,
        Declined
    };

    // Frontend search depths count full moves; the engine counts plies.
    [[nodiscard]] constexpr auto
    fullMovesToPlyDepth (int full_moves)
        -> int
    {
        return full_moves * 2;
    }

    [[nodiscard]] auto
    getFirstHumanPlayerColor (const Players& players)
        -> std::optional<Color>;

    // Runs the status update for the game's current status and returns
    // that status, so a frontend can act on what the update did.
    auto
    transitionGameStatus (GameStatusUpdate& update, const Game& game)
        -> GameStatus;

    // Answers a draw proposal for the engine players: the side to move,
    // when it is an engine, and then its opponent, when that is one too.
    // Each answer is recorded in the game and handed to the callback, which
    // carries it to whoever else must hear it.
    using DrawAnswerCallback = std::function<void (Color who, bool accepted)>;

    void negotiateDraw (
        nonnull_observer_ptr<Game> game,
        ProposedDrawType draw_type,
        Color who,
        const DrawAnswerCallback& answered
    );
}
