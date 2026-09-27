#include "wisdom-chess/ui/viewmodel/viewmodel_types.hpp"

namespace wisdom::ui
{
    auto
    getFirstHumanPlayerColor (const Players& players)
        -> std::optional<Color>
    {
        if (players[0] == Player::Human)
        {
            return Color::White;
        }
        if (players[1] == Player::Human)
        {
            return Color::Black;
        }

        return {};
    }

    auto
    transitionGameStatus (GameStatusUpdate& update, const Game& game)
        -> GameStatus
    {
        update.update (game.status());
        return game.status();
    }

    void negotiateDraw (
        nonnull_observer_ptr<Game> game,
        ProposedDrawType draw_type,
        Color who,
        const DrawAnswerCallback& answered
    ) {
        expects (isColorValid (who));

        for (auto player : { who, colorInvert (who) })
        {
            if (game->getPlayer (player) != Player::ChessEngine)
                continue;

            bool accepted = game->computerWantsDraw (player);
            game->setProposedDrawStatus (draw_type, player, accepted);
            answered (player, accepted);
        }
    }
}
