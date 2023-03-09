import Modal from "./Modal";
import React, { useMemo } from "react";
import "./Settings.css"
import { useGame } from "./lib/useGame";
import { WisdomChess } from "./lib/WisdomChess";

type SettingsModalProps = {
    onApply: () => void
    onDismiss: () => void
}

export function SettingsModal(props: SettingsModalProps) {
    const settings = useGame((state) => state.settings)
    const wisdomChess = WisdomChess()
    return (
        <Modal>
            <h1>Settings Modal</h1>
            <form className="settings">
                <div>White Player</div>
                <label>
                    <input
                        name="whitePlayer"
                        type="radio"
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
                        value={wisdomChess.Computer}
                        defaultChecked={settings.blackPlayer === wisdomChess.ChessEngine}
                    />
                    Computer
                </label>
                <div className="buttons">
                    <button onClick={props.onApply}>Apply</button>
                    <button onClick={props.onDismiss}>Cancel</button>
                </div>
            </form>
        </Modal>
    )
}
