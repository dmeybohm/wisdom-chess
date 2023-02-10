import React from 'react'
import ReactDOM from 'react-dom/client'
import App from './App'
import './index.css'

function startReact() {
    ReactDOM.createRoot(document.getElementById('root') as HTMLElement).render(
        <React.StrictMode>
            <App/>
        </React.StrictMode>,
    )
}
interface ReactWindow {
    startReact: () => void;
}
((window as unknown) as ReactWindow).startReact = startReact