import { Piece } from './lib/Pieces'
import { GameStatus, PieceColor } from './lib/WisdomChess'

// What the view reads from the current game after each change.
export type EngineSnapshot = {
    pieces: Piece[]
    currentTurn: PieceColor
    inCheck: boolean
    gameStatus: GameStatus
    moveStatus: string
    gameOverStatus: string
    hasHumanPlayer: boolean
}

export type GameState = EngineSnapshot & {
    focusedSquare: string
    pawnPromotionDialogSquare: string
    lastDroppedSquare: string
}

export type Action =
    | { type: 'SYNC'; snapshot: EngineSnapshot }
    | { type: 'FOCUS'; square: string }
    | { type: 'CLEAR_FOCUS' }
    | { type: 'REQUEST_PROMOTION'; src: string; dst: string }
    | { type: 'SET_LAST_DROPPED'; square: string }

export function initialState(snapshot: EngineSnapshot): GameState {
    return {
        ...snapshot,
        focusedSquare: '',
        pawnPromotionDialogSquare: '',
        lastDroppedSquare: '',
    }
}

export function reducer(state: GameState, action: Action): GameState {
    switch (action.type) {
        case 'SYNC':
            return { ...state, ...action.snapshot }

        case 'FOCUS':
            return {
                ...state,
                focusedSquare: action.square,
                pawnPromotionDialogSquare: '',
                lastDroppedSquare: '',
            }

        case 'CLEAR_FOCUS':
            return {
                ...state,
                focusedSquare: '',
                pawnPromotionDialogSquare: '',
                lastDroppedSquare: '',
            }

        case 'REQUEST_PROMOTION':
            return {
                ...state,
                focusedSquare: action.src,
                pawnPromotionDialogSquare: action.dst,
            }

        case 'SET_LAST_DROPPED':
            return { ...state, lastDroppedSquare: action.square }
    }
}
