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
    handleMovePiece: (dst: string) => void
    handlePieceClick: (dst: string) => void
    handlePiecePromotion: (piece: PieceType) => void
    pawnPromotionDialogSquare: string
    currentTurn: PieceColor
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
                        onClick={props.handleMovePiece}
                    />
                );
            })}
            {props.pieces.map((piece: Piece) => (
                <div className={`piece ${piece.position} ${piece.position === props.focusedSquare ? 'focused' : ''}`}
                     key={piece.id}
                     onClick={() => props.handlePieceClick(piece.position)}>
                    <img alt="piece" src={piece.icon} />
                </div>
            ))}
            {props.pawnPromotionDialogSquare &&
                <PawnPromotionDialog
                    color={props.currentTurn}
                    square={props.pawnPromotionDialogSquare}
                    direction={-1}
                    selectedPiece={props.handlePiecePromotion}
                />}
        </section>
    )
}

export default Board