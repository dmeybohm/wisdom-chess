import { GameState, WebGameSettings } from './lib/WisdomChess'

export type Action =
    | { type: 'BOOTSTRAP'; snapshot: Partial<GameState> }
    | { type: 'ENGINE_SYNC'; snapshot: Partial<GameState> }
    | { type: 'FOCUS'; square: string }
    | { type: 'CLEAR_FOCUS' }
    | { type: 'REQUEST_PROMOTION'; src: string; dst: string }
    | { type: 'SET_SETTINGS'; settings: WebGameSettings }
    | { type: 'SET_LAST_DROPPED'; square: string }

export function reducer(state: GameState, action: Action): GameState {
    switch (action.type) {
        case 'BOOTSTRAP':
        case 'ENGINE_SYNC':
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

        case 'SET_SETTINGS':
            return { ...state, settings: { ...action.settings } }

        case 'SET_LAST_DROPPED':
            return { ...state, lastDroppedSquare: action.square }

        default:
            return state
    }
}
