import React, { useEffect, useState, useCallback } from 'react'
import './App.css'
import Board from "./Board";
import TopMenu from "./TopMenu";
import StatusBar from "./StatusBar";
import Modal from "./Modal";
import {
    getCurrentGame,
    WisdomChess,
    GameSettings,
    GameStatus,
    PieceColor,
    PieceType,
    WebMove,
    getGameModel,
    getPieces,
    getCurrentGameSettings,
    startNewGame as startNewGameEngine,
    Game,
    DrawByRepetitionType,
    fromColorToNumber,
} from "./lib/WisdomChess";
import { SettingsModal } from "./SettingsModal";
import { DrawDialog } from "./DrawDialog";
import { AboutModal } from "./AboutModal";
import { Piece } from "./lib/Pieces";
import { initialSquares } from "./lib/Squares";

export let wisdomChess : any = undefined

type ChessEngineEventType = 'computerMoved' | 'computerDrawStatusUpdated'

type ComputerDrawStatusUpdate = {
    draw_type: DrawByRepetitionType
    color: PieceColor
    accepted: boolean
}

function updateGameState(currentGame: Game) {
    const gameModel = getGameModel()
    const wisdomChessModule = WisdomChess()
    return {
        pieces: getPieces(currentGame),
        gameStatus: currentGame.getGameStatus(),
        moveStatus: currentGame.getMoveStatus(),
        gameOverStatus: currentGame.getGameOverStatus(),
        hasHumanPlayer: gameModel.getFirstHumanPlayerColor() !== wisdomChessModule.NoColor,
    }
}

function throttle(func: Function, limit: number) {
    let lastTimeout : ReturnType<typeof setTimeout>|null = null;
    let lastRan : number|null = null;

    return function() {
        if (lastRan === null) {
            func()
            lastRan = Date.now();
            return;
        }
        const diff = limit - (Date.now() - lastRan);
        clearTimeout(lastTimeout!);
        lastTimeout = null;
        if (diff <= 0) {
            func()
            lastRan = Date.now();
            return;
        }
        lastTimeout = setTimeout(function() {
            func()
            lastRan = Date.now();
            lastTimeout = null;
        }, diff);
    }
}

function findPieceAtPosition(pieces: Piece[], position: string): Piece|undefined {
    return pieces.find(piece => piece.position === position)
}

