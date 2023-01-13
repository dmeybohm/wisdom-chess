import React, { useState } from 'react'
import Square from "./Square";
import "./Board.css";
import WhitePawn from "./assets/Chess_plt45.svg";
import WhiteBishop from "./assets/Chess_blt45.svg";
import WhiteKnight from "./assets/Chess_nlt45.svg";
import WhiteQueen from "./assets/Chess_qlt45.svg";
import WhiteRook from "./assets/Chess_rlt45.svg";
import WhiteKing from "./assets/Chess_klt45.svg";
import BlackPawn from "./assets/Chess_pdt45.svg";
import BlackBishop from "./assets/Chess_bdt45.svg";
import BlackKnight from "./assets/Chess_ndt45.svg";
import BlackQueen from "./assets/Chess_qdt45.svg";
import BlackRook from "./assets/Chess_rdt45.svg";
import BlackKing from "./assets/Chess_kdt45.svg";

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
                return (
                    <Square
                        key={index}
                        isOddRow={Boolean(Math.floor(index / 8) % 2 > 0)}
                        square={square}
                    />
                );
            })}
            <div className="piece a7">
                <img src={BlackPawn}  />
            </div>
            <div className="piece b7">
                <img src={BlackPawn}  />
            </div>
            <div className="piece c7">
                <img src={BlackPawn}  />
            </div>
            <div className="piece d7">
                <img src={BlackPawn}  />
            </div>
            <div className="piece e7">
                <img src={BlackPawn}  />
            </div>
            <div className="piece f7">
                <img src={BlackPawn}  />
            </div>
            <div className="piece g7">
                <img src={BlackPawn}  />
            </div>
            <div className="piece h7">
                <img src={BlackPawn}  />
            </div>
            <div className="piece a8">
                <img src={BlackRook}  />
            </div>
            <div className="piece b8">
                <img src={BlackKnight}  />
            </div>
            <div className="piece c8">
                <img src={BlackBishop}  />
            </div>
            <div className="piece d8">
                <img src={BlackQueen}  />
            </div>
            <div className="piece e8">
                <img src={BlackKing}  />
            </div>
            <div className="piece f8">
                <img src={BlackBishop}  />
            </div>
            <div className="piece g8">
                <img src={BlackKnight}  />
            </div>
            <div className="piece h8">
                <img src={BlackRook}  />
            </div>
            <div className="piece a2">
                <img src={WhitePawn}  />
            </div>
            <div className="piece b2">
                <img src={WhitePawn}  />
            </div>
            <div className="piece c2">
                <img src={WhitePawn}  />
            </div>
            <div className="piece d2">
                <img src={WhitePawn}  />
            </div>
            <div className="piece e2">
                <img src={WhitePawn}  />
            </div>
            <div className="piece f2">
                <img src={WhitePawn}  />
            </div>
            <div className="piece g2">
                <img src={WhitePawn}  />
            </div>
            <div className="piece h2">
                <img src={WhitePawn}  />
            </div>
            <div className="piece a1">
                <img src={WhiteRook}  />
            </div>
            <div className="piece b1">
                <img src={WhiteKnight}  />
            </div>
            <div className="piece c1">
                <img src={WhiteBishop}  />
            </div>
            <div className="piece d1">
                <img src={WhiteQueen}  />
            </div>
            <div className="piece e1">
                <img src={WhiteKing}  />
            </div>
            <div className="piece f1">
                <img src={WhiteBishop}  />
            </div>
            <div className="piece g1">
                <img src={WhiteKnight}  />
            </div>
            <div className="piece h1">
                <img src={WhiteRook}  />
            </div>
        </section>
    )
}

export default Board