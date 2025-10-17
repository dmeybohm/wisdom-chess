import React from 'react'
import ReactDOM from 'react-dom/client'
import App from './App'
import './index.css'

type ChessEngineEventType = 'computerMoved' | 'computerDrawStatusUpdated'

let receiveWorkerMessageCallback: ((type: ChessEngineEventType, gameId: number, message: string) => void) | null = null

function startReact(window: ReactWindow) {
    const root = ReactDOM.createRoot(document.getElementById('root') as HTMLElement)

    root.render(
        <React.StrictMode>
            <App/>
        </React.StrictMode>,
    )
}

export interface ReactWindow {
    startReact: (window: ReactWindow) => void
    receiveWorkerMessage: (type: ChessEngineEventType, gameId: number, message: string) => void
    setReceiveWorkerMessageCallback: (callback: (type: ChessEngineEventType, gameId: number, message: string) => void) => void
}

const reactWindow = ((window as unknown) as ReactWindow)
reactWindow.startReact = startReact
reactWindow.setReceiveWorkerMessageCallback = (callback) => {
    receiveWorkerMessageCallback = callback
}
reactWindow.receiveWorkerMessage = (type: ChessEngineEventType, gameId: number, message: string) => {
    if (receiveWorkerMessageCallback) {
        receiveWorkerMessageCallback(type, gameId, message)
    }
}