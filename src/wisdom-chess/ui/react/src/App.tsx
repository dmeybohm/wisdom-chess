import React, { useEffect, useMemo, useReducer, useRef, useCallback, useState } from 'react'
import './App.css'
import Board from './Board'
import TopMenu from './TopMenu'
import StatusBar from './StatusBar'
import { SettingsModal } from './SettingsModal'
import { DrawDialog } from './DrawDialog'
import { AboutModal } from './AboutModal'
import {
    WisdomChess,
    getCurrentGame,
    getGameModel,
    getPieces,
    getCurrentGameSettings,
    startNewGame as startNewGameEngine,
    GameStatus,
    PieceColor,
    PieceType,
    WebMove,
    DrawByRepetitionType,
    fromColorToNumber,
    ReactWindow,
    ChessEngineEventType,
    GameState,
    WebGameSettings
} from './lib/WisdomChess'
import Modal from "./Modal";
import {initialSquares} from "./lib/Squares";

type Action =
    | { type: 'BOOTSTRAP'; snapshot: Partial<GameState> }
    | { type: 'ENGINE_SYNC'; snapshot: Partial<GameState> }
    | { type: 'FOCUS'; square: string }
    | { type: 'CLEAR_FOCUS' }
    | { type: 'REQUEST_PROMOTION'; src: string; dst: string }
    | { type: 'SET_SETTINGS'; settings: WebGameSettings }
    | { type: 'SET_LAST_DROPPED'; square: string }

