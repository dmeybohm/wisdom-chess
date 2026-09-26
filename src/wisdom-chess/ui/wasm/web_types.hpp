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
    enum class WebColor
    {
        NoColor,
        White,
        Black
    };

    [[nodiscard]] inline auto
    mapColor (WebColor color)
        -> wisdom::Color
    {
        switch (color)
        {
            case WebColor::NoColor:
                return Color::None;
            case WebColor::White:
                return Color::White;
            case WebColor::Black:
                return Color::Black;
            default:
                throw Error { "Invalid color." };
        }
    }

    [[nodiscard]] inline auto
    mapColor (int color)
        -> wisdom::Color
    {
        return mapColor (static_cast<WebColor> (color));
    }

    [[nodiscard]] inline auto 
    mapColor (wisdom::Color color)
        -> WebColor
    {
        using enum Color;
        switch (color)
        {
            case None:
                return WebColor::NoColor;
            case White:
                return WebColor::White;
            case Black:
                return WebColor::Black;
            default:
                throw Error { "Invalid color." };
        }
    }

    enum class WebPiece
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
    mapPiece (WebPiece piece)
        -> wisdom::Piece
    {
        switch (piece)
        {
            case WebPiece::NoPiece:
                return Piece::None;
            case WebPiece::Pawn:
                return Piece::Pawn;
            case WebPiece::Knight:
                return Piece::Knight;
            case WebPiece::Bishop:
                return Piece::Bishop;
            case WebPiece::Rook:
                return Piece::Rook;
            case WebPiece::Queen:
                return Piece::Queen;
            case WebPiece::King:
                return Piece::King;
            default:
                throw Error { "Invalid piece." };
        }
    }

    [[nodiscard]] inline auto
    mapPiece (int piece)
        -> wisdom::Piece
    {
        return mapPiece (static_cast<WebPiece> (piece));
    }

    [[nodiscard]] inline auto 
    mapPiece (wisdom::Piece piece)
        -> WebPiece
    {
        using enum Piece;
        switch (piece)
        {
            case None:
                return WebPiece::NoPiece;
            case Pawn:
                return WebPiece::Pawn;
            case Knight:
                return WebPiece::Knight;
            case Bishop:
                return WebPiece::Bishop;
            case Rook:
                return WebPiece::Rook;
            case Queen:
                return WebPiece::Queen;
            case King:
                return WebPiece::King;
            default:
                throw Error { "Invalid piece." };
        }
    }

    enum class WebPlayer
    {
        Human,
        ChessEngine
    };

    [[nodiscard]] inline auto
    mapPlayer (WebPlayer player)
        -> wisdom::Player
    {
        switch (player)
        {
            case WebPlayer::Human:
                return Player::Human;
            case WebPlayer::ChessEngine:
                return Player::ChessEngine;
            default:
                throw Error { "Invalid player." };
        }
    }

    [[nodiscard]] inline auto
    mapPlayer (int player)
        -> wisdom::Player
    {
        return mapPlayer (static_cast<WebPlayer> (player));
    }

    [[nodiscard]] inline auto 
    mapPlayer (wisdom::Player player)
        -> WebPlayer
    {
        using enum Player;
        switch (player)
        {
            case Human:
                return WebPlayer::Human;
            case ChessEngine:
                return WebPlayer::ChessEngine;
            default:
                throw Error { "Invalid player." };
        }
    }

    enum class WebGameStatus
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
    mapGameStatus (WebGameStatus status)
        -> wisdom::GameStatus
    {
        switch (status)
        {
            case WebGameStatus::Playing:
                return GameStatus::Playing;
            case WebGameStatus::Checkmate:
                return GameStatus::Checkmate;
            case WebGameStatus::Stalemate:
                return GameStatus::Stalemate;
            case WebGameStatus::ThreefoldRepetitionReached:
                return GameStatus::ThreefoldRepetitionReached;
            case WebGameStatus::ThreefoldRepetitionAccepted:
                return GameStatus::ThreefoldRepetitionAccepted;
            case WebGameStatus::FivefoldRepetitionDraw:
                return GameStatus::FivefoldRepetitionDraw;
            case WebGameStatus::FiftyMovesWithoutProgressReached:
                return GameStatus::FiftyMovesWithoutProgressReached;
            case WebGameStatus::FiftyMovesWithoutProgressAccepted:
                return GameStatus::FiftyMovesWithoutProgressAccepted;
            case WebGameStatus::SeventyFiveMovesWithoutProgressDraw:
                return GameStatus::SeventyFiveMovesWithoutProgressDraw;
            case WebGameStatus::InsufficientMaterialDraw:
                return GameStatus::InsufficientMaterialDraw;
            default:
                throw Error { "Invalid game status" };
        }
    }

    [[nodiscard]] inline auto
    mapGameStatus (int status)
        -> wisdom::GameStatus
    {
        return mapGameStatus (static_cast<WebGameStatus> (status));
    }

    [[nodiscard]] inline auto 
    mapGameStatus (GameStatus status)
        -> WebGameStatus
    {
        using enum GameStatus;
        switch (status)
        {
            case Playing:
                return WebGameStatus::Playing;
            case Checkmate:
                return WebGameStatus::Checkmate;
            case Stalemate:
                return WebGameStatus::Stalemate;
            case ThreefoldRepetitionReached:
                return WebGameStatus::ThreefoldRepetitionReached;
            case ThreefoldRepetitionAccepted:
                return WebGameStatus::ThreefoldRepetitionAccepted;
            case FivefoldRepetitionDraw:
                return WebGameStatus::FivefoldRepetitionDraw;
            case FiftyMovesWithoutProgressReached:
                return WebGameStatus::FiftyMovesWithoutProgressReached;
            case FiftyMovesWithoutProgressAccepted:
                return WebGameStatus::FiftyMovesWithoutProgressAccepted;
            case SeventyFiveMovesWithoutProgressDraw:
                return WebGameStatus::SeventyFiveMovesWithoutProgressDraw;
            case InsufficientMaterialDraw:
                return WebGameStatus::InsufficientMaterialDraw;
            default:
                throw Error { "Invalid game status" };
        }
    }

    struct WebColoredPiece
    {
        WebColoredPiece() : id { 0 }, color { WebColor::NoColor }, piece { WebPiece::NoPiece }, row { 0 }, col { 0 }
        {
        }

        WebColoredPiece (int id_, WebColor color_, WebPiece piece_, int row_, int col_) :
                id { id_ }, color { color_ }, piece { piece_ }, row { row_ }, col { col_ }
        {
        }

        int id;
        WebColor color;
        WebPiece piece;
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

    enum class WebDrawStatus
    {
        NotReached,
        Proposed,
        Accepted,
        Declined,
    };

    enum class WebDrawByRepetitionType
    {
        ThreefoldRepetition,
        FiftyMovesWithoutProgress,
    };

    [[nodiscard]] inline auto
    mapDrawByRepetitionType (WebDrawByRepetitionType type)
        -> wisdom::ProposedDrawType
    {
        switch (type)
        {
            case WebDrawByRepetitionType::ThreefoldRepetition:
                return ProposedDrawType::ThreeFoldRepetition;
            case WebDrawByRepetitionType::FiftyMovesWithoutProgress:
                return ProposedDrawType::FiftyMovesWithoutProgress;
            default:
                throw Error { "Invalid draw type." };
        }
    }

    [[nodiscard]] inline auto
    mapDrawByRepetitionType (int type)
        -> wisdom::ProposedDrawType
    {
        return mapDrawByRepetitionType (static_cast<WebDrawByRepetitionType> (type));
    }

    [[nodiscard]] inline auto 
    mapDrawByRepetitionType (wisdom::ProposedDrawType type)
        -> WebDrawByRepetitionType
    {
        using enum ProposedDrawType;
        switch (type)
        {
            case ThreeFoldRepetition:
                return WebDrawByRepetitionType::ThreefoldRepetition;
            case FiftyMovesWithoutProgress:
                return WebDrawByRepetitionType::FiftyMovesWithoutProgress;
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

