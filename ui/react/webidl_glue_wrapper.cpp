#include <iostream>

#include "game.hpp"

namespace wisdom
{
  enum WebPlayer
  {
      Human,
      ChessEngine
  };

  auto map_player (int player) -> wisdom::Player
  {
    switch (static_cast<WebPlayer>(player))
    {
        case Human:
            return Player::Human;
        case ChessEngine:
            return Player::ChessEngine;
        default:
            throw new Error { "Invalid player." };
    };
  }

  class WebGame
  {
  public:
      WebGame(int white_player, int black_player)
          : my_game {
                  map_player (white_player),
                  map_player (black_player)
            }
      {}

      void set_max_depth (int max_depth)
      {
            std::cout << "Setting max depth: " << max_depth << "\n";
            my_game.set_max_depth (max_depth);
      }
      auto get_max_depth () -> int
      {
            auto result = my_game.get_max_depth ();
            std::cout << "Getting max depth: " << result << "\n";
            return result;
      }


  private:
      Game my_game;
  };
}

// Map the enum to the global namespace:
using wisdom_WebPlayer = wisdom::WebPlayer;

#include "glue.hpp"
