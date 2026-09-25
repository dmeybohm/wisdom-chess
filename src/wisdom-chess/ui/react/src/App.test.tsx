import { describe, it, expect, beforeEach, vi } from 'vitest'
import { act, render, screen } from '@testing-library/react'
import userEvent from '@testing-library/user-event'
import App from './App'
import type { WisdomWindow, Game, GameModel, GameSettings, WasmObject, WisdomChess } from './lib/WisdomChess'
import { ILLEGAL_MOVE, getWisdomWindow, withWasmObjects } from './lib/WisdomChess'
import { wasmEnums } from './test/wasmEnums'

const createMockGame = (): Game => ({
    getGameStatus: vi.fn(() => wasmEnums.Playing),
    getMoveStatus: vi.fn(() => 'White to move'),
    getGameOverStatus: vi.fn(() => ''),
    getCurrentTurn: vi.fn(() => wasmEnums.White),
    getInCheck: vi.fn(() => false),
    getGameId: vi.fn(() => 0),
    getPieceList: vi.fn(() => ({
        length: 0,
        pieceAt: vi.fn(() => null),
    })),
    makeHumanMove: vi.fn(() => 0),
    makeComputerMove: vi.fn(),
    needsPawnPromotion: vi.fn(() => false),
    getPlayerOfColor: vi.fn(() => wasmEnums.Human),
    setSettings: vi.fn(),
    setHumanDrawStatus: vi.fn(),
    setComputerDrawStatus: vi.fn(),
} as unknown as Game)

const createMockGameModel = (): GameModel => ({
    startNewGame: vi.fn(() => createMockGame()),
    getCurrentGameSettings: vi.fn(() => ({
        whitePlayer: wasmEnums.Human,
        blackPlayer: wasmEnums.ChessEngine,
        thinkingTime: 5,
        searchDepth: 4,
        debugLogging: false,
    })),
    getFirstHumanPlayerColor: vi.fn(() => wasmEnums.White),
    getSecondHumanPlayerColor: vi.fn(() => wasmEnums.NoColor),
    setCurrentGameSettings: vi.fn(),
    notifyHumanMove: vi.fn(),
    notifyComputerMove: vi.fn(),
    sendPause: vi.fn(),
    sendUnpause: vi.fn(),
} as unknown as GameModel)

const createMockWisdomChess = (): WisdomChess => ({
    ...wasmEnums,
    GameSettings: vi.fn(function (this: GameSettings) {
        this.whitePlayer = wasmEnums.Human
        this.blackPlayer = wasmEnums.ChessEngine
        this.thinkingTime = 5
        this.searchDepth = 4
        this.debugLogging = false
    }),
    destroy: vi.fn(),
} as unknown as WisdomChess)

