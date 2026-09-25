import React, { useState } from 'react'
import {WisdomChess, Game, PieceColor, pieceColorToString} from "./lib/WisdomChess";

export interface StatusBarProps {
    currentTurn: PieceColor
    inCheck: boolean
    moveStatus: string
    gameOverStatus: string
}

function RenderStrongly(props: { message: string }) {
    const parts = props.message.split(/<strong>(.*?)<\/strong>/)
    return (
        <>
        {parts.map((part, i) => i % 2 === 1 ? <strong key={i}>{part}</strong> : part)}
        </>
    )
}

function StatusBar(props: StatusBarProps) {
    const gameStatus = `<strong>${pieceColorToString(props.currentTurn)}</strong> to move`
    const moveStatus = props.moveStatus +
        (
            Boolean(props.moveStatus) && props.inCheck
                ? " - " : ""
        ) +
        (
            props.inCheck && !Boolean(props.gameOverStatus) ? "Check!" : ""
        )

    return (
        <div className="status-bar">
            <div>
                {props.gameOverStatus ?
                    (<RenderStrongly message={props.gameOverStatus} />) :
                    (<RenderStrongly message={gameStatus} />)
                }
            </div>
            <div>
                <RenderStrongly message={moveStatus} />
            </div>
        </div>
    )
}

export default StatusBar;
