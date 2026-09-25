import React, { useEffect, useMemo, useReducer, useRef, useState } from 'react'
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
    withWasmObjects,
    startNewGame as startNewGameEngine,
    PieceColor,
    PieceType,
    DrawByRepetitionType,
    ILLEGAL_MOVE,
    getWisdomWindow,
    ChessEngineEventType,
    Game,
    GameStatus,
    WebGameSettings
} from './lib/WisdomChess'
import Modal from "./Modal";
import { EngineSnapshot, initialState, reducer } from './reducer'

function snapshotFromEngine(game: Game): EngineSnapshot {
    const WC = WisdomChess()
    return {
        pieces: getPieces(game),
        currentTurn: game.getCurrentTurn(),
        inCheck: game.getInCheck(),
        gameStatus: game.getGameStatus(),
        moveStatus: game.getMoveStatus(),
        gameOverStatus: game.getGameOverStatus(),
        hasHumanPlayer: getGameModel().getFirstHumanPlayerColor() !== WC.NoColor,
    }
}

function drawOfferFor(gameStatus: GameStatus) {
    const WC = WisdomChess()
    switch (gameStatus) {
        case WC.ThreefoldRepetitionReached:
            return {
                drawType: WC.ThreefoldRepetition,
                title: 'Third Repetition Reached',
                message: 'The same position was reached three times. Either player can declare a draw now.',
            }
        case WC.FiftyMovesWithoutProgressReached:
            return {
                drawType: WC.FiftyMovesWithoutProgress,
                title: 'Fifty Moves Without Progress',
                message: 'Fifty moves without any capture or pawn movement. Either player can declare a draw now.',
            }
        default:
            return null
    }
}

function throttle<T extends (...args: never[]) => void>(func: T, limit: number) {
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
    const cancel = () => {
        if (lastTimeout !== null) clearTimeout(lastTimeout)
        lastTimeout = null
    }
    return Object.assign(throttled, { cancel })
}


