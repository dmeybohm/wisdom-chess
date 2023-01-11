import React, { useState } from 'react'
import Square from "./Square";
import "./Board.css";

let initialSquares : Array<number> = [];
for (let i = 0; i < 64; i++) {
    initialSquares.push(0);
}

let row_or_col: Array<number> = [];
for (let i = 0; i < 8; i++) {
    row_or_col.push(i);
}

const Board = () => {
    const [squares, setSquares] = useState(initialSquares)

    return (
        <section className="board">
            {squares.map((square, index) => {
                console.log(typeof squares);
                console.log(`square: ${index} - ${Math.floor(index / 8) % 2 > 0 ? 'odd' : 'even'}`)
                return (
                    <Square
                        key={index}
                        isOddRow={Boolean(Math.floor(index / 8) % 2 > 0)}
                        square={square}
                    />
                );
            })}
        </section>
    )
}

export default Board