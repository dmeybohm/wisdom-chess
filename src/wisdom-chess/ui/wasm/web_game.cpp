#include <iostream>

#include "wisdom-chess/engine/piece.hpp"

#include "wisdom-chess/ui/wasm/web_game.hpp"
#include "wisdom-chess/ui/wasm/game_settings.hpp"

namespace wisdom
{
    WebGame::WebGame (int white_player, int black_player, int game_id)
        : my_game {
            Game::createGame (
                mapPlayer (white_player),
                mapPlayer (black_player)
            )
        }
        , my_game_id { game_id }
    {
        const auto& board = my_game.getBoard();
        int id = 1;

        for (int i = 0; i < Num_Squares; i++)
        {
            auto coord = Coord::fromIndex (i);
            auto piece = board.pieceAt (coord);
            if (piece != Piece_And_Color_None)
            {
                WebColoredPiece new_piece
                    = WebColoredPiece { id, toInt (piece.color()), toInt (piece.type()),
                                        narrow<int> (coord.row()),
                                        narrow<int> (coord.column()) };
                my_pieces.addPiece (new_piece);
                id++;
            }
        }

        updateWebDisplayedGameState();
    }

    auto
    WebGame::makeMove (const WebMove* move_param)
        -> bool
    {
        Move move = move_param->getMove();

        my_game.move (move);

        updatePieceList (move.getPromotedPiece());
        updateWebDisplayedGameState();

        return true;
    }

    auto
    WebGame::newFromSettings (const GameSettings& settings, int game_id)
        -> wisdom::WebGame*
    {
        auto* new_game = new WebGame (settings.whitePlayer, settings.blackPlayer, game_id);

        const auto computer_depth = GameSettings::mapHumanDepthToComputerDepth (settings.searchDepth);
        new_game->setMaxDepth (computer_depth);
        new_game->setThinkingTime (std::chrono::seconds { settings.thinkingTime });

        return new_game;
    }

    auto
    WebGame::createMoveFromCoordinatesAndPromotedPiece (
        const WebCoord* src,
        const WebCoord* dst,
        int promoted_piece_type
    )
        -> WebMove*
    {
        auto game_src = makeCoord (src->row, src->col);
        auto game_dst = makeCoord (dst->row, dst->col);

        auto optionalMove = my_game.mapCoordinatesToMove (game_src, game_dst, mapPiece (promoted_piece_type));

        if (!optionalMove.has_value())
        {
            throw new Error { "Failed to map move." };
        }

        auto move = *optionalMove;
        return new WebMove { move };
    }

    auto
    WebGame::isLegalMove (const WebMove* selectedMovePtr)
        -> bool
    {
        Move selectedMove = selectedMovePtr->getMove();

        // If it's not the human's turn, move is illegal.
        if (my_game.getCurrentPlayer() != wisdom::Player::Human)
        {
            setMoveStatus ("Illegal move");
            return false;
        }

        bool isLegal = GameViewModelBase::isLegalMove (selectedMove);
        if (!isLegal)
        {
            setMoveStatus ("Illegal move");
        }
        return isLegal;
    }

    void WebGame::setSettings (const wisdom::GameSettings& settings)
    {
        settings.applyToGame (&my_game);
    }

    void WebGame::setComputerDrawStatus (int type, int who, bool accepted)
    {
        ProposedDrawType proposed_draw_type = mapDrawByRepetitionType (type);
        Color color = mapColor (who);

        std::cout << "accepted: " << accepted << "\n";
        std::cout << "Color: " << wisdom::asString (color) << "\n";
        my_game.setProposedDrawStatus (proposed_draw_type, color, accepted);
        updateWebDisplayedGameState();
    }

    [[nodiscard]] auto
    WebGame::findAndRemoveId (
        std::unordered_map<int,
        WebColoredPiece>& old_list,
        Coord coord_to_find,
        ColoredPiece piece_to_find
    )
        -> int
    {
        auto found = std::find_if (
            old_list.begin(),
            old_list.end(),
            [piece_to_find, coord_to_find] (const auto& it)
                -> bool
            {
                auto key = it.first;
                auto value = it.second;
                auto piece = mapColoredPiece (value);
                auto piece_coord = makeCoord (value.row, value.col);
                return piece_to_find == piece && piece_coord == coord_to_find;
            }
        );

        if (found != old_list.end())
        {
            auto position = found->first;
            auto value = found->second;
            old_list.erase (found);
            return value.id;
        }

        // find first match by row/column. Otherwise, find first match by
        // piece / color.
        return 0;
    }

    void WebGame::updatePieceList (ColoredPiece promoted_piece)
    {
        const Board& board = my_game.getBoard();

        WebColoredPieceList old_pieces = my_pieces;
        my_pieces.clear();

        std::unordered_map<int, WebColoredPiece> old_list {};
        std::unordered_map<int, ColoredPiece> deferred {};

        // Index the old pieces to be able to find the old ids:
        for (int i = 0; i < old_pieces.length; i++)
        {
            WebColoredPiece piece = old_pieces.pieces[i];
            Coord src = Coord::make (piece.row, piece.col);
            old_list[src.index()] = piece;
        }

        for (int i = 0; i < Num_Squares; i++)
        {
            Coord coord = Coord::fromIndex (i);
            ColoredPiece piece = board.pieceAt (coord);
            if (piece != Piece_And_Color_None)
            {
                int id = findAndRemoveId (old_list, coord, piece);
                if (id != 0)
                {
                    WebColoredPiece new_piece = {
                        id,
                        toInt (piece.color()),
                        toInt (piece.type()),
                        narrow<int8_t> (coord.row()),
                        narrow<int8_t> (coord.column()),
                    };
                    my_pieces.addPiece (new_piece);
                }
                else
                {
                    deferred[i] = piece;
                }
            }
        }

        if (!deferred.empty())
        {
            for (auto& value : deferred)
            {
                auto new_piece = value.second;
                auto pred = [new_piece, promoted_piece] (const auto& list_item) -> bool
                {
                    ColoredPiece old_piece = mapColoredPiece (list_item.second);
                    const auto pieces_match = old_piece == new_piece;
                    const auto promoted_piece_matches = (
                        promoted_piece != Piece_And_Color_None
                        && old_piece.color() == new_piece.color()
                        && old_piece.type() == Piece::Pawn
                        && new_piece.type() != Piece::Pawn
                    );
                    return old_piece == new_piece || promoted_piece_matches;
                };
                auto it = std::find_if (old_list.begin(), old_list.end(), pred);
                if (it == old_list.end())
                {
                    throw Error { "Couldn't find id." };
                }
                auto coord_idx = it->first;
                auto old_piece = it->second;
                auto coord = Coord::fromIndex (value.first);

                my_pieces.addPiece (WebColoredPiece {
                    old_piece.id,
                    old_piece.color,
                    mapPiece (new_piece.type()),
                    coord.row<int8_t>(),
                    coord.column<int8_t>(),
                });
            }
        }

        // Sort by the ID so that the pieces always have the same order
        // CSS animations removing the CSS classes will work.
        std::sort (
            my_pieces.pieces,
            my_pieces.pieces + my_pieces.length,
            [] (const WebColoredPiece& a, const WebColoredPiece& b)
            {
                return a.id < b.id;
            }
        );
    }

    void WebGame::updateWebDisplayedGameState()
    {
        // Call the base class implementation to update shared state:
        updateDisplayedGameState();

        // Update WASM-specific state:
        setMoveNumber (my_game.getHistory().getMoveHistory().size());
    }
}
