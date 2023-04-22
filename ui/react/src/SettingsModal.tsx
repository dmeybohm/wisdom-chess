import Modal from "./Modal";
import React, { useMemo, useRef, useState } from "react";
import { useGame } from "./lib/useGame";
import { GameSettings, WisdomChess } from "./lib/WisdomChess";
import Slider from 'rc-slider';
import 'rc-slider/assets/index.css';
import "./Settings.css"

type SettingsModalProps = {
    flipped: boolean
    onApply: (newSettings: GameSettings, flipped: boolean) => void
    onDismiss: () => void
}

export function SettingsModal(props: SettingsModalProps) {
    const settings = useGame((state) => state.settings)
    const wisdomChess = WisdomChess()
    const humanWhite = useRef<HTMLInputElement|null>(null)
    const humanBlack = useRef<HTMLInputElement|null>(null)
    const flippedRef = useRef<HTMLInputElement|null>(null)

    const [thinkingTime, setThinkingTime] = useState(settings.thinkingTime);
    const [searchDepth, setSearchDepth] = useState(settings.searchDepth);

    const handleApply = (e: React.SyntheticEvent) => {
        e.preventDefault()
        const whitePlayer = humanWhite.current?.checked ?
            wisdomChess.Human :
            wisdomChess.ChessEngine
        const blackPlayer = humanBlack.current?.checked ?
            wisdomChess.Human :
            wisdomChess.ChessEngine

        const newSettings = new wisdomChess.GameSettings(
            whitePlayer,
            blackPlayer,
            thinkingTime,
            searchDepth
        )
        props.onApply(newSettings, Boolean(flippedRef?.current?.checked))
    }

    return (
        <Modal>
            <h1>Settings</h1>
            <form className="settings">
                <div>White Player</div>
                <div className="player-options">
                    <label>
                        <input
                            name="whitePlayer"
                            type="radio"
                            ref={humanWhite}
                            value={wisdomChess.Human}
                            defaultChecked={settings.whitePlayer === wisdomChess.Human}
                        />
                        Human
                    </label>
                    <label>
                        <input
                            name="whitePlayer"
                            type="radio"
                            value={wisdomChess.ChessEngine}
                            defaultChecked={settings.whitePlayer === wisdomChess.ChessEngine}
                        />
                        Computer
                    </label>
                </div>

                <div>Black Player</div>
                <div className="player-options">
                    <label>
                        <input
                            name="blackPlayer"
                            type="radio"
                            ref={humanBlack}
                            value={wisdomChess.Human}
                            defaultChecked={settings.blackPlayer === wisdomChess.Human}
                        />
                        Human
                    </label>
                    <label>
                        <input
                            id="computerBlack"
                            name="blackPlayer"
                            type="radio"
                            value={wisdomChess.ChessEngine}
                            defaultChecked={settings.blackPlayer === wisdomChess.ChessEngine}
                        />
                        Computer
                    </label>
                </div>

                <div>Flip Board</div>
                <div className="flip-board">
                    <input
                        type="checkbox"
                        name="flipped"
                        ref={flippedRef}
                        value="1"
                        defaultChecked={props.flipped}
                    />

                </div>

                <div>Thinking Time</div>
                <div className="thinking-time">
                    <label>
                        0:{thinkingTime < 10 ? '0' + thinkingTime : thinkingTime}
                    </label>
                    <Slider
                        min={1}
                        max={30}
                        value={thinkingTime}
                        onChange={value => setThinkingTime(Number(value))}/>
                </div>

                <div>Search Depth</div>
                <div className="search-depth">
                    <label>{searchDepth} {searchDepth > 1 ? 'moves' : 'move'}</label>
                    <Slider
                        min={1}
                        max={8}
                        value={searchDepth}
                        onChange={value => setSearchDepth(Number(value))}
                    />
                </div>

                <div className="buttons grid-columns-1-3">
                    <button className="btn-highlight" onClick={handleApply}>Apply</button>
                    <button onClick={props.onDismiss}>Cancel</button>
                </div>
            </form>
        </Modal>
    )
}