function App() {
    const gameRef = useRef(getCurrentGame())

    const [flipped, setFlipped] = useState(false)
    const [showNewGame, setShowNewGame] = useState(false)
    const [settings, setSettings] = useState<WebGameSettings | null>(null)
    const [showAbout, setShowAbout] = useState(false)
    const [answeredDraws, setAnsweredDraws] = useState<DrawByRepetitionType[]>([])

    const [state, dispatch] = useReducer(
        reducer,
        null,
        () => initialState(snapshotFromEngine(gameRef.current)),
    )

    const sync = () => dispatch({ type: 'SYNC', snapshot: snapshotFromEngine(gameRef.current) })

    // Throttled engine "computer vs computer" tick
    const throttledComputerMove = useMemo(
        () => throttle(() => getGameModel().notifyComputerMove(), 250),
        [],
    )
    useEffect(() => () => throttledComputerMove.cancel(), [throttledComputerMove])

    useEffect(() => {
        const w = getWisdomWindow()
        if (w.setReceiveWorkerMessageCallback) {
            const onMsg = (type: ChessEngineEventType, gameId: number, message: string) => {
                const game = gameRef.current

                // Reject stale moves from previous games
                if (gameId !== game.getGameId()) {
                    console.debug('Ignoring message from old game:', gameId, 'current:', game.getGameId())
                    return
                }

                switch (type) {
                    case 'computerMoved': {
                        game.makeComputerMove(message)
                        throttledComputerMove()
                        break
                    }
                    case 'computerDrawStatusUpdated': {
                        const params = JSON.parse(message) as {
                            draw_type: DrawByRepetitionType
                            color: PieceColor
                            accepted: boolean
                        }
                        game.setComputerDrawStatus(params.draw_type, params.color, params.accepted)
                        break
                    }
                    default: {
                        console.error('Unknown message type', type)
                        return
                    }
                }
                dispatch({ type: 'SYNC', snapshot: snapshotFromEngine(game) })
            }
            w.setReceiveWorkerMessageCallback(onMsg)
            return () => w.setReceiveWorkerMessageCallback(() => {})
        }
    }, [throttledComputerMove])

    // Modal Pausing
    const anyModalOpen = showAbout || showNewGame || settings !== null
    useEffect(() => {
        if (anyModalOpen) getGameModel().sendPause()
        else getGameModel().sendUnpause()
    }, [anyModalOpen])

    // ----- UI Handlers → Engine adapter -----

    function applyHumanMove(src: string, dst: string, promote?: PieceType) {
        const game = gameRef.current
        if (!src) return

        if (game.needsPawnPromotion(src, dst) && !promote) {
            dispatch({ type: 'REQUEST_PROMOTION', src, dst })
            return
        }

        const move = game.makeHumanMove(src, dst, promote ?? WisdomChess().Queen)
        if (move !== ILLEGAL_MOVE) {
            getGameModel().notifyHumanMove(move)
        }
        dispatch({ type: 'CLEAR_FOCUS' })
        sync()
    }

    function handleDropPiece(src: string, dst: string) {
        if (src === dst) return
        applyHumanMove(src, dst)
        dispatch({ type: 'SET_LAST_DROPPED', square: dst })
    }

    function handlePieceClick(dst: string) {
        const game = gameRef.current
        const wisdomChess = WisdomChess()
        if (state.gameStatus !== wisdomChess.Playing) {
            return
        }

        if (state.focusedSquare === '') {
            const srcPiece = state.pieces.find(p => p.position === dst)
            if (!srcPiece) {
                console.error(`Couldn't find piece coord: ${dst}`)
                return
            }
            if (game.getPlayerOfColor(srcPiece.color) === wisdomChess.Human) {
                dispatch({ type: 'FOCUS', square: dst })
            }
            return
        }

        if (state.focusedSquare === dst) {
            dispatch({ type: 'CLEAR_FOCUS' })
            return
        }

        // If same-color piece, switch focus; otherwise try move
        const dstPiece = state.pieces.find(p => p.position === dst)
        const srcPiece = state.pieces.find(p => p.position === state.focusedSquare)
        if (!dstPiece || !srcPiece || srcPiece.color === dstPiece.color) {
            dispatch({ type: 'FOCUS', square: dst })
            return
        }

        applyHumanMove(state.focusedSquare, dst)
    }

    function handlePromote(pieceType: PieceType) {
        if (!state.focusedSquare || !state.pawnPromotionDialogSquare) return
        applyHumanMove(state.focusedSquare, state.pawnPromotionDialogSquare, pieceType)
    }

    function handleApplySettings(gameSettings: WebGameSettings, flipped: boolean) {
        setSettings(null)
        setFlipped(flipped)

        const wasmGameSettings = new (WisdomChess().GameSettings)()
        withWasmObjects([wasmGameSettings], () => {
            wasmGameSettings.whitePlayer = gameSettings.whitePlayer
            wasmGameSettings.blackPlayer = gameSettings.blackPlayer
            wasmGameSettings.thinkingTime = gameSettings.thinkingTime
            wasmGameSettings.searchDepth = gameSettings.searchDepth
            wasmGameSettings.debugLogging = gameSettings.debugLogging

            getGameModel().setCurrentGameSettings(wasmGameSettings)
            gameRef.current.setSettings(wasmGameSettings)
        })

        sync()
    }

    function startNewGame() {
        throttledComputerMove.cancel()
        gameRef.current = startNewGameEngine()

        dispatch({ type: 'CLEAR_FOCUS' })
        sync()
        setAnsweredDraws([])
        setShowNewGame(false)
    }

    function answerDraw(drawType: DrawByRepetitionType, accepted: boolean) {
        const game = gameRef.current
        const model = getGameModel()
        const noColor = WisdomChess().NoColor
        setAnsweredDraws(answered => [...answered, drawType])

        const first = model.getFirstHumanPlayerColor()
        const second = model.getSecondHumanPlayerColor()
        if (first === noColor) return

        game.setHumanDrawStatus(drawType, first, accepted)
        if (second !== noColor) {
            game.setHumanDrawStatus(drawType, second, accepted)
        }
        sync()
    }

    // ----- Render -----
    const drawOffer = state.hasHumanPlayer ? drawOfferFor(state.gameStatus) : null

    return (
        <div className="App">
            <TopMenu
                newGameClicked={() => setShowNewGame(true)}
                settingsClicked={() => setSettings(getCurrentGameSettings())}
                aboutClicked={() => setShowAbout(true)}
            />

            <div className="container">
                <Board
                    flipped={flipped}
                    currentTurn={state.currentTurn}
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
                    currentTurn={state.currentTurn}
                    inCheck={state.inCheck}
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

            {settings && (
                <SettingsModal
                    flipped={flipped}
                    settings={settings}
                    onApply={handleApplySettings}
                    onDismiss={() => setSettings(null)}
                />
            )}

            {showAbout && (
                <AboutModal onClick={() => setShowAbout(false)} />
            )}

            {drawOffer && !answeredDraws.includes(drawOffer.drawType) && (
                <DrawDialog
                    title={drawOffer.title}
                    onAccepted={() => answerDraw(drawOffer.drawType, true)}
                    onDeclined={() => answerDraw(drawOffer.drawType, false)}
                >
                    <p>{drawOffer.message}</p>
                </DrawDialog>
            )}
        </div>
    )
}

export default App