function reducer(state: GameState, action: Action): GameState {
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

function toWebSettings(wasm: any): WebGameSettings {
    return {
        whitePlayer: wasm.whitePlayer,
        blackPlayer: wasm.blackPlayer,
        thinkingTime: wasm.thinkingTime,
        searchDepth: wasm.searchDepth,
    }
}

function snapshotFromEngine() {
    const game = getCurrentGame()
    const gameModel = getGameModel()
    const WC = WisdomChess()
    return {
        pieces: getPieces(game),
        gameStatus: game.getGameStatus(),
        moveStatus: game.getMoveStatus(),
        gameOverStatus: game.getGameOverStatus(),
        hasHumanPlayer: gameModel.getFirstHumanPlayerColor() !== WC.NoColor,
    }
}

function throttle<T extends (...args: any[]) => void>(func: T, limit: number) {
    let lastTimeout: ReturnType<typeof setTimeout> | null = null
    let lastRan: number | null = null
    const throttled = (...args: Parameters<T>) => {
        const now = Date.now()
        if (lastRan === null) {
            func(...args)
            lastRan = now
            return
        }
        const diff = limit - (now - lastRan)
        if (lastTimeout !== null) clearTimeout(lastTimeout)
        if (diff <= 0) {
            func(...args)
            lastRan = now
            lastTimeout = null
            return
        }
        lastTimeout = setTimeout(() => {
            func(...args)
            lastRan = Date.now()
            lastTimeout = null
        }, diff)
    }
    ;(throttled as any).cancel = () => {
        if (lastTimeout !== null) clearTimeout(lastTimeout)
        lastTimeout = null
    }
    return throttled as T & { cancel: () => void }
}


function App() {
    // Engine refs (imperative, no re-renders)
    const gameRef = useRef(getCurrentGame())
    const modelRef = useRef(getGameModel())
    const wisdomChessRef = useRef(WisdomChess())

    const [flipped, setFlipped] = useState(false)
    const [showNewGame, setShowNewGame] = useState(false)
    const [showSettings, setShowSettings] = useState(false)
    const [showAbout, setShowAbout] = useState(false)

    const [state, dispatch] = useReducer(reducer, {
        pieces: [],
        squares: initialSquares,
        focusedSquare: '',
        pawnPromotionDialogSquare: '',
        lastDroppedSquare: '',
        gameStatus: 0 as GameStatus,
        gameOverStatus: '',
        moveStatus: 'White to move',
        settings: toWebSettings(getCurrentGameSettings()),
        hasHumanPlayer: false,
    })

    const currentTurn = gameRef.current.getCurrentTurn()
    const inCheck = gameRef.current.inCheck

    // Bootstrap + engine message hookup:
    useEffect(() => {
        dispatch({ type: 'BOOTSTRAP', snapshot: snapshotFromEngine() })

        const w = (window as unknown) as ReactWindow
        if (w.setReceiveWorkerMessageCallback) {
            const onMsg = (type: ChessEngineEventType, gameId: number, message: string) => {
                const game = gameRef.current
                const mod = wisdomChessRef.current
                switch (type) {
                    case 'computerMoved': {
                        const move = mod.WebMove.prototype.fromString(message, game.getCurrentTurn())
                        game.makeMove(move)
                        throttledComputerMove()
                        dispatch({ type: 'ENGINE_SYNC', snapshot: snapshotFromEngine() })
                        break
                    }
                    case 'computerDrawStatusUpdated': {
                        const params = JSON.parse(message) as {
                            draw_type: DrawByRepetitionType
                            color: PieceColor
                            accepted: boolean
                        }
                        game.setComputerDrawStatus(params.draw_type, params.color, params.accepted)
                        dispatch({ type: 'ENGINE_SYNC', snapshot: snapshotFromEngine() })
                        break
                    }
                    default: {
                        console.error('Unknown message type', type)
                    }
                }
            }
            w.setReceiveWorkerMessageCallback(onMsg)
            return () => w.setReceiveWorkerMessageCallback(() => {})
        }
    }, [])

    // Throttled engine "computer vs computer" tick
    const throttledComputerMove = useMemo(
        () => throttle(() => modelRef.current.notifyComputerMove(), 250),
        [],
    )

    // Modal Pausing
    useEffect(() => {
        const anyOpen = [showAbout, showNewGame, showSettings].some(Boolean)
        if (anyOpen) modelRef.current.sendPause()
        else modelRef.current.sendUnpause()
    }, [showAbout, showNewGame, showSettings])

    // ----- UI Handlers → Engine adapter -----

    const applyHumanMove = useCallback(
        (src: string, dst: string, promote?: PieceType) => {
            const game = gameRef.current
            const mod = wisdomChessRef.current
            const model = modelRef.current

            if (!src) {
                // Clear promotion dialog if any
                dispatch({ type: 'ENGINE_SYNC', snapshot: snapshotFromEngine() })
                return
            }

            const srcCoord = mod.WebCoord.prototype.fromTextCoord(src)
            const dstCoord = mod.WebCoord.prototype.fromTextCoord(dst)

            if (game.needsPawnPromotion(srcCoord, dstCoord) && !promote) {
                dispatch({ type: 'REQUEST_PROMOTION', src, dst })
                return
            }

            const pieceType = promote ?? mod.Queen
            let move: WebMove | null = null
            try {
                move = game.createMoveFromCoordinatesAndPromotedPiece(srcCoord, dstCoord, pieceType)
            } catch {
                // illegal
            }
            if (!move || !game.isLegalMove(move)) {
                dispatch({ type: 'ENGINE_SYNC', snapshot: snapshotFromEngine() })
                return
            }

            game.makeMove(move)
            model.notifyHumanMove(move)
            dispatch({ type: 'ENGINE_SYNC', snapshot: snapshotFromEngine() })
        },
        [],
    )

    function handleDropPiece(src: string, dst: string) {
        if (src === dst) return
        // Pass src explicitly to avoid stale state issues
        applyHumanMove(src, dst)
        dispatch({ type: 'SET_LAST_DROPPED', square: dst })
    }

    function handlePieceClick(dst: string) {
        const game = gameRef.current
        const wisdomChess = wisdomChessRef.current
        if (state.gameStatus !== wisdomChessRef.current.Playing) {
            return
        }

        if (state.focusedSquare === '') {
            const currentPieces = getPieces(game)
            const srcPiece = currentPieces.find(p => p.position === dst)
            if (!srcPiece) {
                console.error(`Couldn't find piece coord: ${dst}`)
                return
            }
            const colorNum = fromColorToNumber(srcPiece.color)
            if (srcPiece && game.getPlayerOfColor(colorNum) === wisdomChess.Human) {
                dispatch({ type: 'FOCUS', square: dst })
            }
            return
        }

        if (state.focusedSquare === dst) {
            dispatch({ type: 'CLEAR_FOCUS' })
            return
        }

        // If same-color piece, switch focus; otherwise try move
        const currentPieces = getPieces(game)
        const dstPiece = currentPieces.find(p => p.position === dst)
        const srcPiece = currentPieces.find(p => p.position === state.focusedSquare)
        if (!dstPiece || !srcPiece || srcPiece.color === dstPiece.color) {
            dispatch({ type: 'FOCUS', square: dst })
            return
        }

        applyHumanMove(state.focusedSquare, dst)
    }

    function handlePromote(pieceType: PieceType) {
        if (!state.focusedSquare || !state.pawnPromotionDialogSquare) return
        applyHumanMove(state.focusedSquare, state.pawnPromotionDialogSquare, pieceType)
        dispatch({ type: 'SET_LAST_DROPPED', square: '' })
    }

    function handleApplySettings(gameSettings: WebGameSettings, flipped: boolean) {
        setShowSettings(false)
        setFlipped(flipped)

        const wisdomChess = wisdomChessRef.current
        const wasmGameSettings = new wisdomChess.GameSettings()
        wasmGameSettings.whitePlayer = gameSettings.whitePlayer
        wasmGameSettings.blackPlayer = gameSettings.blackPlayer
        wasmGameSettings.thinkingTime = gameSettings.thinkingTime
        wasmGameSettings.searchDepth = gameSettings.searchDepth

        modelRef.current.setCurrentGameSettings(wasmGameSettings)
        gameRef.current.setSettings(wasmGameSettings)

        dispatch({ type: 'SET_SETTINGS', settings: gameSettings })
        dispatch({ type: 'ENGINE_SYNC', snapshot: snapshotFromEngine() })
    }

    function startNewGame(e: React.SyntheticEvent) {
        e.preventDefault()
        startNewGameEngine()
        dispatch({ type: 'CLEAR_FOCUS' })
        dispatch({
            type: 'ENGINE_SYNC',
            snapshot: { ...snapshotFromEngine(), settings: toWebSettings(getCurrentGameSettings()) },
        })
        setShowNewGame(false)
    }

    function setHumanDrawStatus(
        drawType: DrawByRepetitionType,
        who: PieceColor,
        accepted: boolean
    ) {
        const wisdomChess = wisdomChessRef.current
        const game = gameRef.current
        const model = modelRef.current

        const first = model.getFirstHumanPlayerColor()
        const second = model.getSecondHumanPlayerColor()
        if (first === wisdomChess.NoColor) return

        game.setHumanDrawStatus(drawType, first, accepted)
        if (second !== wisdomChess.NoColor) {
            game.setHumanDrawStatus(drawType, second, accepted)
        }

        dispatch({ type: 'ENGINE_SYNC', snapshot: snapshotFromEngine() })
    }

    const [thirdRepetitionDrawAnswered, setThirdRepetitionDrawAnswered] = useState(false)
    const [fiftyMovesDrawAnswered, setFiftyMovesDrawAnswered] = useState(false)

    const handleThirdRepetitionDrawAnswer = (answer: boolean) => {
        setThirdRepetitionDrawAnswered(true)
        setHumanDrawStatus(WisdomChess().ThreefoldRepetition, WisdomChess().White, answer)
    }
    const handleFiftyMovesWithoutProgressDrawAnswer = (answer: boolean) => {
        setFiftyMovesDrawAnswered(true)
        setHumanDrawStatus(WisdomChess().FiftyMovesWithoutProgress, WisdomChess().White, answer)
    }

    // ----- Render -----
    const showModalOverlay = showSettings || showAbout || showNewGame

    return (
        <div className="App">
            <TopMenu
                newGameClicked={() => setShowNewGame(true)}
                settingsClicked={() => setShowSettings(true)}
                aboutClicked={() => setShowAbout(true)}
            />

            <div className="container">
                <Board
                    flipped={flipped}
                    currentTurn={currentTurn}
                    squares={state.squares}
                    focusedSquare={state.focusedSquare}
                    pieces={state.pieces}
                    droppedSquare={state.lastDroppedSquare}
                    pawnPromotionDialogSquare={state.pawnPromotionDialogSquare}
                    onMovePiece={(dst) => applyHumanMove(state.focusedSquare, dst)}
                    onDropPiece={handleDropPiece}
                    onPieceClick={handlePieceClick}
                    onPiecePromotion={handlePromote}
                />

                <StatusBar
                    currentTurn={currentTurn}
                    inCheck={inCheck}
                    moveStatus={state.moveStatus}
                    gameOverStatus={state.gameOverStatus}
                />
            </div>

            {showNewGame && (
                <Modal>
                    <h1>New Game</h1>
                    <p>Start a new Game?</p>
                    <div className="buttons">
                        <button type="button" className="btn-highlight" onClick={startNewGame}>Start New Game</button>
                        <button type="button" onClick={() => setShowNewGame(false)}>Cancel</button>
                    </div>
                </Modal>
            )}

            {showSettings && (
                <SettingsModal
                    flipped={flipped}
                    settings={state.settings}
                    onApply={handleApplySettings}
                    onDismiss={() => setShowSettings(false)}
                />
            )}

            {showAbout && (
                <AboutModal onClick={() => setShowAbout(false)} />
            )}

            {state.gameStatus === WisdomChess().ThreefoldRepetitionReached &&
                !thirdRepetitionDrawAnswered && state.hasHumanPlayer && (
                <DrawDialog
                    title={'Third Repetition Reached'}
                    onAccepted={() => handleThirdRepetitionDrawAnswer(true)}
                    onDeclined={() => handleThirdRepetitionDrawAnswer(false)}
                >
                    <p>The same position was reached three times. Either player can declare a draw now.</p>
                </DrawDialog>
            )}

            {state.gameStatus === WisdomChess().FiftyMovesWithoutProgressReached &&
                !fiftyMovesDrawAnswered && state.hasHumanPlayer && (
                <DrawDialog
                    title={'Fifty Moves Without Progress'}
                    onAccepted={() => handleFiftyMovesWithoutProgressDrawAnswer(true)}
                    onDeclined={() => handleFiftyMovesWithoutProgressDrawAnswer(false)}
                >
                    <p>Fifty moves without any capture or pawn movement. Either player can declare a draw now.</p>
                </DrawDialog>
            )}

            {showModalOverlay && (
                <div className="modal-overlay"></div>
            )}
        </div>
    )
}

export default App
