import Modal from "./Modal";
import React from "react";
import "./DrawDialog.css"

type DrawDialogProps = {
    title: string
    onAccepted: () => void
    onDeclined: () => void
    children: JSX.Element[] | JSX.Element
}

export function DrawDialog(props: DrawDialogProps) {
    return (
        <Modal>
            <h1>{props.title}</h1>
            <form className="draw-dialog">
                {props.children}
                <p>Do you want to declare a draw?</p>
                <div className="buttons">
                    <button onClick={props.onAccepted}>Yes</button>
                    <button onClick={props.onDeclined}>No</button>
                </div>
            </form>
        </Modal>
    )
}
