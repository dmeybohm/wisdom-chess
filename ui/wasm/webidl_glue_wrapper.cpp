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
      case Pawn: return Piece::Pawn;
      case Knight: return Piece::Knight;
      case Bishop: return Piece::Bishop;
      case Rook: return Piece::Rook;
      case Queen: return Piece::Queen;
      case King: return Piece::King;
      default:
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

  struct WebColoredPiece
  {
    WebColoredPiece() : color{0}, piece{0} {}
    WebColoredPiece(int color_, int piece_)
        : color { color_ }, piece { piece_ }
    {}

    long color;
    long piece;
  };

  struct WebColoredPieceList
  {
    WebColoredPieceList()
    {
     clear();
    }

    WebColoredPiece pieces[Num_Squares] {};
    int length = 0;

    void add_piece(WebColoredPiece piece)
    {
      pieces[length++] = piece;
    }

    void clear()
    {
      for (int i = 0; i < Num_Squares; i++) {
        pieces[i].color = WebColor::NoColor;
        pieces[i].piece = WebPiece::NoPiece;
      }
      length = 0;
    }

  };

  class WebGame
  {
  public:
      WebGame (int white_player, int black_player)
          : my_game {
                  map_player (white_player),
                  map_player (black_player)
            }
      {
        update_piece_list();
      }

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

      observer_ptr<WebColoredPieceList> get_piece_list ()
      {
            return &my_pieces;
      }

  private:
      Game my_game;
      WebColoredPieceList my_pieces;

      void update_piece_list ()
      {
        const Board& board = my_game.get_board();
        my_pieces.clear();
        for (int i = 0; i < Num_Squares; i++) {
          ColoredPiece piece = board.piece_at(make_coord_from_index(i));
          if (piece != Piece_And_Color_None) {
            my_pieces.add_piece(WebColoredPiece {
                to_int(piece.color()),
                to_int(piece.type())
            });
          }
        }
      }
  };
}

// Map enums to the global namespace:
using wisdom_WebPlayer = wisdom::WebPlayer;
using wisdom_WebPiece = wisdom::WebPiece;
using wisdom_WebColor = wisdom::WebColor;

#include "glue.hpp"
