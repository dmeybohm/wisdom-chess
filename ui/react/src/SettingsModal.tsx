import Modal from "./Modal";
import React, { useMemo, useRef } from "react";
import "./Settings.css"
import { useGame } from "./lib/useGame";
import { GameSettings, WisdomChess } from "./lib/WisdomChess";

type SettingsModalProps = {
    onApply: (newSettings: GameSettings) => void
    onDismiss: () => void
}

export function SettingsModal(props: SettingsModalProps) {
    const settings = useGame((state) => state.settings)
    const wisdomChess = WisdomChess()
    const humanWhite = useRef<HTMLInputElement|null>(null)
    const humanBlack = useRef<HTMLInputElement|null>(null)

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
            settings.thinkingTime,
            settings.searchDepth
        )
        props.onApply(newSettings)
    }

    return (
        <Modal>
            <h1>Settings Modal</h1>
            <form className="settings">
                <div>White Player</div>
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
                <div>Black Player</div>
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
                <div className="buttons">
                    <button onClick={handleApply}>Apply</button>
                    <button onClick={props.onDismiss}>Cancel</button>
                </div>
            </form>
        </Modal>
    )
}
