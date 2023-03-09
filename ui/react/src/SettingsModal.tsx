import Modal from "./Modal";
import React from "react";
import "./Settings.css"

type SettingsModalProps = {
    show: boolean
    onApply: () => void
    onDismiss: () => void
}

export function SettingsModal(props: SettingsModalProps) {
    return (
        <Modal
            show={props.show}
        >
            <h1>Settings Modal</h1>
            <form>
                <fieldset>
                    <legend>White Player</legend>
                    <input id="humanWhite" name="whitePlayer" type="radio" />
                    <label htmlFor="humanWhite">Human</label>
                    <input id="computerWhite" name="whitePlayer" type="radio" />
                    <label htmlFor="computerWhite">Computer</label>
                </fieldset>
                <fieldset>
                    <legend>Black Player</legend>
                    <input id="humanBlack" name="blackPlayer" type="radio" />
                    <label htmlFor="humanBlack">Human</label>
                    <input id="computerBlack" name="blackPlayer" type="radio" />
                    <label htmlFor="computerBlack">Computer</label>
                </fieldset>
                <button onClick={props.onApply}>Apply</button>
                <button onClick={props.onDismiss}>Cancel</button>
            </form>
        </Modal>
    )
}
