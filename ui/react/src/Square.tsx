import React from "react";
import "./Board.css";
import { Piece } from "./lib/Pieces";
import { useDrag, useDrop } from 'react-dnd';

interface SquareProps {
    position: string
    isOddRow: boolean
    onClick: (position: string) => void;
}

export function Square(props: SquareProps) {
    const [{isOver}, drop] = useDrop({
        accept: 'piece',
        drop: () => {
            console.log('drop')
        },
        collect: (monitor) => ({
            isOver: monitor.isOver()
        })
    })
    return (
        <div
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
    onPieceClick(position: string): void
}

export function PieceOverlay(props: PieceOverlayProps) {
    const [{isDragging}, drag] = useDrag({
        type: 'piece',
        collect: monitor => ({
            isDragging: monitor.isDragging()
        })
    })
    const focused = props.piece.position === props.focusedSquare ? 'focused' : ''
    return (
        <div
            className={`piece ${props.piece.position} ${focused}`}
            onClick={() => props.onPieceClick(props.piece.position)}
            style={{
                opacity: isDragging ? 0.5 : 1,
                cursor: isDragging ? 'move' : ''
            }}
        >
            <img
                alt="piece"
                src={props.piece.icon}
            />
        </div>
    )
}