describe('App', () => {
    let mockGame: Game
    let mockGameModel: GameModel
    let mockWisdomChess: WisdomChess

    beforeEach(() => {
        mockGame = createMockGame()
        mockGameModel = createMockGameModel()
        mockWisdomChess = createMockWisdomChess()

        const wisdomWindow = getWisdomWindow()
        wisdomWindow.wisdomChessWeb = mockWisdomChess
        wisdomWindow.wisdomChessGameModel = mockGameModel
        wisdomWindow.wisdomChessCurrentGame = mockGame
        wisdomWindow.setReceiveWorkerMessageCallback = vi.fn()
        wisdomWindow.receiveWorkerMessage = vi.fn()
    })

    it('renders the chess board', () => {
        render(<App />)

        const board = document.querySelector('.board')
        expect(board).toBeInTheDocument()
    })

    it('renders the top menu with buttons', () => {
        render(<App />)

        expect(screen.getByText('New Game')).toBeInTheDocument()
        expect(screen.getByText('Settings')).toBeInTheDocument()
        expect(screen.getByText('About')).toBeInTheDocument()
    })

    it('renders the status bar', () => {
        render(<App />)

        const statusBar = document.querySelector('.status-bar')
        expect(statusBar).toBeInTheDocument()
        expect(statusBar).toHaveTextContent(/to move/)
    })

    it('opens settings modal when Settings button is clicked', async () => {
        const user = userEvent.setup()
        render(<App />)

        const settingsButton = screen.getByText('Settings')
        await user.click(settingsButton)

        expect(screen.getByText('White Player')).toBeInTheDocument()
        expect(screen.getByText('Black Player')).toBeInTheDocument()
    })

    it('closes settings modal when Cancel is clicked', async () => {
        const user = userEvent.setup()
        render(<App />)

        const settingsButton = screen.getByText('Settings')
        await user.click(settingsButton)

        expect(screen.getByText('White Player')).toBeInTheDocument()

        const cancelButton = screen.getByText('Cancel')
        await user.click(cancelButton)

        expect(screen.queryByText('White Player')).not.toBeInTheDocument()
    })

    it('opens about modal when About button is clicked', async () => {
        const user = userEvent.setup()
        render(<App />)

        const aboutButton = screen.getByText('About')
        await user.click(aboutButton)

        expect(screen.getByText('About Wisdom Chess')).toBeInTheDocument()
    })

    it('opens new game modal when New Game button is clicked', async () => {
        const user = userEvent.setup()
        render(<App />)

        const newGameButton = screen.getByText('New Game')
        await user.click(newGameButton)

        expect(screen.getByText('Start a new Game?')).toBeInTheDocument()
    })

    it('asks about a draw again after a new game is started', async () => {
        const createDrawnGame = (): Game => {
            const game = createMockGame()
            vi.mocked(game.getGameStatus).mockReturnValue(mockWisdomChess.ThreefoldRepetitionReached)
            return game
        }
        const wisdomWindow = getWisdomWindow()
        wisdomWindow.wisdomChessCurrentGame = createDrawnGame()
        vi.mocked(mockGameModel.startNewGame).mockImplementation(createDrawnGame)

        const user = userEvent.setup()
        render(<App />)

        expect(screen.getByText('Third Repetition Reached')).toBeInTheDocument()
        await user.click(screen.getByText('No'))
        expect(screen.queryByText('Third Repetition Reached')).not.toBeInTheDocument()

        await user.click(screen.getByText('New Game'))
        await user.click(screen.getByText('Start New Game'))

        expect(screen.getByText('Third Repetition Reached')).toBeInTheDocument()
    })

    it('reports the answer to a threefold repetition draw with the draw type', async () => {
        vi.mocked(mockGame.getGameStatus).mockReturnValue(mockWisdomChess.ThreefoldRepetitionReached)

        const user = userEvent.setup()
        render(<App />)
        await user.click(screen.getByText('Yes'))

        expect(mockWisdomChess.ThreefoldRepetition).toBeDefined()
        expect(mockGame.setHumanDrawStatus).toHaveBeenCalledWith(
            mockWisdomChess.ThreefoldRepetition,
            mockWisdomChess.White,
            true,
        )
    })

    it('reports the answer to a fifty move draw with the draw type', async () => {
        vi.mocked(mockGame.getGameStatus).mockReturnValue(mockWisdomChess.FiftyMovesWithoutProgressReached)

        const user = userEvent.setup()
        render(<App />)
        await user.click(screen.getByText('No'))

        expect(mockGame.setHumanDrawStatus).toHaveBeenCalledWith(
            mockWisdomChess.FiftyMovesWithoutProgress,
            mockWisdomChess.White,
            false,
        )
    })

    it('shows the overlay behind a draw dialog', () => {
        vi.mocked(mockGame.getGameStatus).mockReturnValue(mockWisdomChess.ThreefoldRepetitionReached)

        render(<App />)

        expect(document.querySelector('.modal-overlay')).not.toBeNull()
    })

    it('pauses the game when a modal is open', async () => {
        const user = userEvent.setup()
        render(<App />)

        const aboutButton = screen.getByText('About')
        await user.click(aboutButton)

        expect(mockGameModel.sendPause).toHaveBeenCalled()
    })

    it('unpauses the game when all modals are closed', async () => {
        const user = userEvent.setup()
        render(<App />)

        const aboutButton = screen.getByText('About')
        await user.click(aboutButton)

        expect(mockGameModel.sendPause).toHaveBeenCalled()

        const closeButton = screen.getByText('OK')
        await user.click(closeButton)

        expect(mockGameModel.sendUnpause).toHaveBeenCalled()
    })

    it('bootstraps state from engine on mount', () => {
        render(<App />)

        expect(mockGame.getGameStatus).toHaveBeenCalled()
        expect(mockGame.getMoveStatus).toHaveBeenCalled()
        expect(mockGame.getGameOverStatus).toHaveBeenCalled()
    })

    it('registers worker message callback on mount', () => {
        const wisdomWindow = getWisdomWindow()
        const setCallbackSpy = vi.fn()
        wisdomWindow.setReceiveWorkerMessageCallback = setCallbackSpy

        render(<App />)

        expect(setCallbackSpy).toHaveBeenCalled()
        expect(typeof setCallbackSpy.mock.calls[0][0]).toBe('function')
    })
})

