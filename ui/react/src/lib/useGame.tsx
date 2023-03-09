import {
    Game, GameSettings,
    getCurrentGame, getCurrentGameSettings,
    getGameModel,
    getPieces,
    PieceType,
    startNewGame,
    WebMove,
    WebPlayer, WisdomChess
} from "./WisdomChess";
import { initialSquares } from "./Squares";
import { Piece } from "./Pieces";
import { wisdomChess } from "../App";
import { create } from "zustand";
import { ReactWindow } from "../main";

export const initialGameState = {
    pieces: [] as Piece[],
    squares: initialSquares,
    focusedSquare: '',
    pawnPromotionDialogSquare: '',
    settings: {
        // Initialized from web assembly later:
        whitePlayer: 0,
        blackPlayer: 1,
        thinkingTime: 2,
        searchDepth: 3
    }
}

export type ChessEngineEventType = 'computerMoved'

interface GameState {
    pieces: Piece[]
    squares: typeof initialSquares,
    focusedSquare: string
    pawnPromotionDialogSquare: string
    settings: GameSettings

    actions: {
        init: (window: ReactWindow) => void
        startNewGame: () => void
        humanMovePiece: (dst: string) => void
        computerMovePiece: (move: WebMove) => void
        pieceClick: (dst: string) => void
        promotePiece: (pieceType: PieceType) => void
        applySettings: (newSettings: GameSettings) => void
        receiveWorkerMessage: (type: ChessEngineEventType, gameId: number, message: string) => void
    }
}

export const useGame = create<GameState>()((set, get) => ({
    ... initialGameState,

    actions: {
        init: (window: ReactWindow) => set(state => {
            window.receiveWorkerMessage = get().actions.receiveWorkerMessage
            return {
                settings: getCurrentGameSettings(),
                pieces: getPieces(getCurrentGame())
            }
        }),
        //
        // Receive a message from the chess engine thread.
        //
        receiveWorkerMessage: (type: ChessEngineEventType, gameId: number, message: string) => {
            switch (type) {
                case 'computerMoved': {
                    const gameModel = getGameModel()
                    const move = wisdomChess.WebMove.prototype.fromString(
                        message,
                        getCurrentGame().getCurrentTurn()
                    );
                    get().actions.computerMovePiece(move)
                    gameModel.notifyComputerMove()
                    break;
                }

                default: {
                    console.error("Unknown message type: ", type);
                    break;
                }
            }
        },
        startNewGame: () => set(state => {
            const newGame = startNewGame()
            return {
                ... initialGameState,
                settings: getCurrentGameSettings(),
                pieces: getPieces(newGame)
            }
        }),
        computerMovePiece: (move: WebMove) => set((prevState) => {
            const game = getCurrentGame()
            game.makeMove(move);
            return {
                pieces: getPieces(game)
            }
        }),
        pieceClick: (dst: string) => set((prevState) => {
            let newState : Partial<GameState> = {}
            const game = getCurrentGame()

            // begin focus if no focus already:
            if (prevState.focusedSquare === '') {
                newState.focusedSquare = dst
                newState.pawnPromotionDialogSquare = ''
                return newState
            }

            // toggle focus if already focused:
            if (prevState.focusedSquare === dst) {
                newState.focusedSquare = ''
                newState.pawnPromotionDialogSquare = ''
                return newState
            }

            // Change focus if piece is the same color:
            const pieces = getPieces(game)
            const dstPiece = findPieceAtPosition(pieces, dst)
            const srcPiece = findPieceAtPosition(pieces, prevState.focusedSquare)
            if (!dstPiece || !srcPiece || srcPiece.color === dstPiece.color) {
                newState.focusedSquare = dst
                newState.pawnPromotionDialogSquare = ''
                return newState
            }

            // take the piece with the move-piece action, and then remove the focus:
            get().actions.humanMovePiece(dst)
            const updatedState = get()
            newState.pieces = getPieces(game)
            if (updatedState.pawnPromotionDialogSquare === '')
                newState.focusedSquare = ''
            return newState
        }),
        promotePiece: (pieceType: PieceType) => set((prevState) => {
            const newState : Partial<GameState> = {}
            const game = getCurrentGame()
            const src = prevState.focusedSquare
            const dst = prevState.pawnPromotionDialogSquare

            newState.pawnPromotionDialogSquare = ''
            newState.focusedSquare = ''

            const srcCoord = wisdomChess.WebCoord.prototype.fromTextCoord(src)
            const dstCoord = wisdomChess.WebCoord.prototype.fromTextCoord(dst)

            const move = game.createMoveFromCoordinatesAndPromotedPiece(
                srcCoord,
                dstCoord,
                pieceType,
            );
            if (!move || !game.isLegalMove(move)) {
                return newState
            }
            game.makeMove(move)
            game.notifyMove(move)
            newState.pieces = getPieces(game)
            return newState
        }),
        humanMovePiece: (dst: string) => set((prevState): Partial<GameState> => {
            const game = getCurrentGame()
            const gameModel = getGameModel()
            const newState : Partial<GameState> = {
                pawnPromotionDialogSquare: '',
            };
            const src = prevState.focusedSquare;
            if (src === '') {
                return newState
            }
            const srcCoord = wisdomChess.WebCoord.prototype.fromTextCoord(src)
            const dstCoord = wisdomChess.WebCoord.prototype.fromTextCoord(dst)

            const move = game.createMoveFromCoordinatesAndPromotedPiece(
                srcCoord,
                dstCoord,
                wisdomChess.Queen
            );
            if (!move || !game.isLegalMove(move)) {
                newState.focusedSquare = ''
                return newState
            }
            if (game.needsPawnPromotion(srcCoord, dstCoord)) {
                newState.pawnPromotionDialogSquare = dst
                return newState
            }

            newState.focusedSquare = ''
            game.makeMove(move)
            gameModel.notifyHumanMove(move)
            newState.pieces = getPieces(game)
            return newState
        }),
        applySettings: (newSettings: GameSettings) => set((prevState): Partial<GameState> => {
            const wisdomChessModule = WisdomChess()
            const workerGameSettings = new wisdomChessModule.GameSettings()
            const gameModel = getGameModel()

            // Map from JS object to lower-level WebAssembly type:
            workerGameSettings.whitePlayer = newSettings.whitePlayer
            workerGameSettings.blackPlayer = newSettings.blackPlayer
            workerGameSettings.thinkingTime = newSettings.thinkingTime
            workerGameSettings.searchDepth = newSettings.searchDepth

            gameModel.setCurrentGameSettings(workerGameSettings)
            const game = getCurrentGame()
            game.setSettings(workerGameSettings)

            return {
                settings: workerGameSettings
            }
        }),
    }
}))

function findPieceAtPosition(pieces: Piece[], position: string): Piece|undefined {
    return pieces.find(piece => piece.position === position)
}