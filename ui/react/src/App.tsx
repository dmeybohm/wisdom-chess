import React, {useEffect, useState} from 'react'
import './App.css'
import Board from "./Board";
import TopMenu from "./TopMenu";
import StatusBar from "./StatusBar";
import Modal from "./Modal";
import {initialSquares, Position} from "./Squares";
import {getPieces, makeGame, PieceColor, WisdomChess} from "./lib/WisdomChess";

function App() {

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

    const wisdomChess : WisdomChess = WisdomChess()

    const [squares, setSquares] = useState(initialSquares)
    const [focusedSquare, setFocusedSquare] = useState('')
    const [game, setGame] = useState(() => makeGame())
    const [pieces, setPieces] = useState(() => getPieces(game))
    const [pawnPromotionDialogSquare, setPawnPromotionDialogSquare] = useState('');
    const [moveStatus, setMoveStatus] = useState('')
    const [gameOverStatus, setGameOverStatus] = useState('');
    const [currentTurn, setCurrentTurn] = useState<PieceColor>(wisdomChess.NoColor)
    const [inCheck, setInCheck] = useState(false)

    useEffect(() => {
        setPieces(getPieces(game))
    }, [game])

    // Update the statuses when the pieces change:
    useEffect(() => {
        setGameOverStatus(game.gameOverStatus)
        setMoveStatus(game.moveStatus)
        setCurrentTurn(game.getCurrentTurn())
        setInCheck(game.inCheck)
    }, [pieces])

    const findIndexOfPieceAtPosition = (position: string) => {
        return pieces.findIndex(piece => piece.position === position)
    }

    //
    // Either:
    // - change the focus to the current piece, or
    // - take the piece if the color of the piece is different
    //
    const handlePieceClick = (dstSquare: string) => {
        setPawnPromotionDialogSquare('')
        if (focusedSquare === '') {
            setFocusedSquare(dstSquare)
            return
        }
        if (focusedSquare === dstSquare) {
            setFocusedSquare('')
            return
        }
        const dstIndex = findIndexOfPieceAtPosition(dstSquare)
        const srcIndex = findIndexOfPieceAtPosition(focusedSquare)
        if (dstIndex === -1 || srcIndex === -1 || pieces[dstIndex].color === pieces[srcIndex].color) {
            setFocusedSquare(dstSquare)
            return
        }
        handleMovePiece(dstSquare)
        setFocusedSquare('')
    }

    const handleMovePiece = (dst: string) => {
        setPawnPromotionDialogSquare('')
        const src = focusedSquare;
        if (src === '') {
            return
        }
        // find the piece at the focused square:
        const srcCoord = wisdomChess.WebCoord.prototype.fromTextCoord(src)
        const dstCoord = wisdomChess.WebCoord.prototype.fromTextCoord(dst)
        if (game.needsPawnPromotion(srcCoord, dstCoord)) {
            setPawnPromotionDialogSquare(dst)
            return
        }
        const movedSuccess = game.makeMove(srcCoord, dstCoord)
        if (movedSuccess) {
            setPieces(oldPieces => getPieces(game))
        }
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
                    squares={squares}
                    focusedSquare={focusedSquare}
                    pieces={pieces}
                    handleMovePiece={handleMovePiece}
                    handlePieceClick={handlePieceClick}
                    pawnPromotionDialogSquare={pawnPromotionDialogSquare}
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
