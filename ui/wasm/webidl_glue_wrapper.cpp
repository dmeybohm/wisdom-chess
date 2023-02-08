#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <iostream>

#include "game.hpp"

extern void console_log(const char *str);

extern emscripten_wasm_worker_t engine_thread_manager;
extern emscripten_wasm_worker_t engine_thread;

namespace wisdom
{
  enum WebColor
  {
    NoColor,
    White,
    Black
  };

  auto map_color (int color) -> wisdom::Color
  {
    switch (static_cast<WebColor>(color))
    {
      case NoColor: return Color::None;
      case White: return Color::White;
      case Black: return Color::Black;
    default:
      throw new Error { "Invalid color." };
    }
  }

  enum WebPiece
  {
    NoPiece,
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,
  };

  auto map_piece (int piece) -> wisdom::Piece
  {
    switch (static_cast<WebPiece>(piece))
    {
      case NoPiece: return Piece::None;
      case Knight: return Piece::Knight;
      case Bishop: return Piece::Bishop;
      case Rook: return Piece::Rook;
      case Queen: return Piece::Queen;
      case King: return Piece::King;
      throw new Error { "Invalid piec." };
    }
  }

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
      WebGame (int white_player, int black_player)
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

      void start_worker ()
      {
            std::cout << "Inside start_worker (a bit of a misnomer now?)\n";
            emscripten_wasm_worker_post_function_v (engine_thread_manager, run_in_worker);
            std::cout << "Exiting start_worker\n";
      }

  private:
      Game my_game;
  };
}

// Map enums to the global namespace:
using wisdom_WebPlayer = wisdom::WebPlayer;
using wisdom_WebPiece = wisdom::WebPiece;
using wisdom_WebColor = wisdom::WebColor;

#include "glue.hpp"
