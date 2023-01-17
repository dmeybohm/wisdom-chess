import React, { useState } from 'react'
import Square from "./Square";
import "./Board.css";
import {initialPieces, Piece} from "./Pieces";
import {initialSquares, Position} from "./Squares";
import "./Positions.css"

const Board = () => {
    const [squares, setSquares] = useState(initialSquares)
    const [focusedSquare, setFocusedSquare] = useState('')
    const [pieces, setPieces] = useState(initialPieces)

    const handleChangeFocus = (square: string) => {
        console.log('change focus to ' +square);
        setFocusedSquare(focusedSquare === square ? '' : square)
    }

    const movePiece = (src: string, dst: string) => {
        if (src === '') {
            return
        }
        // find the piece at the focused square:
        const index = pieces.findIndex(piece => piece.position === src)
        if (index < 0) {
            return;
        }
        const piecesCopy = pieces.map(piece => { return { ... piece }; })
        piecesCopy[index].position = dst
        setPieces(piecesCopy)
        setFocusedSquare('');
    }

    return (
        <section className="board">
            <>
                {squares.map((position: Position) => {
                    return (
                        <Square
                            key={position.index}
                            isOddRow={position.isOddRow}
                            position={position.position}
                            onClick={() => movePiece(focusedSquare, position.position)}
                        />
                    );
                })}
                {pieces.map((piece: Piece, index) => (
                    <div className={`piece ${piece.position} ${piece.position === focusedSquare ? 'focused' : ''}`}
                         key={index}
                         onClick={() => handleChangeFocus(piece.position)}>
                        <img alt="piece" src={piece.type} />
                    </div>
                ))}
            </>
        </section>
    )
}

export default Board