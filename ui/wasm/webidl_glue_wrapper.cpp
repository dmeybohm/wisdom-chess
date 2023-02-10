#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <iostream>

#include "game.hpp"
#include "coord.hpp"

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
    WebColoredPiece() : id { 0 }, color { 0 }, piece { 0 }, row { 0 }, col { 0 }
    {}

    WebColoredPiece(int id_, int color_, int piece_, int row_, int col_)
        : id { id_ }, color { color_ }, piece { piece_ }, row { row_ },
          col { col_ }
    {}

    int id;
    int color;
    int piece;
    int row;
    int col;
  };

  auto map_colored_piece (WebColoredPiece colored_piece) -> ColoredPiece
  {
    auto mapped_color = map_color (colored_piece.color);
    auto mapped_type = map_piece (colored_piece.piece);
    return ColoredPiece::make (mapped_color, mapped_type);
  }

  struct WebColoredPieceList
  {
    WebColoredPieceList ()
    {
     clear();
    }

    WebColoredPiece pieces[Num_Squares] {};
    int length = 0;

    void addPiece (WebColoredPiece piece)
    {
      pieces[length++] = piece;
    }

    auto pieceAt(int index) -> WebColoredPiece
    {
      return pieces[index];
    }

    void clear()
    {
      for (int i = 0; i < Num_Squares; i++) {
        pieces[i].id = 0;
        pieces[i].color = WebColor::NoColor;
        pieces[i].piece = WebPiece::NoPiece;
        pieces[i].row = 0;
        pieces[i].col = 0;
      }
      length = 0;
    }

  };

  struct WebCoord
  {
    int row;
    int col;

    WebCoord () : row { 0 }, col { 0 }
    {}

    WebCoord (int row_, int col_) : row { row_ }, col { col_ }
    {}

    static auto fromTextCoord(char *coord_text) -> WebCoord
    {
      auto coord = coord_parse (coord_text);
      return WebCoord {
        gsl::narrow<int>(Row(coord)), gsl::narrow<int>(Column(coord))
      };
    }
  };

  class WebGame
  {
  private:
    Game my_game;
    WebColoredPieceList my_pieces;

  public:
      WebGame (int white_player, int black_player)
          : my_game {
                  map_player (white_player),
                  map_player (black_player)
            }
      {
        const auto& board = my_game.get_board();
        int id = 1;
        for (int i = 0; i < Num_Squares; i++) {
          auto coord = make_coord_from_index (i);
          auto piece = board.piece_at (coord);
          if (piece != Piece_And_Color_None) {
            WebColoredPiece new_piece = WebColoredPiece {
                id,
                to_int (piece.color()),
                to_int (piece.type()),
                gsl::narrow<int> (Row (coord)),
                gsl::narrow<int> (Column (coord))
            };
            my_pieces.addPiece(new_piece);
            id++;
          }
        }
        std::cout << "Finished initializing piece list: " << my_pieces.length;
      }

      auto makeMove (const WebCoord& src, const WebCoord& dst) -> bool
      {
        auto game_src = make_coord (src.row, src.col);
        auto game_dst = make_coord (dst.row, dst.col);

        auto who = my_game.get_current_turn();
        auto optionalMove = my_game.map_coordinates_to_move (game_src, game_dst, std::nullopt);
        if (!optionalMove.has_value()) {
          return false;
        }
        auto move = *optionalMove;
        if (!isLegalMove(move)) {
//          setMoveStatus("Illegal move");
          return false;
        }
        my_game.move (move);
        return true;
        //auto newColor = updateChessEngineForHumanMove(move);
        //updateDisplayedGameState();
        //updateCurrentTurn(newColor);
        //handleMove(wisdom::Player::Human, move, who);
      }

      auto isLegalMove(Move selectedMove) -> bool
      {
        auto selectedMoveStr = to_string (selectedMove);

        // If it's not the human's turn, move is illegal.
        if (my_game.get_current_player() != wisdom::Player::Human) {
          return false;
        }

        auto who = my_game.get_current_turn();
        auto generator = my_game.get_move_generator();
        auto legalMoves = generator->generate_legal_moves(my_game.get_board(), who);

        return std::any_of(legalMoves.cbegin(), legalMoves.cend(),
                           [selectedMove](const auto& move){
                             return move == selectedMove;
                           });
      }

      void setMaxDepth (int max_depth)
      {
            std::cout << "Setting max depth: " << max_depth << "\n";
            my_game.set_max_depth (max_depth);
      }

      auto getMaxDepth () -> int
      {
            auto result = my_game.get_max_depth ();
            std::cout << "Getting max depth: " << result << "\n";
            return result;
      }

      void startWorker ()
      {
            std::cout << "Inside start_worker (a bit of a misnomer now?)\n";
            emscripten_wasm_worker_post_function_v (engine_thread_manager, run_in_worker);
            std::cout << "Exiting start_worker\n";
      }

      auto getPieceList () -> WebColoredPieceList&
      {
            return my_pieces;
      }

      auto find_and_remove_id (std::unordered_map<int, WebColoredPiece>& list,
                              Coord coord_to_find, ColoredPiece piece_to_find) -> int
      {
            auto found = std::find_if (list.begin (), list.end (),
                         [piece_to_find, coord_to_find](const auto& it) -> bool {
              auto key = it.first;
              auto value = it.second;
              auto piece = map_colored_piece (value);
              auto piece_coord = make_coord (value.row, value.col);
              return piece_to_find == piece && piece_coord == coord_to_find;
            });

            if (found != list.end ()) {
              auto position = found->first;
              auto value = found->second;
              return value.id;
            }

            // find first match by row/column. Otherwise, find first match by
            // piece / color.
            return 0;
      }

      void update_piece_list ()
      {
        const Board& board = my_game.get_board ();
        WebColoredPieceList old_pieces = my_pieces;
        std::unordered_map<int, WebColoredPiece> list {};
        std::unordered_map<int, ColoredPiece> deferred {};

        // Index the old pieces to be able to find the old ids:
        for (int i = 0; i < old_pieces.length; i++) {
          WebColoredPiece piece = old_pieces.pieces[i];
          Coord src = make_coord (piece.row, piece.col);
          list[coord_index (src)] = piece;
        }

        for (int i = 0; i < Num_Squares; i++) {
          Coord coord = make_coord_from_index (i);
          ColoredPiece piece = board.piece_at (coord);
          if (piece != Piece_And_Color_None) {
            int id = find_and_remove_id (list, coord, piece);
            if (id != 0) {
              WebColoredPiece new_piece = {
                  id,
                  to_int(piece.color()),
                  to_int(piece.type()),
                  gsl::narrow<int8_t>(Row(coord)),
                  gsl::narrow<int8_t>(Column(coord)),
              };
              my_pieces.addPiece(new_piece);
            }
          }
        }

        if (!deferred.empty ()) {
          for (auto& value : deferred) {
            auto colored_piece = value.second;
            auto pred = [colored_piece](const auto& list_item) -> bool {
              ColoredPiece value = map_colored_piece (list_item.second);
              return value == colored_piece;
            };
            auto it = std::find_if (list.begin (), list.end (), pred);
            if (it != list.end ()) {
              throw new Error { "Couldn't find id." };
            }
            auto coord_idx = it->first;
            auto old_piece = it->second;
            auto coord = make_coord_from_index (coord_idx);

            WebColoredPiece new_piece = {
                old_piece.id,
                old_piece.color,
                old_piece.piece,
                gsl::narrow<int8_t> (Row (coord)),
                gsl::narrow<int8_t> (Column (coord)),
            };
            my_pieces.addPiece(new_piece);
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
