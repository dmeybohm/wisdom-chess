import React from 'react'
import { PieceColor, pieceColorToString } from "./lib/WisdomChess";

export interface StatusBarProps {
    currentTurn: PieceColor
    inCheck: boolean
    moveStatus: string
    gameOverStatus: string
}

// The engine marks bold text in its status messages with <strong> tags.
function RenderStrongly(props: { message: string }) {
    const parts = props.message.split(/<strong>(.*?)<\/strong>/)
    return (
        <>
        {parts.map((part, i) => i % 2 === 1 ? <strong key={i}>{part}</strong> : part)}
        </>
    )
}

function StatusBar(props: StatusBarProps) {
    const check = props.inCheck && !props.gameOverStatus ? 'Check!' : ''
    const moveStatus = [props.moveStatus, check].filter(Boolean).join(' - ')

    return (
        <div className="status-bar">
            <div>
                {props.gameOverStatus
                    ? <RenderStrongly message={props.gameOverStatus} />
                    : <><strong>{pieceColorToString(props.currentTurn)}</strong> to move</>
                }
            </div>
            <div>{moveStatus}</div>
        </div>
    )
}

export default StatusBar;
