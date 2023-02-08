#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <iostream>

#include "game.hpp"

extern "C"
{

  EM_JS(void, console_log, (const char* str), {
    console.log(UTF8ToString(str))
  })

  EMSCRIPTEN_KEEPALIVE void run_in_worker ()
  {
    console_log ("Hello from a worker!\n");
  }
}

// The engine thread manager receives messages from the main thread
emscripten_wasm_worker_t engine_thread_manager;
emscripten_wasm_worker_t engine_thread;

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
            std::cout << "what? test build\n";
            emscripten_wasm_worker_post_function_v (engine_thread_manager, run_in_worker);
            std::cout << "Exiting start_worker\n";
      }

  private:
      Game my_game;
  };
}

// Map the enum to the global namespace:
using wisdom_WebPlayer = wisdom::WebPlayer;

int main()
{
  // Initialize the worker threads:
  engine_thread_manager = emscripten_malloc_wasm_worker (/*stack size: */1024);
  engine_thread = emscripten_malloc_wasm_worker (/*stack size: */1024);
}

#include "glue.hpp"