describe('Engine interface', () => {
    let mockGame: Game
    let mockGameModel: GameModel
    let mockWisdomChess: WisdomChess
    let wisdomWindow: WisdomWindow

    beforeEach(() => {
        mockGame = createMockGame()
        mockGameModel = createMockGameModel()
        mockWisdomChess = createMockWisdomChess()

        wisdomWindow = getWisdomWindow()
        wisdomWindow.wisdomChessWeb = mockWisdomChess
        wisdomWindow.wisdomChessGameModel = mockGameModel
        wisdomWindow.wisdomChessCurrentGame = mockGame
        wisdomWindow.setReceiveWorkerMessageCallback = vi.fn()
        wisdomWindow.receiveWorkerMessage = vi.fn()
    })

    it('reads the settings when the settings dialog opens and frees them', async () => {
        const user = userEvent.setup()
        const wasmSettings = {
            whitePlayer: wasmEnums.Human,
            blackPlayer: wasmEnums.ChessEngine,
            thinkingTime: 5,
            searchDepth: 4,
            debugLogging: false,
        } as unknown as GameSettings
        vi.mocked(mockGameModel.getCurrentGameSettings).mockReturnValue(wasmSettings)

        render(<App />)
        await user.click(screen.getByText('About'))
        expect(mockGameModel.getCurrentGameSettings).not.toHaveBeenCalled()

        await user.click(screen.getByText('Settings'))
        expect(mockGameModel.getCurrentGameSettings).toHaveBeenCalledTimes(1)
        expect(mockWisdomChess.destroy).toHaveBeenCalledWith(wasmSettings)
    })

    const placeWhitePawnOnE2 = () => {
        vi.mocked(mockGame.getPieceList).mockReturnValue({
            length: 1,
            pieceAt: vi.fn(() => ({ id: 1, color: wasmEnums.White, piece: wasmEnums.Pawn, row: 6, col: 4 })),
        } as unknown as ReturnType<Game['getPieceList']>)
    }

    const clickSquare = async (user: ReturnType<typeof userEvent.setup>, index: number) => {
        await user.click(document.querySelectorAll('.square')[index])
    }

    it('makes a human move from square names and notifies the engine', async () => {
        const user = userEvent.setup()
        placeWhitePawnOnE2()
        vi.mocked(mockGame.makeHumanMove).mockReturnValue(1234)

        render(<App />)
        await user.click(document.querySelector('.piece.e2')!)
        await clickSquare(user, 36)

        expect(mockGame.makeHumanMove).toHaveBeenCalledWith('e2', 'e4', mockWisdomChess.Queen)
        expect(mockGameModel.notifyHumanMove).toHaveBeenCalledWith(1234)
    })

    it('does not notify the engine about an illegal human move', async () => {
        const user = userEvent.setup()
        placeWhitePawnOnE2()
        vi.mocked(mockGame.makeHumanMove).mockReturnValue(ILLEGAL_MOVE)

        render(<App />)
        await user.click(document.querySelector('.piece.e2')!)
        await clickSquare(user, 36)

        expect(mockGame.makeHumanMove).toHaveBeenCalled()
        expect(mockGameModel.notifyHumanMove).not.toHaveBeenCalled()
    })

    it('asks for a promotion piece before making the move', async () => {
        const user = userEvent.setup()
        placeWhitePawnOnE2()
        vi.mocked(mockGame.needsPawnPromotion).mockReturnValue(true)

        render(<App />)
        await user.click(document.querySelector('.piece.e2')!)
        await clickSquare(user, 36)

        expect(mockGame.needsPawnPromotion).toHaveBeenCalledWith('e2', 'e4')
        expect(mockGame.makeHumanMove).not.toHaveBeenCalled()
    })

    it('closes the promotion dialog once the piece is chosen', async () => {
        const user = userEvent.setup()
        placeWhitePawnOnE2()
        vi.mocked(mockGame.needsPawnPromotion).mockReturnValue(true)

        render(<App />)
        await user.click(document.querySelector('.piece.e2')!)
        await clickSquare(user, 36)
        const queen = () => document.querySelector('.pawn-promotion-dialog__piece')!
        await user.click(queen())
        await user.click(queen())

        expect(mockGame.makeHumanMove).toHaveBeenCalledWith('e2', 'e4', mockWisdomChess.Queen)
        expect(document.querySelector('.pawn-promotion-dialog')).toBeNull()
    })

    it('clears the selected square after a click move', async () => {
        const user = userEvent.setup()
        placeWhitePawnOnE2()

        render(<App />)
        await user.click(document.querySelector('.piece.e2')!)
        expect(document.querySelector('.piece.e2.focused')).not.toBeNull()
        await clickSquare(user, 36)

        expect(document.querySelector('.focused')).toBeNull()
    })

    it('passes a computer move to the game as text', () => {
        render(<App />)

        const onMessage = vi.mocked(wisdomWindow.setReceiveWorkerMessageCallback).mock.calls[0][0]
        act(() => onMessage('computerMoved', 0, 'e2 e4'))

        expect(mockGame.makeComputerMove).toHaveBeenCalledWith('e2 e4')
    })

    it('ignores a computer move from an earlier game', () => {
        render(<App />)

        const onMessage = vi.mocked(wisdomWindow.setReceiveWorkerMessageCallback).mock.calls[0][0]
        act(() => onMessage('computerMoved', 7, 'e2 e4'))

        expect(mockGame.makeComputerMove).not.toHaveBeenCalled()
    })

    it('drops a throttled search request when the app unmounts', () => {
        vi.useFakeTimers()
        try {
            const { unmount } = render(<App />)
            const onMessage = vi.mocked(wisdomWindow.setReceiveWorkerMessageCallback).mock.calls[0][0]
            act(() => onMessage('computerMoved', 0, 'e2 e4'))
            act(() => onMessage('computerMoved', 0, 'e7 e5'))
            expect(mockGameModel.notifyComputerMove).toHaveBeenCalledTimes(1)

            unmount()
            vi.advanceTimersByTime(1000)

            expect(mockGameModel.notifyComputerMove).toHaveBeenCalledTimes(1)
        } finally {
            vi.useRealTimers()
        }
    })

    it('frees the settings object built when settings are applied', async () => {
        const user = userEvent.setup()
        render(<App />)

        await user.click(screen.getByText('Settings'))
        await user.click(screen.getByText('Apply'))

        const wasmSettings = vi.mocked(mockWisdomChess.GameSettings).mock.instances[0]
        expect(mockGameModel.setCurrentGameSettings).toHaveBeenCalledWith(wasmSettings)
        expect(mockWisdomChess.destroy).toHaveBeenCalledWith(wasmSettings)
    })

    it('applies the choices made in the settings dialog', async () => {
        const user = userEvent.setup()
        render(<App />)

        await user.click(screen.getByText('Settings'))
        await user.click(screen.getAllByLabelText('Computer')[0])
        await user.click(screen.getAllByLabelText('Human')[1])
        await user.click(document.querySelector('input[name="debugLogging"]')!)
        await user.click(document.querySelector('input[name="flipped"]')!)
        await user.click(screen.getByText('Apply'))

        const wasmSettings = vi.mocked(mockWisdomChess.GameSettings).mock.instances[0]
        expect(wasmSettings.whitePlayer).toBe(mockWisdomChess.ChessEngine)
        expect(wasmSettings.blackPlayer).toBe(mockWisdomChess.Human)
        expect(wasmSettings.debugLogging).toBe(true)
        expect(document.querySelector('.board.flipped')).not.toBeNull()
    })
})

describe('withWasmObjects', () => {
    let mockWisdomChess: WisdomChess

    beforeEach(() => {
        mockWisdomChess = createMockWisdomChess()
        const wisdomWindow = getWisdomWindow()
        wisdomWindow.wisdomChessWeb = mockWisdomChess
    })

    it('frees objects added while the callback runs and returns its result', () => {
        const owned: WasmObject[] = []
        const first = {} as WasmObject
        const second = {} as WasmObject

        const result = withWasmObjects(owned, () => {
            owned.push(first)
            owned.push(second)
            return 42
        })

        expect(result).toBe(42)
        expect(mockWisdomChess.destroy).toHaveBeenCalledWith(first)
        expect(mockWisdomChess.destroy).toHaveBeenCalledWith(second)
    })

    it('frees objects when the callback throws', () => {
        const object = {} as WasmObject

        expect(() => withWasmObjects([object], () => {
            throw new Error('failed')
        })).toThrow('failed')

        expect(mockWisdomChess.destroy).toHaveBeenCalledWith(object)
    })
})
