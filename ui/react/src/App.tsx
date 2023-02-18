import React, { useEffect, useReducer, useState } from 'react'
import './App.css'
import Board from "./Board";
import TopMenu from "./TopMenu";
import StatusBar from "./StatusBar";
import Modal from "./Modal";
import { initialSquares, Position } from "./Squares";
import { getPieces, makeGame, PieceColor, WisdomChess } from "./lib/WisdomChess";
import { Piece } from "./Pieces";

interface GameState {
    squares:  Position[]
    focusedSquare: string
    pieces: Piece[]
    pawnPromotionDialogSquare: string
    moveStatus: string
    gameOverStatus: string
    currentTurn: PieceColor
    inCheck: boolean
}

// This is the web assembly module. It's constant across changes:
let wisdomChess : any = undefined

type Action =
    | { type: 'move-piece', dst: string }
    | { type: 'piece-click', dst: string }

function findPieceAtPosition(pieces: Piece[], position: string): Piece|undefined {
    return pieces.find(piece => piece.position === position)
}

function gameStateReducer(state: GameState, action: Action): GameState {
    const newState = { ... state }
    const currentGame = makeGame()

    switch (action.type) {
        case 'move-piece':
            newState.pawnPromotionDialogSquare = ''
            const src = newState.focusedSquare;
            if (src === '') {
                return updateGameStateFromGame(newState)
            }
            // find the piece at the focused square:
            const srcCoord = wisdomChess.WebCoord.prototype.fromTextCoord(src)
            const dstCoord = wisdomChess.WebCoord.prototype.fromTextCoord(action.dst)
            if (currentGame.needsPawnPromotion(srcCoord, dstCoord)) {
                newState.pawnPromotionDialogSquare = action.dst
                return updateGameStateFromGame(newState)
            }
            const movedSuccess = currentGame.makeMove(srcCoord, dstCoord)
            // todo set error if move state
            return updateGameStateFromGame(newState)

        //
        // Either:
        // - change the focus to the current piece, or
        // - take the piece if the color of the piece is different
        //
        case 'piece-click':
            newState.pawnPromotionDialogSquare = ''

            // begin focus if no focus already:
            if (newState.focusedSquare === '') {
                newState.focusedSquare = action.dst
                return updateGameStateFromGame(newState)
            }

            // toggle focus if already focused:
            if (newState.focusedSquare === action.dst) {
                newState.focusedSquare = ''
                return updateGameStateFromGame(newState)
            }

            // Change focus if piece is the same color:
            const dstPiece = findPieceAtPosition(newState.pieces, action.dst)
            const srcPiece = findPieceAtPosition(newState.pieces, newState.focusedSquare)
            if (!dstPiece || !srcPiece || srcPiece.color === dstPiece.color) {
                newState.focusedSquare = action.dst
                return updateGameStateFromGame(newState)
            }

            // take the piece:
            const updated = gameStateReducer(
                updateGameStateFromGame(newState),
                { type:'move-piece', dst: action.dst}
            )
            newState.focusedSquare = ''
            return updated

        default:
            return newState
    }
}

//
// Update the reactive state from any changes on the Game object.
//
function updateGameStateFromGame(gameState: GameState) {
    const currentGame = makeGame()
    return {
        ... gameState,
        pieces: getPieces(currentGame),
        moveStatus: currentGame.moveStatus,
        gameOverStatus: currentGame.gameOverStatus,
        currentTurn: currentGame.getCurrentTurn(),
        inCheck: currentGame.inCheck
    }
}

const initialState = {
    squares: initialSquares,
    focusedSquare: '',
    pieces: [],
    pawnPromotionDialogSquare: '',
    moveStatus: '',
    gameOverStatus: '',
    currentTurn: 0,
    inCheck: false,
}

function App() {

    useEffect(() => {
        wisdomChess = WisdomChess()
    }, [])

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

    const [gameState, dispatch] = useReducer(gameStateReducer, initialState, () => {
        return updateGameStateFromGame(initialState)
    })

    return (
        <div className="App">
            <TopMenu
                aboutClicked={handleAboutClicked}
                newGameClicked={handleNewGameClicked}
                settingsClicked={handleSettingsClicked}
            />
            <div className="container">
                <Board
                    squares={gameState.squares}
                    focusedSquare={gameState.focusedSquare}
                    pieces={gameState.pieces}
                    handleMovePiece={dst => dispatch({type: 'move-piece', dst})}
                    handlePieceClick={dst => dispatch({type: 'piece-click', dst})}
                    pawnPromotionDialogSquare={gameState.pawnPromotionDialogSquare}
                />

                <StatusBar
                    currentTurn={gameState.currentTurn}
                    inCheck={gameState.inCheck}
                    moveStatus={gameState.moveStatus}
                    gameOverStatus={gameState.gameOverStatus}
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
