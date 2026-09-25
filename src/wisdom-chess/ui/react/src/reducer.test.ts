import { describe, it, expect } from 'vitest'
import { initialState as makeInitialState, reducer, type Action, type EngineSnapshot, type GameState } from './reducer'
import { wasmEnums } from './test/wasmEnums'

const createSnapshot = (): EngineSnapshot => ({
    pieces: [],
    currentTurn: wasmEnums.White,
    inCheck: false,
    gameStatus: wasmEnums.Playing,
    moveStatus: 'White to move',
    gameOverStatus: '',
    hasHumanPlayer: true,
})

const createInitialState = (): GameState => makeInitialState(createSnapshot())

describe('reducer', () => {
    describe('initialState', () => {
        it('starts from the snapshot with nothing selected', () => {
            const state = makeInitialState(createSnapshot())

            expect(state.moveStatus).toBe('White to move')
            expect(state.focusedSquare).toBe('')
            expect(state.pawnPromotionDialogSquare).toBe('')
            expect(state.lastDroppedSquare).toBe('')
        })
    })

    describe('SYNC action', () => {
        it('replaces the engine fields and keeps the selection', () => {
            const initialState = createInitialState()
            initialState.focusedSquare = 'e2'
            const mockPieces = [
                { id: 1, icon: 'pawn-white.svg', position: 'e2', color: wasmEnums.White },
                { id: 2, icon: 'pawn-black.svg', position: 'e7', color: wasmEnums.Black },
            ]
            const action: Action = {
                type: 'SYNC',
                snapshot: {
                    ...createSnapshot(),
                    pieces: mockPieces,
                    currentTurn: wasmEnums.Black,
                    inCheck: true,
                    moveStatus: 'Black in check',
                    gameStatus: wasmEnums.Checkmate,
                },
            }

            const newState = reducer(initialState, action)

            expect(newState.pieces).toEqual(mockPieces)
            expect(newState.currentTurn).toBe(wasmEnums.Black)
            expect(newState.inCheck).toBe(true)
            expect(newState.moveStatus).toBe('Black in check')
            expect(newState.gameStatus).toBe(wasmEnums.Checkmate)
            expect(newState.focusedSquare).toBe('e2')
        })
    })

    describe('FOCUS action', () => {
        it('sets focusedSquare and clears promotion/dropped state', () => {
            const initialState = createInitialState()
            initialState.pawnPromotionDialogSquare = 'e8'
            initialState.lastDroppedSquare = 'd4'

            const action: Action = {
                type: 'FOCUS',
                square: 'e2',
            }

            const newState = reducer(initialState, action)

            expect(newState.focusedSquare).toBe('e2')
            expect(newState.pawnPromotionDialogSquare).toBe('')
            expect(newState.lastDroppedSquare).toBe('')
        })

        it('preserves other state', () => {
            const initialState = createInitialState()
            initialState.moveStatus = 'White to move'

            const action: Action = {
                type: 'FOCUS',
                square: 'a1',
            }

            const newState = reducer(initialState, action)

            expect(newState.moveStatus).toBe('White to move')
        })
    })

    describe('CLEAR_FOCUS action', () => {
        it('clears all focus-related state', () => {
            const initialState = createInitialState()
            initialState.focusedSquare = 'e2'
            initialState.pawnPromotionDialogSquare = 'e8'
            initialState.lastDroppedSquare = 'e4'

            const action: Action = { type: 'CLEAR_FOCUS' }

            const newState = reducer(initialState, action)

            expect(newState.focusedSquare).toBe('')
            expect(newState.pawnPromotionDialogSquare).toBe('')
            expect(newState.lastDroppedSquare).toBe('')
        })

        it('preserves other state', () => {
            const initialState = createInitialState()
            initialState.moveStatus = 'Black to move'
            initialState.gameStatus = wasmEnums.Stalemate

            const action: Action = { type: 'CLEAR_FOCUS' }

            const newState = reducer(initialState, action)

            expect(newState.moveStatus).toBe('Black to move')
            expect(newState.gameStatus).toBe(2)
        })
    })

    describe('REQUEST_PROMOTION action', () => {
        it('sets focusedSquare and pawnPromotionDialogSquare', () => {
            const initialState = createInitialState()

            const action: Action = {
                type: 'REQUEST_PROMOTION',
                src: 'e7',
                dst: 'e8',
            }

            const newState = reducer(initialState, action)

            expect(newState.focusedSquare).toBe('e7')
            expect(newState.pawnPromotionDialogSquare).toBe('e8')
        })

        it('preserves other state', () => {
            const initialState = createInitialState()
            initialState.moveStatus = 'White to move'
            initialState.lastDroppedSquare = 'd4'

            const action: Action = {
                type: 'REQUEST_PROMOTION',
                src: 'a7',
                dst: 'a8',
            }

            const newState = reducer(initialState, action)

            expect(newState.moveStatus).toBe('White to move')
            expect(newState.lastDroppedSquare).toBe('d4')
        })
    })

    describe('SET_LAST_DROPPED action', () => {
        it('updates lastDroppedSquare', () => {
            const initialState = createInitialState()

            const action: Action = {
                type: 'SET_LAST_DROPPED',
                square: 'e4',
            }

            const newState = reducer(initialState, action)

            expect(newState.lastDroppedSquare).toBe('e4')
        })

        it('can clear lastDroppedSquare', () => {
            const initialState = createInitialState()
            initialState.lastDroppedSquare = 'd4'

            const action: Action = {
                type: 'SET_LAST_DROPPED',
                square: '',
            }

            const newState = reducer(initialState, action)

            expect(newState.lastDroppedSquare).toBe('')
        })

        it('preserves other state', () => {
            const initialState = createInitialState()
            initialState.focusedSquare = 'a1'
            initialState.moveStatus = 'White to move'

            const action: Action = {
                type: 'SET_LAST_DROPPED',
                square: 'h8',
            }

            const newState = reducer(initialState, action)

            expect(newState.focusedSquare).toBe('a1')
            expect(newState.moveStatus).toBe('White to move')
        })
    })

    describe('immutability', () => {
        it('always returns a new state object', () => {
            const initialState = createInitialState()
            const actions: Action[] = [
                { type: 'SYNC', snapshot: createSnapshot() },
                { type: 'FOCUS', square: 'e2' },
                { type: 'CLEAR_FOCUS' },
                { type: 'REQUEST_PROMOTION', src: 'e7', dst: 'e8' },
                { type: 'SET_LAST_DROPPED', square: 'e4' },
            ]

            actions.forEach((action) => {
                const newState = reducer(initialState, action)
                expect(newState).not.toBe(initialState)
            })
        })

        it('does not mutate the original state', () => {
            const initialState = createInitialState()
            const originalFocused = initialState.focusedSquare
            const originalMoveStatus = initialState.moveStatus

            const action: Action = {
                type: 'FOCUS',
                square: 'e2',
            }

            reducer(initialState, action)

            expect(initialState.focusedSquare).toBe(originalFocused)
            expect(initialState.moveStatus).toBe(originalMoveStatus)
        })
    })
})
