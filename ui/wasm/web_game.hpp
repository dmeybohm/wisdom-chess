#ifndef WISDOMCHESS_WEB_GAME_HPP
#define WISDOMCHESS_WEB_GAME_HPP

#include "web_types.hpp"
#include "web_move.hpp"

namespace wisdom
{

    class WebGame
    {
    private:
        Game my_game;
        WebColoredPieceList my_pieces;
        std::string my_move_status;
        std::string my_game_over_status;
        static int our_game_id;

    public:
        bool inCheck = false;
        char* moveStatus{};
        char* gameOverStatus{};
        int moveNumber{};
        int gameId = ++our_game_id;

        WebGame (int white_player, int black_player) :
            my_game { map_player (white_player), map_player (black_player) }
        {
            const auto& board = my_game.get_board();
            int id = 1;

            for (int i = 0; i < Num_Squares; i++)
            {
                auto coord = make_coord_from_index (i);
                auto piece = board.piece_at (coord);
                if (piece != Piece_And_Color_None)
                {
                    WebColoredPiece new_piece
                        = WebColoredPiece { id, to_int (piece.color()), to_int (piece.type()),
                                            gsl::narrow<int> (Row (coord)),
                                            gsl::narrow<int> (Column (coord)) };
                    my_pieces.addPiece (new_piece);
                    id++;
                }
            }

            update_displayed_game_state();
        }

        auto needsPawnPromotion (const WebCoord* src, const WebCoord* dst) -> bool
        {
            auto game_src = make_coord (src->row, src->col);
            auto game_dst = make_coord (dst->row, dst->col);

            auto optionalMove = my_game.map_coordinates_to_move (game_src, game_dst,
                                                                 Piece::Queen);
            if (!optionalMove.has_value())
                return false;
            return optionalMove->is_promoting();
        }

        auto createMoveFromCoordinatesAndPromotedPiece (const WebCoord* src, const WebCoord* dst,
                                                        int promoted_piece_type) -> WebMove*
        {
            auto game_src = make_coord (src->row, src->col);
            auto game_dst = make_coord (dst->row, dst->col);

            auto optionalMove = my_game.map_coordinates_to_move (game_src, game_dst,
                                                                 map_piece (promoted_piece_type));

            if (!optionalMove.has_value())
            {
                std::cout << "Failed to map to move"
                          << "\n";
                return nullptr;
            }

            auto move = *optionalMove;
            return new WebMove { move };
        }

        auto makeMove (const WebMove* move_param) -> bool
        {
            Move move = move_param->get_move();

            std::cout << "Trying to do move: " << to_string (move) << "\n";
            my_game.move (move);
            std::cout << "Updating piece list..."
                      << "\n";
            update_piece_list (move.get_promoted_piece());
            std::cout << "After updatePieceList()"
                      << "\n";
            update_displayed_game_state();

            // todo get if it's a computer:
            if (my_game.get_current_turn() == Color::Black)
                send_move_to_worker (move);

            return true;
        }

        auto isLegalMove (const WebMove* selectedMovePtr) -> bool
        {
            Move selectedMove = selectedMovePtr->get_move();
            auto selectedMoveStr = to_string (selectedMove);

            // If it's not the human's turn, move is illegal.
            if (my_game.get_current_player() != wisdom::Player::Human)
            {
                set_move_status ("Illegal move");
                return false;
            }

            auto who = my_game.get_current_turn();
            auto generator = my_game.get_move_generator();
            auto legalMoves = generator->generate_legal_moves (my_game.get_board(), who);

            auto result = std::any_of (legalMoves.cbegin(), legalMoves.cend(),
                                       [selectedMove] (const auto& move)
                                       {
                                           return move == selectedMove;
                                       });
            if (!result)
            {
                set_move_status ("Illegal move");
                return false;
            }
            return result;
        }

        void setMaxDepth (int max_depth)
        {
            std::cout << "Setting max depth: " << max_depth << "\n";
            my_game.set_max_depth (max_depth);
        }

        auto getMaxDepth() -> int
        {
            auto result = my_game.get_max_depth();
            std::cout << "Getting max depth: " << result << "\n";
            return result;
        }

        auto getPieceList() -> WebColoredPieceList&
        {
            return my_pieces;
        }

        auto getCurrentTurn() -> WebColor
        {
            return map_color (my_game.get_current_turn());
        }

    private:

