#include <iostream>

#include "wisdom-chess/engine/coord.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/piece.hpp"

#include "wisdom-chess/ui/wasm/web_game.hpp"
#include "wisdom-chess/ui/wasm/game_settings.hpp"
#include "wisdom-chess/ui/viewmodel/piece_movement.hpp"

namespace wisdom
{
    WebGame::WebGame (WebPlayer white_player, WebPlayer black_player, int game_id)
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
                    = WebColoredPiece { id, mapColor (piece.color()), mapPiece (piece.type()),
                                        widen<int> (coord.row()),
                                        widen<int> (coord.column()) };
                my_pieces.addPiece (new_piece);
                id++;
            }
        }

        updateDisplayedGameState();
    }

    namespace
    {
        auto
        parseSquare (czstring text) noexcept
            -> optional<Coord>
        {
            if (text == nullptr)
                return nullopt;

            return coordParseOptional (text);
        }
    }

    void WebGame::applyMove (Move move)
    {
        my_game.move (move);

        updatePieceList (move);
        updateDisplayedGameState();
    }

    auto
    WebGame::needsPawnPromotion (czstring src, czstring dst) const
        -> bool
    {
        auto src_coord = parseSquare (src);
        auto dst_coord = parseSquare (dst);

        if (!src_coord.has_value() || !dst_coord.has_value())
            return false;

        return GameViewModelBase::needsPawnPromotion (
            src_coord->row<int>(), src_coord->column<int>(),
            dst_coord->row<int>(), dst_coord->column<int>()
        );
    }

    void WebGame::makeComputerMove (czstring move_text)
    {
        applyMove (moveParse (move_text, my_game.getCurrentTurn()));
    }

    auto
    WebGame::newFromSettings (const GameSettings& settings, int game_id)
        -> unique_ptr<WebGame>
    {
        auto new_game = make_unique<WebGame> (settings.whitePlayer, settings.blackPlayer, game_id);

        const auto computer_depth = ui::fullMovesToPlyDepth (settings.searchDepth);
        new_game->setMaxDepth (computer_depth);
        new_game->setThinkingTime (chrono::seconds { settings.thinkingTime });

        return new_game;
    }

    auto
    WebGame::makeHumanMove (czstring src, czstring dst, WebPiece promoted_piece_type)
        -> int
    {
        auto src_coord = parseSquare (src);
        auto dst_coord = parseSquare (dst);

        if (!src_coord.has_value() || !dst_coord.has_value())
            return Illegal_Move;

        auto move = my_game.mapCoordinatesToMove (*src_coord, *dst_coord, mapPiece (promoted_piece_type));

        if (!move.has_value())
            return Illegal_Move;

        if (my_game.getCurrentPlayer() != wisdom::Player::Human || !isLegalMove (*move))
        {
            setMoveStatus ("Illegal move");
            return Illegal_Move;
        }

        applyMove (*move);
        return move->toPacked();
    }

    void WebGame::setSettings (const wisdom::GameSettings& settings)
    {
        settings.applyToGame (&my_game);
    }

    void WebGame::setComputerDrawStatus (WebDrawByRepetitionType type, WebColor who, bool accepted)
    {
        ProposedDrawType proposed_draw_type = mapDrawByRepetitionType (type);
        Color color = mapColor (who);

        my_game.setProposedDrawStatus (proposed_draw_type, color, accepted);
        updateDisplayedGameState();
    }

    void WebGame::updatePieceList (Move move)
    {
        auto movement = ui::pieceMovement (move);

        if (movement.captured.has_value())
        {
            int captured_index = my_pieces.indexOf (*movement.captured);
            EXPECTS( captured_index >= 0 );
            my_pieces.removeAt (captured_index);
        }

        auto relocate = [this] (ui::PieceStep step) -> WebColoredPiece&
        {
            int index = my_pieces.indexOf (step.src);
            EXPECTS( index >= 0 );

            auto& piece = my_pieces.pieces[index];
            piece.row = step.dst.row<int>();
            piece.col = step.dst.column<int>();
            return piece;
        };

        auto& mover = relocate (movement.mover);
        if (movement.promoted_piece != Piece::None)
            mover.piece = mapPiece (movement.promoted_piece);

        if (movement.castling_rook.has_value())
            relocate (*movement.castling_rook);

        ASSERT( hasPieceListMatchingBoard() );
    }

    auto
    WebGame::hasPieceListMatchingBoard() const
        -> bool
    {
        const auto& board = my_game.getBoard();
        int pieces_on_board = 0;

        for (auto coord : Board::allCoords())
        {
            auto piece = board.pieceAt (coord);
            if (piece == Piece_And_Color_None)
                continue;

            pieces_on_board++;

            int index = my_pieces.indexOf (coord);
            if (index < 0)
                return false;

            const auto& displayed = my_pieces.pieces[index];
            if (displayed.color != mapColor (piece.color()) || displayed.piece != mapPiece (piece.type()))
                return false;
        }

        return pieces_on_board == my_pieces.length;
    }
}
