import React from 'react'
import ReactDOM from 'react-dom/client'
import App from './App'
import './index.css'
import {
    ReceiveWorkerMessageCallback,
    ChessEngineEventType,
    ReactWindow
} from "./lib/WisdomChess"

let receiveWorkerMessageCallback: ReceiveWorkerMessageCallback | null = null
let pending: Array<[ChessEngineEventType, number, string]> = []

function startReact(window: ReactWindow) {
    const root = ReactDOM.createRoot(document.getElementById('root') as HTMLElement)
    root.render(
        <React.StrictMode>
            <App />
        </React.StrictMode>,
    )
}

const reactWindow = (window as unknown) as ReactWindow
reactWindow.startReact = startReact
reactWindow.setReceiveWorkerMessageCallback = (cb: ReceiveWorkerMessageCallback) => {
    receiveWorkerMessageCallback = cb
    if (receiveWorkerMessageCallback && pending.length) {
        for (const args of pending) receiveWorkerMessageCallback(...args)
        pending = []
    }
}
reactWindow.receiveWorkerMessage = (type, gameId, message) => {
    if (receiveWorkerMessageCallback) {
        receiveWorkerMessageCallback(type, gameId, message)
    } else {
        pending.push([type, gameId, message])
    }
}
