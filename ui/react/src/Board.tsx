import React, { useEffect, useState } from 'react'
import Square from "./Square";
import "./Board.css";
import { Piece } from "./lib/Pieces";
import { Position } from "./lib/Squares";
import "./Positions.css"
import PawnPromotionDialog from "./PawnPromotionDialog";
import { PieceColor, PieceType } from "./lib/WisdomChess";

export interface BoardProps {
    squares: Array<Position>
    focusedSquare: string
    pieces: Array<Piece>
    pawnPromotionDialogSquare: string
    currentTurn: PieceColor

    onMovePiece: (dst: string) => void
    onPieceClick: (dst: string) => void
    onPiecePromotion: (piece: PieceType) => void
}

const Board = (props: BoardProps) => {
    return (
        <section className="board">
            {props.squares.map((position: Position) => {
                return (
                    <Square
                        key={position.index}
                        isOddRow={position.isOddRow}
                        position={position.position}
                        onClick={props.onMovePiece}
                    />
                );
            })}
            {props.pieces.map((piece: Piece) => (
                <div className={`piece ${piece.position} ${piece.position === props.focusedSquare ? 'focused' : ''}`}
                     key={piece.id}
                     onClick={() => props.onPieceClick(piece.position)}>
                    <img alt="piece" src={piece.icon} />
                </div>
            ))}
            {props.pawnPromotionDialogSquare &&
                <PawnPromotionDialog
                    color={props.currentTurn}
                    square={props.pawnPromotionDialogSquare}
                    direction={-1}
                    selectedPiece={props.onPiecePromotion}
                />}
        </section>
    )
}

export default Board