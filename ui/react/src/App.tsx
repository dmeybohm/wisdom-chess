import React, { useState } from 'react'
import reactLogo from './assets/react.svg'
import './App.css'
import Board from "./Board";
import TopMenu from "./TopMenu";
import StatusBar from "./StatusBar";

function App() {
  const [count, setCount] = useState(0)

  return (
    <div className="App">
        <TopMenu />
        <div className="container">
            <Board />

            <StatusBar />
        </div>
    </div>
  )
}

export default App
