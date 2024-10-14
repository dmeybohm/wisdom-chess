import React from 'react'
import ReactDOM from 'react-dom/client'
import App from './App'
import './index.css'
import { ChessEngineEventType, useGame } from "./lib/useGame";

function startReact(window: ReactWindow) {
    const state = useGame.getState()
    state.actions.init(window)

    ReactDOM.createRoot(document.getElementById('root') as HTMLElement).render(
        <React.StrictMode>
            <App/>
        </React.StrictMode>,
    )
}

export interface ReactWindow {
    startReact: (window: ReactWindow) => void
    receiveWorkerMessage: (type: ChessEngineEventType, gameId: number, message: string) => void
}

((window as unknown) as ReactWindow).startReact = startReact