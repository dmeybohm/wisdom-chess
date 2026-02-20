#pragma once

#include <emscripten.h>
#include <emscripten/wasm_worker.h>

#include "wisdom-chess/engine/coord.hpp"
#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/evaluate.hpp"

#include "wisdom-chess/ui/wasm/bindings.hpp"

extern emscripten_wasm_worker_t engine_thread;

namespace wisdom
{
    enum WebColor
    {
        NoColor,
        White,
        Black
    };

    [[nodiscard]] inline auto 
    mapColor (int color) 
        -> wisdom::Color
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

    [[nodiscard]] inline auto 
    mapColor (wisdom::Color color)
        -> WebColor
    {
        using enum Color;
        switch (color)
        {
            case None:
                return NoColor;
            case White:
                return ::wisdom::White;
            case Black:
                return ::wisdom::Black;
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

    [[nodiscard]] inline auto 
    mapPiece (int piece) 
        -> wisdom::Piece
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

    [[nodiscard]] inline auto 
    mapPiece (wisdom::Piece piece)
        -> WebPiece
    {
        using enum Piece;
        switch (piece)
        {
            case None:
                return NoPiece;
            case Pawn:
                return ::wisdom::Pawn;
            case Knight:
                return ::wisdom::Knight;
            case Bishop:
                return ::wisdom::Bishop;
            case Rook:
                return ::wisdom::Rook;
            case Queen:
                return ::wisdom::Queen;
            case King:
                return ::wisdom::King;
            default:
                throw Error { "Invalid piece." };
        }
    }

    enum WebPlayer
    {
        Human,
        ChessEngine
    };

    [[nodiscard]] inline auto 
    mapPlayer (int player) 
        -> wisdom::Player
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

    [[nodiscard]] inline auto 
    mapPlayer (wisdom::Player player)
        -> WebPlayer
    {
        using enum Player;
        switch (player)
        {
            case Human:
                return ::wisdom::Human;
            case ChessEngine:
                return ::wisdom::ChessEngine;
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

    [[nodiscard]] inline auto 
    mapGameStatus (int status) 
        -> wisdom::GameStatus
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
            default:
                throw Error { "Invalid game status" };
        }
    }

    [[nodiscard]] inline auto 
    mapGameStatus (GameStatus status)
        -> WebGameStatus
    {
        using enum GameStatus;
        switch (status)
        {
            case Playing:
                return ::wisdom::Playing;
            case Checkmate:
                return ::wisdom::Checkmate;
            case Stalemate:
                return ::wisdom::Stalemate;
            case ThreefoldRepetitionReached:
                return ::wisdom::ThreefoldRepetitionReached;
            case ThreefoldRepetitionAccepted:
                return ::wisdom::ThreefoldRepetitionAccepted;
            case FivefoldRepetitionDraw:
                return ::wisdom::FivefoldRepetitionDraw;
            case FiftyMovesWithoutProgressReached:
                return ::wisdom::FiftyMovesWithoutProgressReached;
            case FiftyMovesWithoutProgressAccepted:
                return ::wisdom::FiftyMovesWithoutProgressAccepted;
            case SeventyFiveMovesWithoutProgressDraw:
                return ::wisdom::SeventyFiveMovesWithoutProgressDraw;
            case InsufficientMaterialDraw:
                return ::wisdom::InsufficientMaterialDraw;
            default:
                throw Error { "Invalid game status" };
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

    [[nodiscard]] inline auto 
    mapColoredPiece (WebColoredPiece colored_piece) 
        -> ColoredPiece
    {
        auto mapped_color = mapColor (colored_piece.color);
        auto mapped_type = mapPiece (colored_piece.piece);
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

        auto 
        pieceAt (int index) 
            -> WebColoredPiece
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

        [[nodiscard]] static auto 
        fromTextCoord (char* coord_text) 
            -> WebCoord*
        {
            auto coord = coordParse (coord_text);
            return new WebCoord { 
                narrow<int> (coord.row()), 
                narrow<int> (coord.column()) 
            };
        }
    };

    enum WebDrawStatus
    {
        NotReached,
        Proposed,
        Accepted,
        Declined,
    };

    enum WebDrawByRepetitionType
    {
        ThreeFoldRepeition,
        FiftyMovesWithoutProgress,
    };

    [[nodiscard]] inline auto 
    mapDrawByRepetitionType (int type) 
        -> wisdom::ProposedDrawType
    {
        switch (static_cast<WebDrawByRepetitionType> (type))
        {
            case ThreeFoldRepeition:
                return ProposedDrawType::ThreeFoldRepetition;
            case FiftyMovesWithoutProgress:
                return ProposedDrawType::FiftyMovesWithoutProgress;
            default:
                throw Error { "Invalid draw type." };
        }
    }

    [[nodiscard]] inline auto 
    mapDrawByRepetitionType (wisdom::ProposedDrawType type)
        -> WebDrawByRepetitionType
    {
        using enum ProposedDrawType;
        switch (type)
        {
            case ThreeFoldRepetition:
                return ThreeFoldRepeition;
            case FiftyMovesWithoutProgress:
                return ::wisdom::FiftyMovesWithoutProgress;
            default:
                throw Error { "Invalid draw type." };
        }
    }


}

// Map enums to the global namespace:
using wisdom_WebPlayer = wisdom::WebPlayer;
using wisdom_WebPiece = wisdom::WebPiece;
using wisdom_WebColor = wisdom::WebColor;
using wisdom_WebGameStatus = wisdom::WebGameStatus;
using wisdom_WebDrawStatus = wisdom::WebDrawStatus;
using wisdom_WebDrawByRepetitionType = wisdom::WebDrawByRepetitionType;

