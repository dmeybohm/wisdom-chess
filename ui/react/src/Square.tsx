import React from "react";
import "./Board.css";
import { Piece } from "./lib/Pieces";
import { useDrag, useDrop } from 'react-dnd';

interface SquareProps {
    position: string
    isOddRow: boolean
    onClick: (position: string) => void;
    onDropPiece: (src: string, dst: string) => void;
}

type DroppedWithPosition = {
    src: string
}

export function Square(props: SquareProps) {
    const [{isOver}, drop] = useDrop({
        accept: 'piece',
        drop: (dropped) => {
            console.log('drop')
            console.log(dropped)
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
    onPieceClick(position: string): void
}

export function PieceOverlay(props: PieceOverlayProps) {
    const [{isDragging}, drag, preview] = useDrag({
        type: 'piece',
        item: { src: props.piece.position },
        collect: monitor => ({
            isDragging: monitor.isDragging(),
        }),
    }, [props.piece.position])

    const focused = props.piece.position === props.focusedSquare ? 'focused' : ''
    const draggingClass = props.droppedSquare === props.piece.position ? "dragging" : ''
    return (
        <>
        <div
            ref={drag}
            className={`piece ${props.piece.position} ${focused} ${draggingClass}`}
            onClick={() => props.onPieceClick(props.piece.position)}
            style={{
                transform: 'translate(0, 0)', // workaround background showing up
                opacity: isDragging ? 0.5 : 1,
            }}
        >
            <img
                alt="piece"
                src={props.piece.icon}
            />
        </div>
        <img
            ref={preview}
            alt="piece"
            src={props.piece.icon}
            style={{
                cursor: 'move',
                display: 'none',
                background: 'transparent'
            }}
        />
        </>
    )
}
