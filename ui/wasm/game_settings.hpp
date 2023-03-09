#ifndef WISDOMCHESS_GAME_SETTINGS_HPP
#define WISDOMCHESS_GAME_SETTINGS_HPP

#include "web_types.hpp"
#include "game.hpp"

namespace wisdom
{
    struct GameSettings
    {
        GameSettings() = default;

        GameSettings (int whitePlayer, int blackPlayer, int thinkingTime_, int searchDepth_)
            : whitePlayer { map_player (map_player (whitePlayer)) }
            , blackPlayer { map_player (map_player (blackPlayer)) }
            , thinkingTime { thinkingTime_ }
            , searchDepth { searchDepth_ }
        {}

        GameSettings (enum WebPlayer whitePlayer_, enum WebPlayer blackPlayer_, int thinkingTime_, int searchDepth_)
            : whitePlayer { whitePlayer_ }
            , blackPlayer { blackPlayer_ }
            , thinkingTime { thinkingTime_ }
            , searchDepth { searchDepth_ }
        {}

        enum WebPlayer whitePlayer = Human;
        enum WebPlayer blackPlayer = ChessEngine;

#if NDEBUG
        int thinkingTime = 2;
#else
        int thinkingTime = 4;
#endif

        int searchDepth = 3;


        [[nodiscard]] static auto map_human_depth_to_computer_depth (int human_depth) -> int
        {
            return human_depth * 2 - 1;
        }

        void apply_to_game (observer_ptr<wisdom::Game> game) const
        {
            game->set_search_timeout (std::chrono::seconds { thinkingTime });
            game->set_max_depth (GameSettings::map_human_depth_to_computer_depth (searchDepth));
            game->set_players ({ map_player (whitePlayer), map_player (blackPlayer) });
        }
    };
};

#endif // WISDOMCHESS_GAME_SETTINGS_HPP
