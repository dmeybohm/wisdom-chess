import React, { useState } from 'react'
import {WisdomChess, Game, PieceColor, pieceColorToString} from "./lib/WisdomChess";

export interface StatusBarProps {
    currentTurn: PieceColor
    inCheck: boolean
    moveStatus: string
    gameOverStatus: string
}

function StatusBar(props: StatusBarProps) {
    return (
        <div className="status-bar">
            <div>
                {props.gameOverStatus ? props.gameOverStatus : (
                    <>
                        <strong>{pieceColorToString(props.currentTurn)}</strong> to move
                    </>
                )}
            </div>
            <div>
                {
                    props.moveStatus +
                    (
                        Boolean(props.moveStatus) && props.inCheck
                        ? " - " : ""
                    ) +
                    (
                        props.inCheck && !Boolean(props.gameOverStatus) ? "Check!" : ""
                    )
                }
            </div>
        </div>
    )
}

export default StatusBar;
