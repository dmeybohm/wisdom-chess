#ifndef WISDOMCHESS_WEB_TYPES_HPP
#define WISDOMCHESS_WEB_TYPES_HPP

#include <emscripten.h>
#include <emscripten/wasm_worker.h>

#include "bindings.hpp"
#include "coord.hpp"
#include "game.hpp"
#include "check.hpp"

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

    [[nodiscard]] inline auto map_color (int color) -> wisdom::Color
    {
        switch (static_cast<WebColor> (color))
        {
            case NoColor:
                return Color::None;
            case White:
                return Color::White;
            case Black:
                return Color::Black;
            default:
                throw Error { "Invalid color." };
        }
    }

    [[nodiscard]] inline auto map_color (wisdom::Color color) -> WebColor
    {
        switch (color)
        {
            case Color::None:
                return NoColor;
            case Color::White:
                return White;
            case Color::Black:
                return Black;
            default:
                throw Error { "Invalid color." };
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

    [[nodiscard]] inline auto map_piece (int piece) -> wisdom::Piece
    {
        switch (static_cast<WebPiece> (piece))
        {
            case NoPiece:
                return Piece::None;
            case Pawn:
                return Piece::Pawn;
            case Knight:
                return Piece::Knight;
            case Bishop:
                return Piece::Bishop;
            case Rook:
                return Piece::Rook;
            case Queen:
                return Piece::Queen;
            case King:
                return Piece::King;
            default:
                throw Error { "Invalid piece." };
        }
    }

    [[nodiscard]] inline auto map_piece (wisdom::Piece piece) -> WebPiece
    {
        switch (piece)
        {
            case Piece::None:
                return NoPiece;
            case Piece::Pawn:
                return Pawn;
            case Piece::Knight:
                return Knight;
            case Piece::Bishop:
                return Bishop;
            case Piece::Rook:
                return Rook;
            case Piece::Queen:
                return Queen;
            case Piece::King:
                return King;
            default:
                throw Error { "Invalid piece." };
        }
    }

    enum WebPlayer
    {
        Human,
        ChessEngine
    };

    [[nodiscard]] inline auto map_player (int player) -> wisdom::Player
    {
        switch (static_cast<WebPlayer> (player))
        {
            case Human:
                return Player::Human;
            case ChessEngine:
                return Player::ChessEngine;
            default:
                throw Error { "Invalid player." };
        }
    }

    [[nodiscard]] inline auto map_player (wisdom::Player player) -> WebPlayer
    {
        switch (player)
        {
            case Player::Human:
                return Human;
            case Player::ChessEngine:
                return ChessEngine;
            default:
                throw Error { "Invalid player." };
        }
    }

    enum WebGameStatus
    {
        Playing,
        Checkmate,
        Stalemate,
        ThreefoldRepetitionReached,
        ThreefoldRepetitionAccepted,
        FivefoldRepetitionDraw,
        FiftyMovesWithoutProgressReached,
        FiftyMovesWithoutProgressAccepted,
        SeventyFiveMovesWithoutProgressDraw,
        InsufficientMaterialDraw,
    };

    [[nodiscard]] inline auto map_game_status (int status) -> wisdom::GameStatus
    {
        switch (static_cast<WebGameStatus> (status))
        {
            case Playing:
                return GameStatus::Playing;
            case Checkmate:
                return GameStatus::Checkmate;
            case Stalemate:
                return GameStatus::Stalemate;
            case ThreefoldRepetitionReached:
                return GameStatus::ThreefoldRepetitionReached;
            case ThreefoldRepetitionAccepted:
                return GameStatus::ThreefoldRepetitionAccepted;
            case FivefoldRepetitionDraw:
                return GameStatus::FivefoldRepetitionDraw;
            case FiftyMovesWithoutProgressReached:
                return GameStatus::FiftyMovesWithoutProgressReached;
            case FiftyMovesWithoutProgressAccepted:
                return GameStatus::FiftyMovesWithoutProgressAccepted;
            case SeventyFiveMovesWithoutProgressDraw:
                return GameStatus::SeventyFiveMovesWithoutProgressDraw;
            case InsufficientMaterialDraw:
                return GameStatus::InsufficientMaterialDraw;
        }
    }

    struct WebColoredPiece
    {
        WebColoredPiece() : id { 0 }, color { 0 }, piece { 0 }, row { 0 }, col { 0 }
        {
        }

        WebColoredPiece (int id_, int color_, int piece_, int row_, int col_) :
                id { id_ }, color { color_ }, piece { piece_ }, row { row_ }, col { col_ }
        {
        }

        int id;
        int color;
        int piece;
        int row;
        int col;
    };

    [[nodiscard]] inline auto map_colored_piece (WebColoredPiece colored_piece) -> ColoredPiece
    {
        auto mapped_color = map_color (colored_piece.color);
        auto mapped_type = map_piece (colored_piece.piece);
        return ColoredPiece::make (mapped_color, mapped_type);
    }

    struct WebColoredPieceList
    {
        WebColoredPieceList()
        {
            clear();
        }

        WebColoredPiece pieces[Num_Squares] {};
        int length = 0;

        void addPiece (WebColoredPiece piece)
        {
            pieces[length++] = piece;
        }

        auto pieceAt (int index) -> WebColoredPiece
        {
            return pieces[index];
        }

        void clear()
        {
            for (int i = 0; i < Num_Squares; i++)
            {
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

        WebCoord() : row { 0 }, col { 0 }
        {
        }

        WebCoord (int row_, int col_) : row { row_ }, col { col_ }
        {
        }

        static auto fromTextCoord (char* coord_text) -> WebCoord*
        {
            auto coord = coord_parse (coord_text);
            return new WebCoord { gsl::narrow<int> (Row (coord)),
                                  gsl::narrow<int> (Column (coord)) };
        }
    };

    enum WebDrawStatus
    {
        NotReached,
        Proposed,
        Accepted,
        Declined,
    };
}

// Map enums to the global namespace:
using wisdom_WebPlayer = wisdom::WebPlayer;
using wisdom_WebPiece = wisdom::WebPiece;
using wisdom_WebColor = wisdom::WebColor;
using wisdom_WebGameStatus = wisdom::WebGameStatus;
using wisdom_WebDrawStatus = wisdom::WebDrawStatus;

#endif // WISDOMCHESS_WEB_TYPES_HPP
