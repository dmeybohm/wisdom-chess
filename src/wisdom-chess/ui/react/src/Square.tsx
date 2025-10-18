import React from "react";
import "./Board.css";
import { Piece } from "./lib/Pieces";
import { useDrag, useDrop } from 'react-dnd';
import {
    fromColorToNumber,
    getCurrentGame,
    WisdomChess,
    PieceColor
} from "./lib/WisdomChess";

interface SquareProps {
    position: string
    isOddRow: boolean
    onClick(position: string): void;
    onDropPiece(src: string, dst: string): void;
}

type DroppedWithPosition = {
    src: string
}

export function Square(props: SquareProps) {
    const [{isOver}, drop] = useDrop({
        accept: 'piece',
        drop: (dropped) => {
            props.onDropPiece((dropped as DroppedWithPosition).src, props.position)
        },
        collect: (monitor) => ({
            isOver: monitor.isOver(),
            canDrop: monitor.canDrop(),
        })
    })
    return (
        <div
            ref={drop}
            className={`square ${props.isOddRow ? "odd" : ""} `}
            onClick={() => {
                props.onClick(props.position)
            }}
        >
        </div>
    );
}

interface PieceOverlayProps {
    piece: Piece
    focusedSquare: string
    droppedSquare: string
    currentTurn: PieceColor
    onPieceClick(position: string): void
    onDropPiece(src: string, dst: string): void;
}

export function PieceOverlay(props: PieceOverlayProps) {
    const wisdomChess = WisdomChess()

    const [{isDragging}, drag, preview] = useDrag({
        type: 'piece',
        item: { src: props.piece.position },
        canDrag: monitor => {
            const game = getCurrentGame()
            const pieceColor = fromColorToNumber(props.piece.color)
            return pieceColor === props.currentTurn &&
                game.getPlayerOfColor(pieceColor) === wisdomChess.Human
        },
        collect: monitor => ({
            isDragging: monitor.isDragging(),
        }),
    }, [props.piece.position, props.currentTurn])

    const [{isOver}, drop] = useDrop({
        accept: 'piece',
        drop: (dropped) => {
            props.onDropPiece((dropped as DroppedWithPosition).src, props.piece.position)
        },
        collect: (monitor) => ({
            isOver: monitor.isOver(),
        })
    }, [props.piece.position])

    const focused = props.piece.position === props.focusedSquare ? 'focused' : ''
    const draggingClass = props.droppedSquare === props.piece.position ? "dragging" : ''
    return (
        <div
            ref={drop}
            className={`piece ${props.piece.position} ${focused} ${draggingClass}`}
            onClick={() => props.onPieceClick(props.piece.position)}
        >
            <div
                ref={drag}
                style={{
                    transform: 'translate(0, 0)', // workaround background showing up
                    opacity: isDragging ? 0.5 : 1,
                }}
            >
                {!isDragging &&
                    <img
                        ref={drag}
                        draggable={false}
                        alt="piece"
                        src={props.piece.icon}
                    />
                }
            </div>
        {isDragging &&
            <img
                ref={preview}
                alt="piece"
                draggable={false}
                src={props.piece.icon}
                style={{
                    display: 'none',
                }}
            />
        }
        </div>
    )
}
