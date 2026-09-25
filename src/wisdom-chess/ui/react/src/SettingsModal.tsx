import Modal from "./Modal";
import React, { useState } from "react";
import {
    WebGameSettings,
    WisdomChess
} from "./lib/WisdomChess";
import "./Settings.css"

type SettingsModalProps = {
    flipped: boolean
    settings: WebGameSettings
    onApply: (newSettings: WebGameSettings, flipped: boolean) => void
    onDismiss: () => void
}

export function SettingsModal(props: SettingsModalProps) {
    const wisdomChess = WisdomChess()
    const [settings, setSettings] = useState(props.settings)
    const [flipped, setFlipped] = useState(props.flipped)
    const { thinkingTime, searchDepth } = settings

    const update = (changes: Partial<WebGameSettings>) =>
        setSettings(current => ({ ...current, ...changes }))

    const handleApply = (e: React.SyntheticEvent) => {
        e.preventDefault()
        props.onApply(settings, flipped)
    }

    const playerOptions = (name: 'whitePlayer' | 'blackPlayer') => (
        <div className="player-options">
            <label>
                <input
                    name={name}
                    type="radio"
                    checked={settings[name] === wisdomChess.Human}
                    onChange={() => update({ [name]: wisdomChess.Human })}
                />
                Human
            </label>
            <label>
                <input
                    name={name}
                    type="radio"
                    checked={settings[name] === wisdomChess.ChessEngine}
                    onChange={() => update({ [name]: wisdomChess.ChessEngine })}
                />
                Computer
            </label>
        </div>
    )

    return (
        <Modal>
            <h1>Settings</h1>
            <form className="settings">
                <div>White Player</div>
                {playerOptions('whitePlayer')}

                <div>Black Player</div>
                {playerOptions('blackPlayer')}

                <div>Flip Board</div>
                <div className="flip-board">
                    <input
                        type="checkbox"
                        name="flipped"
                        checked={flipped}
                        onChange={e => setFlipped(e.target.checked)}
                    />
                </div>

                <div>Debug Logging</div>
                <div className="debug-logging">
                    <input
                        type="checkbox"
                        name="debugLogging"
                        checked={settings.debugLogging}
                        onChange={e => update({ debugLogging: e.target.checked })}
                    />
                </div>

                <div>Thinking Time</div>
                <div className="thinking-time">
                    <label>
                        0:{String(thinkingTime).padStart(2, '0')}
                    </label>
                    <input
                        type="range"
                        name="thinkingTime"
                        min={1}
                        max={10}
                        value={thinkingTime}
                        onChange={e => update({ thinkingTime: Number(e.target.value) })}
                    />
                </div>

                <div>Search Depth</div>
                <div className="search-depth">
                    <label>{searchDepth} {searchDepth > 1 ? 'moves' : 'move'}</label>
                    <input
                        type="range"
                        name="searchDepth"
                        min={1}
                        max={8}
                        value={searchDepth}
                        onChange={e => update({ searchDepth: Number(e.target.value) })}
                    />
                </div>

                <div className="buttons grid-columns-1-3">
                    <button type="button" className="btn-highlight" onClick={handleApply}>Apply</button>
                    <button type="button" onClick={props.onDismiss}>Cancel</button>
                </div>
            </form>
        </Modal>
    )
}
