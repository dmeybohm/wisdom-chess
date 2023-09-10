#include "web_game.hpp"
#include "game_settings.hpp"
#include "piece.hpp"

int wisdom::WebGame::our_game_id;

namespace wisdom
{
    WebGame::WebGame (int white_player, int black_player) :
        my_game { mapPlayer (white_player), mapPlayer (black_player) }
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
                                        gsl::narrow<int> (Row (coord)),
                                        gsl::narrow<int> (Column (coord)) };
                my_pieces.addPiece (new_piece);
                id++;
            }
        }

        updateDisplayedGameState();
    }

    auto WebGame::makeMove (const WebMove* move_param) -> bool
    {
        Move move = move_param->getMove();

        my_game.move (move);

        updatePieceList (move.getPromotedPiece());
        updateDisplayedGameState();

        return true;
    }

    auto WebGame::newFromSettings (const GameSettings& settings) -> wisdom::WebGame*
    {
        auto* new_game = new WebGame ( settings.whitePlayer, settings.blackPlayer );

        const auto computer_depth = GameSettings::mapHumanDepthToComputerDepth (settings.searchDepth);
        new_game->setMaxDepth (computer_depth);
        new_game->setThinkingTime (std::chrono::seconds { settings.thinkingTime });

        return new_game;
    }

    auto WebGame::createMoveFromCoordinatesAndPromotedPiece (const WebCoord* src,
                                                             const WebCoord* dst,
                                                             int promoted_piece_type)
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

    auto WebGame::isLegalMove (const WebMove* selectedMovePtr) -> bool
    {
        Move selectedMove = selectedMovePtr->getMove();
        auto selectedMoveStr = asString (selectedMove);

        // If it's not the human's turn, move is illegal.
        if (my_game.getCurrentPlayer() != wisdom::Player::Human)
        {
            setMoveStatus ("Illegal move");
            return false;
        }

        auto who = my_game.getCurrentTurn();
        auto& generator = my_game.getMoveGenerator();
        auto legalMoves = generator.generateLegalMoves (my_game.getBoard(), who);

        auto result = std::any_of (legalMoves.cbegin(), legalMoves.cend(),
                                   [selectedMove] (const auto& move)
                                   {
                                       return move == selectedMove;
                                   });
        if (!result)
        {
            setMoveStatus ("Illegal move");
            return false;
        }
        return result;
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
        updateDisplayedGameState();
    }

    [[nodiscard]] auto WebGame::findAndRemoveId (std::unordered_map<int,
        WebColoredPiece>& old_list, Coord coord_to_find, ColoredPiece piece_to_find) -> int
    {
        auto found
            = std::find_if (old_list.begin(), old_list.end(),
                            [piece_to_find, coord_to_find] (const auto& it) -> bool
                            {
                                auto key = it.first;
                                auto value = it.second;
                                auto piece = mapColoredPiece (value);
                                auto piece_coord = makeCoord (value.row, value.col);
                                return piece_to_find == piece && piece_coord == coord_to_find;
                            });

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
            Coord src = makeCoord (piece.row, piece.col);
            old_list[coordIndex (src)] = piece;
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
                        gsl::narrow<int8_t> (Row (coord)),
                        gsl::narrow<int8_t> (Column (coord)),
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
                    Row<int8_t> (coord),
                    Column<int8_t> (coord),
                });
            }
        }

        // Sort by the ID so that the pieces always have the same order
        // CSS animations removing the CSS classes will work.
        std::sort (my_pieces.pieces, my_pieces.pieces + my_pieces.length,
                   [] (const WebColoredPiece& a, const WebColoredPiece& b)
                   {
                       return a.id < b.id;
                   });
    }

    class WebGameStatusUpdate : public GameStatusUpdate
    {
    private:
        observer_ptr<WebGame> parent;

    public:
        explicit WebGameStatusUpdate (observer_ptr<WebGame> parent_) : parent { parent_ }
        {}

        [[nodiscard]] static auto get_first_human_player (Players players) -> optional<Color>
        {
            if (players[0] == Player::Human) {
                return Color::White;
            }
            if (players[1] == Player::Human) {
                return Color::Black;
            }

            return {};
        }

        void checkmate() override
        {
            auto who = parent->my_game.getCurrentTurn();
            auto whoString = "<strong>Checkmate</strong> - " + wisdom::asString (colorInvert (who)) +
                " wins the game.";
            parent->setGameOverStatus (whoString);
        }

        void stalemate() override
        {
            auto who = parent->my_game.getCurrentTurn();
            auto stalemateStr = "<strong>Stalemate</strong> - No legal moves for " + wisdom::asString (who);
            parent->setGameOverStatus (stalemateStr);
        }

        void insufficientMaterial() override
        {
            parent->setGameOverStatus (
                "<strong>Draw</strong> - Insufficient material to checkmate.");
        }

        void thirdRepetitionDrawReached() override
        {
            // nothing
        }

        void thirdRepetitionDrawAccepted() override
        {
            parent->setGameOverStatus ("<strong>Draw</strong> - Threefold repetition rule.");
        }

        void fifthRepetitionDraw() override
        {
            parent->setGameOverStatus ("<strong>Draw</strong> - Fivefold repetition rule.");
        }

        void fiftyMovesWithoutProgressReached() override
        {
            // nothing
        }

        void fiftyMovesWithoutProgressAccepted() override
        {
            parent->setGameOverStatus ("<strong>Draw</strong> - Fifty moves without progress.");
        }

        void seventyFiveMovesWithNoProgress() override
        {
            parent->setGameOverStatus (
                "<strong>Draw</strong> - Seventy-five moves without progress.");
        }
    };

    void WebGame::updateDisplayedGameState()
    {
        auto who = my_game.getCurrentTurn();
        auto& board = my_game.getBoard();

        setMoveStatus ("");
        setGameOverStatus ("");
        setInCheck (false);
        setMoveNumber (my_game.getHistory().getMoveHistory().size());

        WebGameStatusUpdate update { this };
        auto nextStatus = my_game.status();
        update.update (nextStatus);

        if (wisdom::isKingThreatened (board, who, board.getKingPosition (who)))
            setInCheck (true);
    }

}

