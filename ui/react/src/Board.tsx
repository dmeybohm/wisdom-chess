import React, { useState } from 'react'
import Square from "./Square";
import "./Board.css";
import {initialPieces, Piece} from "./Pieces";

let initialSquares : Array<number> = [];
for (let i = 0; i < 64; i++) {
    initialSquares.push(i);
}

let row_or_col: Array<number> = [];
for (let i = 0; i < 8; i++) {
    row_or_col.push(i);
}

const Board = () => {
    const [squares, setSquares] = useState(initialSquares)
    const [focusedSquare, setFocusedSquare] = useState('')
    const [pieces, setPieces] = useState(initialPieces)

    const handleChangeFocus = (square: string) => {
        console.log('change focus to ' +square);
        setFocusedSquare(square)
    }

    return (
        <section className="board">
            <>
                {squares.map((square, index) => {
                    return (
                        <Square
                            key={index}
                            isOddRow={Boolean(Math.floor(index / 8) % 2 > 0)}
                            square={square}
                        />
                    );
                })}
                {pieces.map((piece: Piece) => (
                    <div className={`piece ${piece.position} ${piece.position === focusedSquare ? 'focused' : ''}`}
                         onClick={() => handleChangeFocus(piece.position)}>
                        <img alt="piece" src={piece.type} />
                    </div>
                ))}
            </>
        </section>
    )
}

export default Board