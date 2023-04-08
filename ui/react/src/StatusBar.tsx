import React, { useState } from 'react'
import {WisdomChess, Game, PieceColor, pieceColorToString} from "./lib/WisdomChess";

export interface StatusBarProps {
    currentTurn: PieceColor
    inCheck: boolean
    moveStatus: string
    gameOverStatus: string
}

function splitMessageByStrong(message: string): string[] {
    const regex = /^<strong>(.*)<\/strong>(.*)$/
    const matches = message.match(regex)
    if (matches && matches.length > 0) {
        return [
            matches[1],
            matches[2]
        ]
    } else {
        return ['', message]
    }
}

function RenderStrongly(props: { message: string }) {
    const [boldPrefix, rest] = splitMessageByStrong(props.message)
    return (
        <>
        {boldPrefix && <strong>{boldPrefix}</strong>}
        {rest}
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
