import React, { useEffect, useMemo, useReducer, useState } from 'react'
import './App.css'
import Board from "./Board";
import TopMenu from "./TopMenu";
import StatusBar from "./StatusBar";
import Modal from "./Modal";
import { initialSquares, Position } from "./Squares";
import {
    getPieces,
    getCurrentGame,
    PieceColor,
    WisdomChess,
    Move,
    WebMove,
    Game,
    HumanMove,
    ComputerMove
} from "./lib/WisdomChess";
import { Piece } from "./lib/Pieces";

const initialState = {
    squares: initialSquares,
    moves: [] as Move[],
    focusedSquare: '',
    pawnPromotionDialogSquare: '',
}

type MoveState = {
    moveStatus: string,
    gameOverStatus: string,
    currentTurn:  PieceColor,
    inCheck: boolean,
    pieces: Piece[],
}

type GameState = typeof initialState;

// This is the web assembly module. It's constant across changes:
let wisdomChess : any = undefined

type Action =
    | { type: 'move-piece', dst: string }
    | { type: 'computer-move-piece', move: WebMove }
    | { type: 'piece-click', dst: string, pieces: Piece[] }

function findPieceAtPosition(pieces: Piece[], position: string): Piece|undefined {
    return pieces.find(piece => piece.position === position)
}

function convertHumanMove(game: Game, humanMove: HumanMove): WebMove {
    // find the piece at the focused square:
    const srcCoord = wisdomChess.WebCoord.prototype.fromTextCoord(humanMove.src)
    const dstCoord = wisdomChess.WebCoord.prototype.fromTextCoord(humanMove.dst)
    return getCurrentGame().createMoveFromCoordinatesAndPromotedPiece(
        srcCoord,
        dstCoord,
        humanMove.promotedPieceType,
    )
}

function gameStateReducer(state: GameState, action: Action): GameState {
    const newState = { ... state }
    const currentGame = getCurrentGame()
    let move;

    switch (action.type) {
        case 'move-piece':
            newState.pawnPromotionDialogSquare = ''
            const src = newState.focusedSquare;
            if (src === '') {
                return newState
            }
            const srcCoord = wisdomChess.WebCoord.prototype.fromTextCoord(src)
            const dstCoord = wisdomChess.WebCoord.prototype.fromTextCoord(action.dst)

            if (!currentGame.createMoveFromCoordinatesAndPromotedPiece(srcCoord, dstCoord)) {
                newState.focusedSquare = ''
                return newState
            }
            if (currentGame.needsPawnPromotion(srcCoord, dstCoord)) {
                newState.pawnPromotionDialogSquare = action.dst
                return newState
            }
            newState.moves = [
                ... newState.moves,
                {
                    type: 'human',
                    src: src,
                    dst: action.dst,
                    promotedPieceType: 0
                }
            ]
            return newState

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
                return newState
            }

            // toggle focus if already focused:
            if (newState.focusedSquare === action.dst) {
                newState.focusedSquare = ''
                return newState
            }

            // Change focus if piece is the same color:
            const dstPiece = findPieceAtPosition(action.pieces, action.dst)
            const srcPiece = findPieceAtPosition(action.pieces, newState.focusedSquare)
            if (!dstPiece || !srcPiece || srcPiece.color === dstPiece.color) {
                newState.focusedSquare = action.dst
                return newState
            }

            // take the piece with the move-piece action, and then remove the focus:
            return {
                ... gameStateReducer(
                    newState,
                    {type: 'move-piece', dst: action.dst}
                ),
                focusedSquare: ''
            }

        case 'computer-move-piece':
            newState.moves = [
                ... newState.moves,
                {
                    type: 'computer',
                    move: action.move.asString()
                }
            ]
            return newState

        default:
            return newState
    }
}

//
// Check if the number of moves is not equal, and we need to apply / remove moves:
//
function applyMoveListChanges(gameState: GameState) {
    const currentGame = getCurrentGame()
    if (currentGame.moveNumber < gameState.moves.length) {
        const nextMove = gameState.moves[currentGame.moveNumber];
        const currentColor = currentGame.getCurrentTurn()
        const move = nextMove.type === 'human'
            ? convertHumanMove(currentGame, nextMove as HumanMove)
            : wisdomChess.WebMove.prototype.fromString((nextMove as ComputerMove).move, currentGame.getCurrentTurn())
        currentGame.makeMove(move)
    }
}

//
// Update the reactive state from any changes on the Game object.
//
function newMoveState(): MoveState {
    const currentGame = getCurrentGame()
    return {
        pieces: getPieces(currentGame),
        moveStatus: currentGame.moveStatus,
        gameOverStatus: currentGame.gameOverStatus,
        currentTurn: currentGame.getCurrentTurn(),
        inCheck: currentGame.inCheck,
    }
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

    const [gameState, dispatch] = useReducer(gameStateReducer, initialState)

    const moveState = useMemo(() => {
        applyMoveListChanges(gameState)
        return newMoveState()
    }, [gameState.moves])

    useEffect(() => {
        const listener = (event: CustomEvent) => {
            console.log('computerMoved', event.detail)
            const move = wisdomChess.WebMove.prototype.fromString(event.detail, getCurrentGame().getCurrentTurn());
            // handle castle / promotion etc
            dispatch({type: 'computer-move-piece', move: move as WebMove })
        }
        (window as any).computerMoved = listener;
        window.addEventListener('computerMoved', listener as EventListener);
        return () => {
            window.removeEventListener('computerMoved', listener as EventListener)
        }
    }, [])

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
                    pieces={moveState.pieces}
                    handleMovePiece={dst => dispatch({type: 'move-piece', dst})}
                    handlePieceClick={dst => dispatch({type: 'piece-click', dst, pieces: moveState.pieces})}
                    pawnPromotionDialogSquare={gameState.pawnPromotionDialogSquare}
                />

                <StatusBar
                    currentTurn={moveState.currentTurn}
                    inCheck={moveState.inCheck}
                    moveStatus={moveState.moveStatus}
                    gameOverStatus={moveState.gameOverStatus}
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
