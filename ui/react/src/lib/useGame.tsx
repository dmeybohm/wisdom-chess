import {
    DrawByRepetitionType,
    DrawProposed, fromColorToNumber,
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
    gameOverStatus: '',
    moveStatus: 'White to move',
    pawnPromotionDialogSquare: '',

    settings: {
        // Initialized from web assembly later:
        whitePlayer: 0 as WebPlayer,
        blackPlayer: 1 as WebPlayer,
        thinkingTime: 2,
        searchDepth: 3,
    }
}

export type ChessEngineEventType = 'computerMoved' | 'computerDrawStatusUpdated'

interface GameState {
    pieces: Piece[]
    squares: typeof initialSquares,
    focusedSquare: string
    pawnPromotionDialogSquare: string
    gameStatus: GameStatus
    gameOverStatus: string
    moveStatus: string
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
        setHumanDrawStatus: (drawType: DrawByRepetitionType, who: PieceColor, accepted: boolean) => void
    }
}

type ComputerDrawStatusUpdate = {
    draw_type: DrawByRepetitionType
    color: PieceColor
    accepted: boolean
}

function updateGameState(initialState: Partial<GameState>, currentGame: Game): Partial<GameState> {
    return {
        ... initialState,
        pieces: getPieces(currentGame),
        gameStatus: currentGame.getGameStatus(),
        moveStatus: currentGame.getMoveStatus(),
        gameOverStatus: currentGame.getGameOverStatus(),
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
                    set((prevState) => {
                        const currentGame = getCurrentGame()
                        currentGame.setComputerDrawStatus(
                            params.draw_type,
                            params.color,
                            params.accepted
                        )
                        return updateGameState({}, getCurrentGame())
                    })
                    break;
                }

                default: {
                    console.error("Unknown message type: ", type);
                    break;
                }
            }
        },
        startNewGame: () => set(state => {
            startNewGame()
            return updateGameState({
                focusedSquare: '',
                pawnPromotionDialogSquare: '',
                settings: getCurrentGameSettings(),
            }, getCurrentGame())
        }),
        computerMovePiece: (move: WebMove) => set((prevState) => {
            const game = getCurrentGame()
            game.makeMove(move);
            return updateGameState({}, game)
        }),
        pieceClick: (dst: string) => set((prevState) => {
            const game = getCurrentGame()

            // do nothing if not playing:
            if (prevState.gameStatus != wisdomChess.Playing) {
                return {}
            }

            // begin focus if no focus already:
            if (prevState.focusedSquare === '') {
                const pieces = getPieces(game)
                const srcPiece = findPieceAtPosition(pieces, dst)
                if (srcPiece && game.getPlayerOfColor(fromColorToNumber(srcPiece.color)) == wisdomChess.Human) {
                    return updateGameState({
                        focusedSquare: dst,
                        pawnPromotionDialogSquare: ''
                    }, game)
                } else {
                    return {}
                }
            }

            // toggle focus if already focused:
            if (prevState.focusedSquare === dst) {
                return updateGameState({
                    focusedSquare: '',
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

            let move;
            try {
                move = game.createMoveFromCoordinatesAndPromotedPiece(
                    srcCoord,
                    dstCoord,
                    pieceType,
                );
            } catch (e) {
                console.error("Error:", e)
                return updateGameState({
                    pawnPromotionDialogSquare: '',
                    focusedSquare: '',
                }, game)
            }
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

            let move;
            try {
                move = game.createMoveFromCoordinatesAndPromotedPiece(
                    srcCoord,
                    dstCoord,
                    wisdomChess.Queen
                );
            } catch (e) {
                console.error("Error:", e)
                console.log('returning:')
                return updateGameState({
                    pawnPromotionDialogSquare: '',
                    focusedSquare: '',
                }, game)
            }
            if (!move || !game.isLegalMove(move)) {
                return updateGameState({
                    pawnPromotionDialogSquare: '',
                    focusedSquare: '',
                }, game)
            }
            if (game.needsPawnPromotion(srcCoord, dstCoord)) {
                return updateGameState({
                    pawnPromotionDialogSquare: dst,
                    focusedSquare: '',
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
        setHumanDrawStatus: (drawType: DrawByRepetitionType, who: PieceColor, accepted: boolean) => set((prevState) => {
            const currentGame = getCurrentGame()
            const gameModel = getGameModel()
            const firstHumanPlayer = gameModel.getFirstHumanPlayerColor()
            const secondHumanPlayer = gameModel.getSecondHumanPlayerColor()

            if (firstHumanPlayer === wisdomChess.NoColor) {
                console.error("Invalid color for human player draw type")
                return prevState
            }
            currentGame.setHumanDrawStatus(drawType, firstHumanPlayer, accepted)
            if (secondHumanPlayer != wisdomChess.NoColor) {
                currentGame.setHumanDrawStatus(drawType, secondHumanPlayer, accepted);
            }
            return updateGameState({}, getCurrentGame())
        }),
    }
}))

function findPieceAtPosition(pieces: Piece[], position: string): Piece|undefined {
    return pieces.find(piece => piece.position === position)
}