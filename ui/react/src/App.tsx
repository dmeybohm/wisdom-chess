import React, { useEffect, useRef, useState } from 'react'
import './App.css'
import Board from "./Board";
import TopMenu from "./TopMenu";
import StatusBar from "./StatusBar";
import Modal from "./Modal";
import { initialSquares, Position } from "./Squares";
import {
    getPieces,
    getCurrentGame,
    WisdomChess,
    WebMove,
    Game,
} from "./lib/WisdomChess";
import { Piece } from "./lib/Pieces";

const initialGameState = {
    pieces: [] as Piece[],
    squares: initialSquares,
    focusedSquare: '',
    pawnPromotionDialogSquare: '',
}

type GameState = typeof initialGameState;

// This is the web assembly module. It's constant across changes:
let wisdomChess : any = undefined

function findPieceAtPosition(pieces: Piece[], position: string): Piece|undefined {
    return pieces.find(piece => piece.position === position)
}

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
                    squares={gameState.squares}
                    focusedSquare={gameState.focusedSquare}
                    pieces={gameState.pieces}
                    handleMovePiece={dst => actions.movePiece(dst)}
                    handlePieceClick={dst => actions.pieceClick(dst)}
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

function useGameState(initialGameState: GameState, game: Game) {
    const [gameState, setGameState] = useState<GameState>({
        pieces: getPieces(game),
        squares: initialSquares,
        focusedSquare: '',
        pawnPromotionDialogSquare: '',
    })

    function movePiece(prevState: GameState, dst: string): GameState {
        let newState = { ... gameState }

        newState.pawnPromotionDialogSquare = ''
        const src = newState.focusedSquare;
        if (src === '') {
            return newState
        }
        const srcCoord = wisdomChess.WebCoord.prototype.fromTextCoord(src)
        const dstCoord = wisdomChess.WebCoord.prototype.fromTextCoord(dst)

        const move = game.createMoveFromCoordinatesAndPromotedPiece(
            srcCoord,
            dstCoord,
            wisdomChess.Queen
        );
        if (!move || !game.isLegalMove(move)) {
            newState.focusedSquare = ''
            return newState
        }
        if (game.needsPawnPromotion(srcCoord, dstCoord)) {
            newState.pawnPromotionDialogSquare = dst
            return newState
        }
        newState.focusedSquare = ''
        game.makeMove(move)
        newState.pieces = getPieces(game)
        return newState
    }

    //
    // Either:
    // - change the focus to the current piece, or
    // - take the piece if the color of the piece is different
    //
    function pieceClick(prevState: GameState, dst: string): GameState {
        let newState = { ... gameState }
        newState.pawnPromotionDialogSquare = ''

        // begin focus if no focus already:
        if (newState.focusedSquare === '') {
            newState.focusedSquare = dst
            return newState
        }

        // toggle focus if already focused:
        if (newState.focusedSquare === dst) {
            newState.focusedSquare = ''
            return newState
        }

        // Change focus if piece is the same color:
        const pieces = getPieces(game)
        const dstPiece = findPieceAtPosition(pieces, dst)
        const srcPiece = findPieceAtPosition(pieces, newState.focusedSquare)
        if (!dstPiece || !srcPiece || srcPiece.color === dstPiece.color) {
            newState.focusedSquare = dst
            return newState
        }

        // take the piece with the move-piece action, and then remove the focus:
        newState = movePiece(newState, dst)
        newState.pieces = getPieces(game)
        newState.focusedSquare = ''
        return newState
    }

    function computerMovePiece(prevState: GameState, move: WebMove): GameState {
        game.makeMove(move)
        return {
            ... prevState,
            pieces: getPieces(game)
        }
    }

    //
    // Wrap action functions to update the state with their return values.
    //
    const actions = {
        movePiece: (dst: string) => setGameState(movePiece(gameState, dst)),
        pieceClick: (dst: string) => setGameState(pieceClick(gameState, dst)),
        computerMovePiece: (move: WebMove) => setGameState(computerMovePiece(gameState, move))
    }

    return [
        gameState,
        actions
    ] as const
}

export default App