function App() {

    wisdomChess = WisdomChess()

    const [flipped, setFlipped] = useState(false);
    const [showAbout, setShowAbout] = useState(false);
    const [showNewGame, setShowNewGame] = useState(false);
    const [showSettings, setShowSettings] = useState(false);

    const [thirdRepetitionDrawAnswered, setThirdRepetitionDrawAnswered] = useState(false)
    const [fiftyMovesDrawAnswered, setFiftyMovesDrawAnswered] = useState(false)

    const [pieces, setPieces] = useState<Piece[]>([])
    const [squares] = useState(initialSquares)
    const [focusedSquare, setFocusedSquare] = useState('')
    const [hasHumanPlayer, setHasHumanPlayer] = useState(false)
    const [pawnPromotionDialogSquare, setPawnPromotionDialogSquare] = useState('')
    const [gameStatus, setGameStatus] = useState<GameStatus>(0)
    const [gameOverStatus, setGameOverStatus] = useState('')
    const [moveStatus, setMoveStatus] = useState('White to move')
    const [settings, setSettings] = useState<GameSettings>({
        whitePlayer: 0,
        blackPlayer: 1,
        thinkingTime: 2,
        searchDepth: 3,
    })
    const [lastDroppedSquare, setLastDroppedSquare] = useState('')

    function updateState(updates: Partial<ReturnType<typeof updateGameState>> & {
        focusedSquare?: string
        pawnPromotionDialogSquare?: string
        lastDroppedSquare?: string
        settings?: GameSettings
    }) {
        if (updates.pieces !== undefined) setPieces(updates.pieces)
        if (updates.gameStatus !== undefined) setGameStatus(updates.gameStatus)
        if (updates.moveStatus !== undefined) setMoveStatus(updates.moveStatus)
        if (updates.gameOverStatus !== undefined) setGameOverStatus(updates.gameOverStatus)
        if (updates.hasHumanPlayer !== undefined) setHasHumanPlayer(updates.hasHumanPlayer)
        if (updates.focusedSquare !== undefined) setFocusedSquare(updates.focusedSquare)
        if (updates.pawnPromotionDialogSquare !== undefined) setPawnPromotionDialogSquare(updates.pawnPromotionDialogSquare)
        if (updates.lastDroppedSquare !== undefined) setLastDroppedSquare(updates.lastDroppedSquare)
        if (updates.settings !== undefined) setSettings(updates.settings)
    }

    const throttledComputerMove = useCallback(throttle(() => getGameModel().notifyComputerMove(), 250), [])

    const handleAboutClicked = (event: React.SyntheticEvent): void => {
        event.preventDefault()
        setShowAbout(true)
        resetFocusState()
    }

    const handleNewGameClicked = (event: React.SyntheticEvent): void => {
        event.preventDefault()
        setShowNewGame(true)
        resetFocusState()
    }

    const handleSettingsClicked = (event: React.SyntheticEvent): void => {
        event.preventDefault()
        setShowSettings(true)
        resetFocusState()
    }

    const game = getCurrentGame()

    function resetFocusState() {
        setFocusedSquare('')
        setPawnPromotionDialogSquare('')
    }

    function computerMovePiece(move: WebMove) {
        const currentGame = getCurrentGame()
        currentGame.makeMove(move);
        updateState({
            ...updateGameState(currentGame),
            lastDroppedSquare: ''
        })
    }

    const receiveWorkerMessage = useCallback((type: ChessEngineEventType, gameId: number, message: string) => {
        const currentGame = getCurrentGame()
        switch (type) {
            case 'computerMoved': {
                const move = wisdomChess.WebMove.prototype.fromString(
                    message,
                    currentGame.getCurrentTurn()
                );
                computerMovePiece(move)
                throttledComputerMove()
                break;
            }

            case 'computerDrawStatusUpdated': {
                const params = JSON.parse(message) as ComputerDrawStatusUpdate
                currentGame.setComputerDrawStatus(
                    params.draw_type,
                    params.color,
                    params.accepted
                )
                updateState({
                    ...updateGameState(currentGame)
                })
                break;
            }

            default: {
                console.error("Unknown message type: ", type);
                break;
            }
        }
    }, [throttledComputerMove])

    function humanMovePiece(dst: string) {
        const currentGame = getCurrentGame()
        const gameModel = getGameModel()
        const src = focusedSquare;

        if (src === '') {
            setPawnPromotionDialogSquare('')
            return
        }
        const srcCoord = wisdomChess.WebCoord.prototype.fromTextCoord(src)
        const dstCoord = wisdomChess.WebCoord.prototype.fromTextCoord(dst)

        let move;
        try {
            move = currentGame.createMoveFromCoordinatesAndPromotedPiece(
                srcCoord,
                dstCoord,
                wisdomChess.Queen
            );
        } catch (e) {
            console.error("Error:", e)
            updateState({
                ...updateGameState(currentGame),
                pawnPromotionDialogSquare: '',
                focusedSquare: '',
            })
            return
        }
        if (!move || !currentGame.isLegalMove(move)) {
            updateState({
                ...updateGameState(currentGame),
                pawnPromotionDialogSquare: '',
                focusedSquare: '',
            })
            return
        }
        if (currentGame.needsPawnPromotion(srcCoord, dstCoord)) {
            updateState({
                ...updateGameState(currentGame),
                pawnPromotionDialogSquare: dst,
                focusedSquare: src,
            })
            return
        }
        currentGame.makeMove(move)
        gameModel.notifyHumanMove(move)
        updateState({
            ...updateGameState(currentGame),
            focusedSquare: ''
        })
    }

    function promotePiece(pieceType: PieceType) {
        const currentGame = getCurrentGame()
        const gameModel = getGameModel()
        const src = focusedSquare
        const dst = pawnPromotionDialogSquare

        const srcCoord = wisdomChess.WebCoord.prototype.fromTextCoord(src)
        const dstCoord = wisdomChess.WebCoord.prototype.fromTextCoord(dst)

        let move;
        try {
            move = currentGame.createMoveFromCoordinatesAndPromotedPiece(
                srcCoord,
                dstCoord,
                pieceType,
            );
        } catch (e) {
            console.error("Error:", e)
            updateState({
                ...updateGameState(currentGame),
                pawnPromotionDialogSquare: '',
                focusedSquare: '',
                lastDroppedSquare: '',
            })
            return
        }
        if (!move || !currentGame.isLegalMove(move)) {
            updateState({
                ...updateGameState(currentGame),
                pawnPromotionDialogSquare: '',
                focusedSquare: '',
                lastDroppedSquare: '',
            })
            return
        }
        currentGame.makeMove(move)
        gameModel.notifyHumanMove(move)
        updateState({
            ...updateGameState(currentGame),
            focusedSquare: '',
            pawnPromotionDialogSquare: '',
            lastDroppedSquare: '',
        })
    }

    function pieceClick(dst: string) {
        const currentGame = getCurrentGame()

        if (gameStatus != wisdomChess.Playing) {
            return
        }

        if (focusedSquare === '') {
            const currentPieces = getPieces(currentGame)
            const srcPiece = findPieceAtPosition(currentPieces, dst)
            if (srcPiece && currentGame.getPlayerOfColor(fromColorToNumber(srcPiece.color)) == wisdomChess.Human) {
                updateState({
                    ...updateGameState(currentGame),
                    focusedSquare: dst,
                    lastDroppedSquare: '',
                    pawnPromotionDialogSquare: ''
                })
            }
            return
        }

        if (focusedSquare === dst) {
            updateState({
                ...updateGameState(currentGame),
                focusedSquare: '',
                lastDroppedSquare: '',
                pawnPromotionDialogSquare: ''
            })
            return
        }

        const currentPieces = getPieces(currentGame)
        const dstPiece = findPieceAtPosition(currentPieces, dst)
        const srcPiece = findPieceAtPosition(currentPieces, focusedSquare)
        if (!dstPiece || !srcPiece || srcPiece.color === dstPiece.color) {
            updateState({
                ...updateGameState(currentGame),
                focusedSquare: dst,
                lastDroppedSquare: '',
                pawnPromotionDialogSquare: ''
            })
            return
        }

        humanMovePiece(dst)
        if (pawnPromotionDialogSquare === '') {
            setLastDroppedSquare('')
            setFocusedSquare('')
        }
    }

    function dropPiece(src: string, dst: string) {
        setFocusedSquare(src)
        humanMovePiece(dst)
        updateState({
            ...updateGameState(getCurrentGame()),
            lastDroppedSquare: dst
        })
    }

    const pauseGame = useCallback(() => {
        getGameModel().sendPause()
    }, [])

    const unpauseGame = useCallback(() => {
        getGameModel().sendUnpause()
    }, [])

    function applySettings(newSettings: GameSettings) {
        const wisdomChessModule = WisdomChess()
        const workerGameSettings = new wisdomChessModule.GameSettings()
        const gameModel = getGameModel()

        workerGameSettings.whitePlayer = newSettings.whitePlayer
        workerGameSettings.blackPlayer = newSettings.blackPlayer
        workerGameSettings.thinkingTime = newSettings.thinkingTime
        workerGameSettings.searchDepth = newSettings.searchDepth

        gameModel.setCurrentGameSettings(workerGameSettings)
        const currentGame = getCurrentGame()
        currentGame.setSettings(workerGameSettings)

        updateState({
            ...updateGameState(currentGame),
            settings: workerGameSettings
        })
    }

    function setHumanDrawStatus(drawType: DrawByRepetitionType, who: PieceColor, accepted: boolean) {
        const currentGame = getCurrentGame()
        const gameModel = getGameModel()
        const firstHumanPlayer = gameModel.getFirstHumanPlayerColor()
        const secondHumanPlayer = gameModel.getSecondHumanPlayerColor()

        if (firstHumanPlayer === wisdomChess.NoColor) {
            console.error("Invalid color for human player draw type")
            return
        }
        currentGame.setHumanDrawStatus(drawType, firstHumanPlayer, accepted)
        if (secondHumanPlayer != wisdomChess.NoColor) {
            currentGame.setHumanDrawStatus(drawType, secondHumanPlayer, accepted);
        }
        updateState({
            ...updateGameState(getCurrentGame())
        })
    }

    useEffect(() => {
        updateState({
            settings: getCurrentGameSettings(),
            ...updateGameState(getCurrentGame())
        })

        const reactWindow = (window as any)
        if (reactWindow.setReceiveWorkerMessageCallback) {
            reactWindow.setReceiveWorkerMessageCallback(receiveWorkerMessage)
        }
    }, [receiveWorkerMessage])

    useEffect(() => {
        if ([showAbout, showNewGame, showSettings].some(v => v)) {
            pauseGame();
        } else {
            unpauseGame();
        }
    }, [showAbout, showNewGame, showSettings, pauseGame, unpauseGame])

    const handleThirdRepetitionDrawAnswer = (answer: boolean): void => {
        setThirdRepetitionDrawAnswered(true)
        setHumanDrawStatus(wisdomChess.ThreefoldRepetition, wisdomChess.White, answer);
    }
    const handleFiftyMovesWithoutProgressDrawAnswer = (answer: boolean): void => {
        setFiftyMovesDrawAnswered(true);
        setHumanDrawStatus(wisdomChess.FiftyMovesWithoutProgress, wisdomChess.White, answer);
    }

    const currentTurn = game.getCurrentTurn()
    const inCheck = game.inCheck

    const startNewGame = (event: React.SyntheticEvent) => {
        event.preventDefault()
        startNewGameEngine()
        updateState({
            focusedSquare: '',
            pawnPromotionDialogSquare: '',
            settings: getCurrentGameSettings(),
            ...updateGameState(getCurrentGame())
        })
        setShowNewGame(false)

        setThirdRepetitionDrawAnswered(false)
        setFiftyMovesDrawAnswered(false)
        resetFocusState();
    }

    function handleApplySettings(gameSettings: GameSettings, flipped: boolean) {
        setShowSettings(false);
        setFlipped(flipped)
        applySettings(gameSettings)
    }

    function handleCancelNewGame(e: React.SyntheticEvent) {
        e.preventDefault()
        setShowNewGame(false)
    }

    function handleDropPiece(src: string, dst: string) {
        if (src !== dst) {
            dropPiece(src, dst)
        }
    }

    function handlePieceClick(dst: string) {
        pieceClick(dst)
    }

    const showModalOverlay = showSettings || showAbout || showNewGame

    return (
        <div className="App">
            <TopMenu
                aboutClicked={handleAboutClicked}
                newGameClicked={handleNewGameClicked}
                settingsClicked={handleSettingsClicked}
            />
            <div className="container">
                <Board
                    flipped={flipped}
                    currentTurn={currentTurn}
                    squares={squares}
                    focusedSquare={focusedSquare}
                    pieces={pieces}
                    droppedSquare={lastDroppedSquare}
                    pawnPromotionDialogSquare={pawnPromotionDialogSquare}
                    onMovePiece={humanMovePiece}
                    onDropPiece={handleDropPiece}
                    onPieceClick={handlePieceClick}
                    onPiecePromotion={promotePiece}
                />

                <StatusBar
                    currentTurn={currentTurn}
                    inCheck={inCheck}
                    moveStatus={moveStatus}
                    gameOverStatus={gameOverStatus}
                />
            </div>
            {showNewGame &&
                <Modal>
                    <h1>New Game</h1>
                    <p>Start a new game?</p>
                    <div className="buttons">
                        <button className="btn-highlight" onClick={startNewGame}>Start New Game</button>
                        <button onClick={handleCancelNewGame}>Cancel</button>
                    </div>
                </Modal>
            }
            {showAbout &&
                <AboutModal onClick={() => setShowAbout(false) }/>
            }
            {showSettings &&
                <SettingsModal
                    flipped={flipped}
                    settings={settings}
                    onApply={handleApplySettings}
                    onDismiss={() => setShowSettings(false) }
                />
            }
            {gameStatus === wisdomChess.ThreefoldRepetitionReached &&
                !thirdRepetitionDrawAnswered &&
                hasHumanPlayer &&
                <DrawDialog
                    title={"Third Repetition Reached"}
                    onAccepted={() => handleThirdRepetitionDrawAnswer(true)}
                    onDeclined={() => handleThirdRepetitionDrawAnswer(false)}
                >
                    <p>The same position was reached three times. Either player can declare a draw now.</p>
                </DrawDialog>
            }
            {gameStatus === wisdomChess.FiftyMovesWithoutProgressReached &&
                !fiftyMovesDrawAnswered &&
                hasHumanPlayer &&
                <DrawDialog
                    title={"Fifty Moves Without Progress"}
                    onAccepted={() => handleFiftyMovesWithoutProgressDrawAnswer(true)}
                    onDeclined={() => handleFiftyMovesWithoutProgressDrawAnswer(false)}
                >
                    <p>There has been fifty moves without a pawn move or a piece taken. Either player can declare a draw now.</p>
                </DrawDialog>
            }
            {showModalOverlay && <div className="modal-overlay"></div>}
        </div>
    );
}

export default App
