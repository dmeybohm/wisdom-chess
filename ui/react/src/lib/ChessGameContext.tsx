import {createContext, useEffect, useState} from "react";
import {initialSquares, Position} from "../Squares";
import {Game, getPieces, makeGame, WisdomChess} from "./WisdomChess";
import {Piece} from "../Pieces";

export interface ChessGameContext {
    squares: Array<Position>
    focusedSquare: string
    game: Game
    pieces: Array<Piece>
    pawnPromotionDialogSquare: string
    actions: {
        handlePieceClick: (dst: string) => void
        handleMovePiece: (dst: string) => void
    }
}

export const ChessGameContext = createContext<ChessGameContext|undefined>(undefined);

interface ChessGameProps {
    children: JSX.Element
}

export function ChessGameProvider(props: ChessGameProps) {
    const [squares, setSquares] = useState(initialSquares)
    const [focusedSquare, setFocusedSquare] = useState('')
    const [game, setGame] = useState(() => makeGame())
    const [pieces, setPieces] = useState(() => getPieces(game))
    const [pawnPromotionDialogSquare, setPawnPromotionDialogSquare] = useState('');

    const wisdomChess : WisdomChess = WisdomChess()

    useEffect(() => {
       setGame(makeGame())
    }, [])

    useEffect(() => {
        setPieces(getPieces(game))
    }, [game])

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
        <ChessGameContext.Provider value={{
            squares,
            focusedSquare,
            game,
            pieces,
            pawnPromotionDialogSquare,
            actions: {
                handleMovePiece,
                handlePieceClick,
            }
        }}>
            {props.children}
        </ChessGameContext.Provider>
    );
}