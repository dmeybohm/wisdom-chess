import {
    DrawProposed,
    Game, GameSettings, GameStatus,
    getCurrentGame, getCurrentGameSettings,
    getGameModel,
    getPieces, PieceColor,
    PieceType,
    startNewGame,
    WebMove,
    WebPlayer, WisdomChess
} from "./WisdomChess";
import { initialSquares } from "./Squares";
import { wisdomChess } from "../App"
import { Color, Piece } from "./Pieces";
import { create } from "zustand";
import { ReactWindow } from "../main";

export type DrawStatus = 'not-reached' | 'proposed' | 'accepted' | 'declined'

export const initialGameState = {
    pieces: [] as Piece[],
    squares: initialSquares,
    focusedSquare: '',
    gameStatus: 0 as GameStatus,
    pawnPromotionDialogSquare: '',

    settings: {
        // Initialized from web assembly later:
        whitePlayer: 0,
        blackPlayer: 1,
        thinkingTime: 2,
        searchDepth: 3
    }
}

export type ChessEngineEventType = 'computerMoved' | 'computerDrawStatusUpdated'

interface GameState {
    pieces: Piece[]
    squares: typeof initialSquares,
    focusedSquare: string
    pawnPromotionDialogSquare: string
    gameStatus: GameStatus
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
        pauseGame: () => void
        unpauseGame: () => void
        setHumanThirdRepetitionDrawStatus: (accepted: boolean) => void
        setHumanFiftyMovesDrawStatus: (accepted: boolean) => void
    }
}

type ComputerDrawStatusUpdate = {
    is_third_repetition: boolean
    color: PieceColor,
    accepted: boolean
}

function updateGameState(initialState: Partial<GameState>, currentGame: Game): Partial<GameState> {
    return {
        ... initialState,
        pieces: getPieces(currentGame),
        gameStatus: currentGame.getStatus(),
    }
}

export const useGame = create<GameState>()((set, get) => ({
    ... initialGameState,

    actions: {
        init: (window: ReactWindow) => set(state => {
            window.receiveWorkerMessage = get().actions.receiveWorkerMessage
            return updateGameState({
                settings: getCurrentGameSettings(),
            }, getCurrentGame())
        }),
        //
        // Receive a message from the chess engine thread.
        //
        receiveWorkerMessage: (type: ChessEngineEventType, gameId: number, message: string) => {
            const wisdomChess = WisdomChess()
            switch (type) {
                case 'computerMoved': {
                    const move = wisdomChess.WebMove.prototype.fromString(
                        message,
                        getCurrentGame().getCurrentTurn()
                    );
                    get().actions.computerMovePiece(move)
                    setTimeout(() => getGameModel().notifyComputerMove(), 555)
                    break;
                }

                case 'computerDrawStatusUpdated': {
                    const params = JSON.parse(message) as ComputerDrawStatusUpdate
                    const currentGame = getCurrentGame()
                    currentGame.setComputerDrawStatus(params.is_third_repetition ?
                                        'third' : 'fifty',
                        params.color,
                        params.accepted
                    )
                    get().actions.computer
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
        computerUpdateDrawStatus: (color: Color, accepted: boolean) => set((prevState) => {

        }),
        computerMovePiece: (move: WebMove) => set((prevState) => {
            const game = getCurrentGame()
            game.makeMove(move);
            return updateGameState({}, game)
        }),
        pieceClick: (dst: string) => set((prevState) => {
            const game = getCurrentGame()

            // begin focus if no focus already:
            if (prevState.focusedSquare === '') {
                return updateGameState({
                    focusedSquare: dst,
                    pawnPromotionDialogSquare: ''
                }, game)
            }

            // toggle focus if already focused:
            if (prevState.focusedSquare === dst) {
                return updateGameState({
                    focusedSquare: dst,
                    pawnPromotionDialogSquare: ''
                }, game)
            }

            // Change focus if piece is the same color:
            const pieces = getPieces(game)
            const dstPiece = findPieceAtPosition(pieces, dst)
            const srcPiece = findPieceAtPosition(pieces, prevState.focusedSquare)
            if (!dstPiece || !srcPiece || srcPiece.color === dstPiece.color) {
                return updateGameState({
                    focusedSquare: dst,
                    pawnPromotionDialogSquare: ''
                }, game)
            }

            // take the piece with the move-piece action, and then remove the focus:
            get().actions.humanMovePiece(dst)
            const updatedState = get()
            if (updatedState.pawnPromotionDialogSquare === '')
                updatedState.focusedSquare = ''
            return updateGameState(updatedState, game)
        }),
        promotePiece: (pieceType: PieceType) => set((prevState) => {
            const newState : Partial<GameState> = {}
            const game = getCurrentGame()
            const src = prevState.focusedSquare
            const dst = prevState.pawnPromotionDialogSquare

            const srcCoord = wisdomChess.WebCoord.prototype.fromTextCoord(src)
            const dstCoord = wisdomChess.WebCoord.prototype.fromTextCoord(dst)

            const move = game.createMoveFromCoordinatesAndPromotedPiece(
                srcCoord,
                dstCoord,
                pieceType,
            );
            if (!move || !game.isLegalMove(move)) {
                return updateGameState({
                    pawnPromotionDialogSquare: '',
                    focusedSquare: '',
                }, game)
            }
            game.makeMove(move)
            game.notifyMove(move)
            return updateGameState(newState, game)
        }),
        humanMovePiece: (dst: string) => set((prevState): Partial<GameState> => {
            const game = getCurrentGame()
            const gameModel = getGameModel()
            const src = prevState.focusedSquare;

            if (src === '') {
                return updateGameState({
                    pawnPromotionDialogSquare: ''
                }, game)
            }
            const srcCoord = wisdomChess.WebCoord.prototype.fromTextCoord(src)
            const dstCoord = wisdomChess.WebCoord.prototype.fromTextCoord(dst)

            const move = game.createMoveFromCoordinatesAndPromotedPiece(
                srcCoord,
                dstCoord,
                wisdomChess.Queen
            );
            if (!move || !game.isLegalMove(move)) {
                return updateGameState({
                    focusedSquare: '',
                    pawnPromotionDialogSquare: '',
                }, game)
            }
            if (game.needsPawnPromotion(srcCoord, dstCoord)) {
                return updateGameState({
                    focusedSquare: '',
                    pawnPromotionDialogSquare: dst,
                }, game)
            }

            game.makeMove(move)
            gameModel.notifyHumanMove(move)
            return updateGameState({
                focusedSquare: ''
            }, game)
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

            return updateGameState({ settings: workerGameSettings }, game)
        }),
        pauseGame: () => {
            getGameModel().sendPause()
        },
        unpauseGame: () => {
            getGameModel().sendUnpause()
        },
        setHumanThirdRepetitionDrawStatus: (accepted: boolean) => set((prevState): Partial<GameState> => {
            getCurrentGame().setHumanThirdRepetitionDrawStatus(accepted)
            return updateGameState({}, getCurrentGame())
        }),
        setHumanFiftyMovesDrawStatus: (accepted: boolean) => set((prevState): Partial<GameState> => {
            getCurrentGame().setHumanFiftyMovesDrawStatus(accepted)
            return updateGameState({}, getCurrentGame())
        })
    }
}))

function findPieceAtPosition(pieces: Piece[], position: string): Piece|undefined {
    return pieces.find(piece => piece.position === position)
}