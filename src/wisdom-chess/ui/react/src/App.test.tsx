import { describe, it, expect, beforeEach, vi } from 'vitest'
import { render, screen } from '@testing-library/react'
import userEvent from '@testing-library/user-event'
import App from './App'
import type { WisdomWindow, Game, GameModel, WisdomChess } from './lib/WisdomChess'

const createMockGame = (): Game => ({
    getGameStatus: vi.fn(() => 0),
    getMoveStatus: vi.fn(() => 'White to move'),
    getGameOverStatus: vi.fn(() => ''),
    getCurrentTurn: vi.fn(() => 0),
    inCheck: vi.fn(() => false),
    getPieceList: vi.fn(() => ({
        length: 0,
        pieceAt: vi.fn(() => null),
    })),
    isLegalMove: vi.fn(() => true),
    makeMove: vi.fn(),
    needsPawnPromotion: vi.fn(() => false),
    createMoveFromCoordinatesAndPromotedPiece: vi.fn(),
    getPlayerOfColor: vi.fn(() => 0),
    setSettings: vi.fn(),
    setHumanDrawStatus: vi.fn(),
    setComputerDrawStatus: vi.fn(),
} as unknown as Game)

const createMockGameModel = (): GameModel => ({
    startNewGame: vi.fn(() => createMockGame()),
    getCurrentGameSettings: vi.fn(() => ({
        whitePlayer: 0,
        blackPlayer: 1,
        thinkingTime: 5,
        searchDepth: 4,
    })),
    getFirstHumanPlayerColor: vi.fn(() => 0),
    getSecondHumanPlayerColor: vi.fn(() => 2),
    setCurrentGameSettings: vi.fn(),
    notifyHumanMove: vi.fn(),
    notifyComputerMove: vi.fn(),
    sendPause: vi.fn(),
    sendUnpause: vi.fn(),
} as unknown as GameModel)

const createMockWisdomChess = (): WisdomChess => ({
    White: 0,
    Black: 1,
    NoColor: 2,
    Human: 0,
    ChessEngine: 1,
    Queen: 0,
    Rook: 1,
    Bishop: 2,
    Knight: 3,
    Pawn: 5,
    NoPiece: 6,
    Playing: 0,
    Checkmate: 1,
    Stalemate: 2,
    ThreefoldRepetitionReached: 3,
    FiftyMovesWithoutProgressReached: 4,
    DrawAccepted: 5,
    DrawDeclined: 6,
    ThreefoldRepetition: 0,
    FiftyMovesWithoutProgress: 1,
    GameSettings: vi.fn(function (this: any) {
        this.whitePlayer = 0
        this.blackPlayer = 1
        this.thinkingTime = 5
        this.searchDepth = 4
    }) as any,
    WebMove: {
        prototype: {
            fromString: vi.fn(),
        },
    } as any,
    WebCoord: {
        prototype: {
            fromTextCoord: vi.fn(() => ({})),
        },
    } as any,
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

        const wisdomWindow = window as unknown as WisdomWindow
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
        const wisdomWindow = window as unknown as WisdomWindow
        const setCallbackSpy = vi.fn()
        wisdomWindow.setReceiveWorkerMessageCallback = setCallbackSpy

        render(<App />)

        expect(setCallbackSpy).toHaveBeenCalled()
        expect(typeof setCallbackSpy.mock.calls[0][0]).toBe('function')
    })
})
