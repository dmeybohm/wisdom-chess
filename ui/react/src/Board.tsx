import React, {useEffect, useState} from 'react'
import Square from "./Square";
import "./Board.css";
import {Piece} from "./Pieces";
import {initialSquares, Position} from "./Squares";
import "./Positions.css"
import {getPieces, makeGame, WisdomChess} from "./lib/WisdomChess";
import PawnPromotionDialog from "./PawnPromotionDialog";

const Board = () => {
    const [squares, setSquares] = useState(initialSquares)
    const [focusedSquare, setFocusedSquare] = useState('')
    const [game, setGame] = useState(() => makeGame())
    const [pieces, setPieces] = useState(() => getPieces(game))
    const [pawnPromotionDialogSquare, setPawnPromotionDialogSquare] = useState('');

    useEffect(() => {
        setPieces(getPieces(game))
    }, [game])

    const wisdomChess : WisdomChess = WisdomChess()

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
        <section className="board">
            {squares.map((position: Position) => {
                return (
                    <Square
                        key={position.index}
                        isOddRow={position.isOddRow}
                        position={position.position}
                        onClick={handleMovePiece}
                    />
                );
            })}
            {pieces.map((piece: Piece) => (
                <div className={`piece ${piece.position} ${piece.position === focusedSquare ? 'focused' : ''}`}
                     key={piece.id}
                     onClick={() => handlePieceClick(piece.position)}>
                    <img alt="piece" src={piece.icon} />
                </div>
            ))}
            {pawnPromotionDialogSquare && <PawnPromotionDialog square={pawnPromotionDialogSquare} direction={-1} />}
        </section>
    )
}

export default Board