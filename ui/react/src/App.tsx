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
import { initialGameState, useGameState } from "./lib/useGameState";

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
    const [gameState, actions] = useGameState(initialGameState, gameRef.current)

    useEffect(() => {
        const listener = (event: CustomEvent) => {
            console.log('computerMoved', event.detail)
            const move = wisdomChess.WebMove.prototype.fromString(event.detail, gameRef.current.getCurrentTurn());
            // handle castle / promotion etc
            actions.computerMovePiece(move as WebMove)
        }
        (window as any).computerMoved = listener;
        window.addEventListener('computerMoved', listener as EventListener);
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
                    handleMovePiece={dst => actions.humanMovePiece(dst)}
                    handlePieceClick={dst => actions.pieceClick(dst)}
                    handlePiecePromotion={pieceType => actions.promotePiece(pieceType)}
                    pawnPromotionDialogSquare={gameState.pawnPromotionDialogSquare}
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
                <button onClick={() => setShowNewGame(false)}>OK</button>
            </Modal>
            <Modal show={showAbout}>
                <h1>About Modal</h1>
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