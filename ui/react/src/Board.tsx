import React from 'react'
import { Square, PieceOverlay } from "./Square";
import "./Board.css";
import { Piece } from "./lib/Pieces";
import { Position } from "./lib/Squares";
import "./Positions.css"
import PawnPromotionDialog from "./PawnPromotionDialog";
import { PieceColor, PieceType } from "./lib/WisdomChess";
import { DndProvider } from "react-dnd";
import { HTML5Backend } from "react-dnd-html5-backend";

export interface BoardProps {
    squares: Array<Position>
    focusedSquare: string
    droppedSquare: string
    pieces: Array<Piece>
    pawnPromotionDialogSquare: string
    currentTurn: PieceColor
    flipped: boolean

    onMovePiece: (dst: string) => void
    onPieceClick: (dst: string) => void
    onDropPiece: (dst: string, src: string) => void
    onPiecePromotion: (piece: PieceType) => void
}

const Board = (props: BoardProps) => {
    return (
        <DndProvider backend={HTML5Backend}>
            <section className={`board ${props.flipped ? 'flipped' : ''}`}>
                {props.squares.map((position: Position) => {
                    return (
                        <Square
                            key={position.index}
                            isOddRow={position.isOddRow}
                            position={position.position}
                            onClick={props.onMovePiece}
                            onDropPiece={props.onDropPiece}
                        />
                    );
                })}
                {props.pieces.map((piece: Piece) => (
                    <PieceOverlay
                        key={piece.id}
                        piece={piece}
                        focusedSquare={props.focusedSquare}
                        droppedSquare={props.droppedSquare}
                        onPieceClick={props.onPieceClick}
                        onDropPiece={props.onDropPiece}
                    />
                ))}
                {props.pawnPromotionDialogSquare &&
                    <PawnPromotionDialog
                        color={props.currentTurn}
                        square={props.pawnPromotionDialogSquare}
                        direction={-1}
                        selectedPiece={props.onPiecePromotion}
                    />}
            </section>
        </DndProvider>
    )
}

export default Board