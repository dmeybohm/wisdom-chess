import React, { useEffect, useRef, useState } from 'react'
import './App.css'
import Board from "./Board";
import TopMenu from "./TopMenu";
import StatusBar from "./StatusBar";
import Modal from "./Modal";
import {
    getCurrentGame,
    WisdomChess,
    WebMove,
} from "./lib/WisdomChess";
import { initialGameState, useGame } from "./lib/useGame";

// This is the web assembly module. It's constant across changes:
export let wisdomChess : any = undefined

function App() {

    wisdomChess = WisdomChess()

    const [showAbout, setShowAbout] = useState(false);
    const [showNewGame, setShowNewGame] = useState(false);
    const [showSettings, setShowSettings] = useState(false);

    const handleAboutClicked = (): void => {
        setShowAbout(true);
    };

    const handleNewGameClicked = (): void => {
        setShowNewGame(true);
    }

    const handleSettingsClicked = (): void => {
        setShowSettings(true);
    }

    const gameRef = useRef(getCurrentGame())
    const gameState = useGame((state) => state)
    const actions = useGame((state) => state.actions)

    useEffect(() => {
        const listener = (event: CustomEvent) => {
            console.log('computerMoved', event.detail)
            const moveStr = event.detail.move;
            const gameId = event.detail.gameId;
            const move = wisdomChess.WebMove.prototype.fromString(moveStr, gameRef.current.getCurrentTurn());
            // handle castle / promotion etc
            actions.computerMovePiece(move as WebMove)
        }
        (window as any).computerMoved = listener;
        window.addEventListener('computerMoved', listener as EventListener);
        actions.init()
        return () => {
            window.removeEventListener('computerMoved', listener as EventListener)
        }
    }, [])

    const gameOverStatus = gameRef.current.gameOverStatus
    const currentTurn = gameRef.current.getCurrentTurn()
    const inCheck = gameRef.current.inCheck
    const moveStatus = gameRef.current.moveStatus

    return (
        <div className="App">
            <TopMenu
                aboutClicked={handleAboutClicked}
                newGameClicked={handleNewGameClicked}
                settingsClicked={handleSettingsClicked}
            />
            <div className="container">
                <Board
                    currentTurn={currentTurn}
                    squares={gameState.squares}
                    focusedSquare={gameState.focusedSquare}
                    pieces={gameState.pieces}
                    pawnPromotionDialogSquare={gameState.pawnPromotionDialogSquare}
                    onMovePiece={dst => actions.humanMovePiece(dst)}
                    onPieceClick={dst => actions.pieceClick(dst)}
                    onPiecePromotion={pieceType => actions.promotePiece(pieceType)}
                />

                <StatusBar
                    currentTurn={currentTurn}
                    inCheck={inCheck}
                    moveStatus={moveStatus}
                    gameOverStatus={gameOverStatus}
                />
            </div>
            <Modal show={showNewGame}>
                <h1>New Game</h1>
                <button onClick={() => actions.startNewGame()}>Cancel</button>
                <button onClick={() => setShowNewGame(false)}>Cancel</button>
            </Modal>
            <Modal show={showAbout}>
                <h1>About Modal</h1>
                <p>
                    <a
                        target="_blank"
                        href="https://github.com/dmeybohm/wisdom-chess">
                        View the source
                    </a>
                </p>
                <button onClick={() => setShowAbout(false)}>OK</button>
            </Modal>
            <Modal show={showSettings}>
                <h1>Settings Modal</h1>
                <button onClick={() => setShowSettings(false)}>OK</button>
            </Modal>
        </div>
    );
}

export default App