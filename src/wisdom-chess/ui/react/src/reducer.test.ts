import { describe, it, expect } from 'vitest'
import { reducer, type Action } from './reducer'
import { GameState } from './lib/WisdomChess'
import { initialSquares } from './lib/Squares'

const createInitialState = (): GameState => ({
    pieces: [],
    squares: initialSquares,
    focusedSquare: '',
    pawnPromotionDialogSquare: '',
    lastDroppedSquare: '',
    gameStatus: 0,
    moveStatus: 'White to move',
    gameOverStatus: '',
    settings: {
        whitePlayer: 0,
        blackPlayer: 1,
        thinkingTime: 5,
        searchDepth: 4,
    },
    hasHumanPlayer: true,
})

describe('reducer', () => {
    describe('BOOTSTRAP action', () => {
        it('merges snapshot into state', () => {
            const initialState = createInitialState()
            const action: Action = {
                type: 'BOOTSTRAP',
                snapshot: {
                    moveStatus: 'Black to move',
                    gameStatus: 1,
                },
            }

            const newState = reducer(initialState, action)

            expect(newState.moveStatus).toBe('Black to move')
            expect(newState.gameStatus).toBe(1)
            expect(newState.focusedSquare).toBe('')
        })

        it('returns a new state object', () => {
            const initialState = createInitialState()
            const action: Action = {
                type: 'BOOTSTRAP',
                snapshot: { moveStatus: 'Test' },
            }

            const newState = reducer(initialState, action)

            expect(newState).not.toBe(initialState)
        })
    })

    describe('ENGINE_SYNC action', () => {
        it('merges snapshot into state', () => {
            const initialState = createInitialState()
            const action: Action = {
                type: 'ENGINE_SYNC',
                snapshot: {
                    moveStatus: 'White in check',
                    gameOverStatus: '',
                },
            }

            const newState = reducer(initialState, action)

            expect(newState.moveStatus).toBe('White in check')
            expect(newState.gameOverStatus).toBe('')
        })

        it('can update pieces array', () => {
            const initialState = createInitialState()
            const mockPieces = [
                { position: 'e2', color: 0, type: 5 },
                { position: 'e7', color: 1, type: 5 },
            ]
            const action: Action = {
                type: 'ENGINE_SYNC',
                snapshot: { pieces: mockPieces },
            }

            const newState = reducer(initialState, action)

            expect(newState.pieces).toEqual(mockPieces)
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
            initialState.gameStatus = 2

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

    describe('SET_SETTINGS action', () => {
        it('updates settings', () => {
            const initialState = createInitialState()

            const newSettings = {
                whitePlayer: 1,
                blackPlayer: 1,
                thinkingTime: 10,
                searchDepth: 6,
            }

            const action: Action = {
                type: 'SET_SETTINGS',
                settings: newSettings,
            }

            const newState = reducer(initialState, action)

            expect(newState.settings).toEqual(newSettings)
        })

        it('creates a new settings object', () => {
            const initialState = createInitialState()
            const newSettings = {
                whitePlayer: 0,
                blackPlayer: 0,
                thinkingTime: 15,
                searchDepth: 5,
            }

            const action: Action = {
                type: 'SET_SETTINGS',
                settings: newSettings,
            }

            const newState = reducer(initialState, action)

            expect(newState.settings).not.toBe(initialState.settings)
            expect(newState.settings).toEqual(newSettings)
        })

        it('preserves other state', () => {
            const initialState = createInitialState()
            initialState.focusedSquare = 'e4'
            initialState.moveStatus = 'Black to move'

            const action: Action = {
                type: 'SET_SETTINGS',
                settings: {
                    whitePlayer: 0,
                    blackPlayer: 1,
                    thinkingTime: 5,
                    searchDepth: 4,
                },
            }

            const newState = reducer(initialState, action)

            expect(newState.focusedSquare).toBe('e4')
            expect(newState.moveStatus).toBe('Black to move')
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
                { type: 'BOOTSTRAP', snapshot: {} },
                { type: 'ENGINE_SYNC', snapshot: {} },
                { type: 'FOCUS', square: 'e2' },
                { type: 'CLEAR_FOCUS' },
                { type: 'REQUEST_PROMOTION', src: 'e7', dst: 'e8' },
                { type: 'SET_SETTINGS', settings: initialState.settings },
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