        auto find_and_remove_id (std::unordered_map<int, WebColoredPiece>& old_list,
                                 Coord coord_to_find, ColoredPiece piece_to_find) -> int
        {
            auto found
                = std::find_if (old_list.begin(), old_list.end(),
                                [piece_to_find, coord_to_find] (const auto& it) -> bool
                                {
                                    auto key = it.first;
                                    auto value = it.second;
                                    auto piece = map_colored_piece (value);
                                    auto piece_coord = make_coord (value.row, value.col);
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

        void update_piece_list (ColoredPiece promoted_piece)
        {
            const Board& board = my_game.get_board();

            WebColoredPieceList old_pieces = my_pieces;
            my_pieces.clear();

            std::unordered_map<int, WebColoredPiece> old_list {};
            std::unordered_map<int, ColoredPiece> deferred {};

            // Index the old pieces to be able to find the old ids:
            for (int i = 0; i < old_pieces.length; i++)
            {
                WebColoredPiece piece = old_pieces.pieces[i];
                Coord src = make_coord (piece.row, piece.col);
                old_list[coord_index (src)] = piece;
            }

            for (int i = 0; i < Num_Squares; i++)
            {
                Coord coord = make_coord_from_index (i);
                ColoredPiece piece = board.piece_at (coord);
                if (piece != Piece_And_Color_None)
                {
                    int id = find_and_remove_id (old_list, coord, piece);
                    if (id != 0)
                    {
                        WebColoredPiece new_piece = {
                            id,
                            to_int (piece.color()),
                            to_int (piece.type()),
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
                        ColoredPiece old_piece = map_colored_piece (list_item.second);
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
                    auto coord = make_coord_from_index (value.first);

                    my_pieces.addPiece (WebColoredPiece {
                        old_piece.id,
                        old_piece.color,
                        map_piece (new_piece.type()),
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

        void update_displayed_game_state()
        {
            auto who = my_game.get_current_turn();
            auto& board = my_game.get_board();

            set_move_status ("");
            set_game_over_status ("");
            set_in_check (false);
            set_move_number (my_game.get_history().get_move_history().size());

            auto nextStatus = my_game.status();

            switch (nextStatus)
            {
                case GameStatus::Playing:
                    break;

                case GameStatus::Checkmate:
                {
                    auto whoString = "<strong>Checkmate</strong> - " + wisdom::to_string (color_invert (who))
                                     + " wins the game.";
                    set_game_over_status (whoString);
                    return;
                }

                case GameStatus::Stalemate:
                {
                    auto stalemateStr = "<strong>Stalemate</strong> - No legal moves for "
                                        + wisdom::to_string (who);
                    set_game_over_status (stalemateStr);
                    return;
                }

                case GameStatus::FivefoldRepetitionDraw:
                    set_game_over_status ("<strong>Draw</strong> - Fivefold repetition rule.");
                    return;

                case GameStatus::ThreefoldRepetitionReached:
//                    if (getFirstHumanPlayerColor (my_game->get_players()).has_value())
//                    {
//                        setThirdRepetitionDrawStatus (DrawStatus::Proposed);
//                    }
                    break;

                case GameStatus::FiftyMovesWithoutProgressReached:
//                    if (getFirstHumanPlayerColor (my_game->get_players()).has_value())
//                    {
//                        setFiftyMovesDrawStatus (DrawStatus::Proposed);
//                    }
                    break;

                case GameStatus::ThreefoldRepetitionAccepted:
                    set_game_over_status ("<strong>Draw</strong> - Threefold repetition rule.");
                    return;

                case GameStatus::FiftyMovesWithoutProgressAccepted:
                    set_game_over_status ("<strong>Draw</strong> - Fifty moves without progress.");
                    return;

                case GameStatus::SeventyFiveMovesWithoutProgressDraw:
                    set_game_over_status ("<strong>Draw</strong> - Seventy-five moves without progress.");
                    return;

                case GameStatus::InsufficientMaterialDraw:
                    set_game_over_status ("<strong>Draw</strong> - Insufficient material to checkmate.");
                    return;
            }

            if (wisdom::is_king_threatened (board, who, board.get_king_position (who)))
            {
                set_in_check (true);
            }
        }

        void send_move_to_worker (Move move)
        {
            emscripten_wasm_worker_post_function_vi (
                engine_thread,
                worker_receive_move,
                move.to_int()
            );
        }

        void set_game_over_status (std::string new_status)
        {
            my_game_over_status = std::move (new_status);
            gameOverStatus = const_cast<char*> (my_game_over_status.c_str());
        }

        void set_in_check (bool new_in_check)
        {
            inCheck = new_in_check;
        }

        void set_move_number (size_t size)
        {
            moveNumber = gsl::narrow<int> (size);
        }

        void set_move_status (std::string new_move_status)
        {
            my_move_status = std::move (new_move_status);
            moveStatus = const_cast<char*> (my_move_status.c_str());
        }

    };
};
#endif // WISDOMCHESS_WEB_GAME_HPP
