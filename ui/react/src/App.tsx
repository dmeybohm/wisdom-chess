import React, { useEffect, useRef, useState } from 'react'
import './App.css'
import Board from "./Board";
import TopMenu from "./TopMenu";
import StatusBar from "./StatusBar";
import Modal from "./Modal";
import {
    getCurrentGame,
    WisdomChess,
    GameSettings,
} from "./lib/WisdomChess";
import { initialGameState, useGame } from "./lib/useGame";
import { SettingsModal } from "./SettingsModal";

// This is the web assembly module. It's constant across changes:
export let wisdomChess : any = undefined

function App() {

    wisdomChess = WisdomChess()

    const [flipped, setFlipped] = useState(false);
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

    const game = getCurrentGame()
    const gameState = useGame((state) => state)
    const actions = useGame((state) => state.actions)

    const gameOverStatus = game.gameOverStatus
    const currentTurn = game.getCurrentTurn()
    const inCheck = game.inCheck
    const moveStatus = game.moveStatus

    const handleNewGame = () => {
        actions.startNewGame()
        setShowNewGame(false)
    }

    function handleApplySettings(gameSettings: GameSettings, flipped: boolean) {
        setShowSettings(false);
        setFlipped(flipped)
        actions.applySettings(gameSettings)
    }

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
            {showNewGame &&
                <Modal>
                    <h1>New Game</h1>
                    <p>Start a new game?</p>
                    <div className="buttons">
                        <button className="btn-highlight" onClick={handleNewGame}>Start New Game</button>
                        <button onClick={() => setShowNewGame(false)}>Cancel</button>
                    </div>
                </Modal>
            }
            {showAbout &&
                <Modal>
                    <h1>About Wisdom Chess</h1>
                    <p>
                        <a
                            target="_blank"
                            href="https://github.com/dmeybohm/wisdom-chess">
                            View the source
                        </a>
                    </p>
                    <button onClick={() => setShowAbout(false)}>OK</button>
                </Modal>
            }
            {showSettings &&
                <SettingsModal
                    flipped={flipped}
                    onApply={handleApplySettings}
                    onDismiss={() => setShowSettings(false) }
                />
            }
        </div>
    );
}

export default App