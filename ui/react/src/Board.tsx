import React, { useState } from 'react'
import Square from "./Square";
import "./Board.css";
import WhitePawn from "./assets/Chess_plt45.svg";
import WhiteBishop from "./assets/Chess_blt45.svg";
import WhiteKnight from "./assets/Chess_nlt45.svg";
import WhiteQueen from "./assets/Chess_qlt45.svg";
import WhiteRook from "./assets/Chess_rlt45.svg";
import WhiteKing from "./assets/Chess_klt45.svg";

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
            <div className="piece a2">
                <img src={WhitePawn} width={96} height={96} />
            </div>
            <div className="piece b2">
                <img src={WhitePawn} width={96} height={96} />
            </div>
            <div className="piece c2">
                <img src={WhitePawn} width={96} height={96} />
            </div>
            <div className="piece d2">
                <img src={WhitePawn} width={96} height={96} />
            </div>
            <div className="piece e2">
                <img src={WhitePawn} width={96} height={96} />
            </div>
            <div className="piece f2">
                <img src={WhitePawn} width={96} height={96} />
            </div>
            <div className="piece g2">
                <img src={WhitePawn} width={96} height={96} />
            </div>
            <div className="piece h2">
                <img src={WhitePawn} width={96} height={96} />
            </div>
            <div className="piece a1">
                <img src={WhiteRook} width={96} height={96} />
            </div>
            <div className="piece b1">
                <img src={WhiteKnight} width={96} height={96} />
            </div>
            <div className="piece c1">
                <img src={WhiteBishop} width={96} height={96} />
            </div>
            <div className="piece d1">
                <img src={WhiteQueen} width={96} height={96} />
            </div>
            <div className="piece e1">
                <img src={WhiteKing} width={96} height={96} />
            </div>
            <div className="piece f1">
                <img src={WhiteBishop} width={96} height={96} />
            </div>
            <div className="piece g1">
                <img src={WhiteKnight} width={96} height={96} />
            </div>
            <div className="piece h1">
                <img src={WhiteRook} width={96} height={96} />
            </div>
        </section>
    )
}

export default Board