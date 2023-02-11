import React, {useEffect, useState} from 'react'
import Square from "./Square";
import "./Board.css";
import {initialPieces, Piece} from "./Pieces";
import {initialSquares, Position} from "./Squares";
import "./Positions.css"
import {getPieces, makeGame, WisdomChess} from "./lib/WisdomChess";

interface WisdomWindow {
    worker: Worker
}

const Board = () => {
    const [squares, setSquares] = useState(initialSquares)
    const [focusedSquare, setFocusedSquare] = useState('')
    const [game, setGame] = useState(() => makeGame())
    const [pieces, setPieces] = useState(() => getPieces(game))

    useEffect(() => {
        setPieces(getPieces(game))
    }, [game])

    const wisdomChess : WisdomChess = WisdomChess()

    const findIndexOfPieceAtPosition = (position: string) => {
        return pieces.findIndex(piece => piece.position === position)
    }

    //
    // Either:
    // - change the focus to the current piece, or
    // - take the piece if the color of the piece is different
    //
    const handlePieceClick = (dstSquare: string) => {
        if (focusedSquare === '') {
            setFocusedSquare(dstSquare)
            return
        }
        if (focusedSquare === dstSquare) {
            setFocusedSquare('')
            return
        }
        const dstIndex = findIndexOfPieceAtPosition(dstSquare)
        const srcIndex = findIndexOfPieceAtPosition(focusedSquare)
        if (dstIndex === -1 || srcIndex === -1 || pieces[dstIndex].color === pieces[srcIndex].color) {
            setFocusedSquare(dstSquare)
            return
        }
        takePiece(srcIndex, dstIndex, dstSquare)
        setFocusedSquare('')
    }

    const takePiece = (srcIndex: number, dstIndex: number, dstSquare: string) => {
        const takenPiece = pieces[dstIndex]
        setPieces(pieces.map((piece, index) => {
                const result = { ... piece };
                if (index === srcIndex) {
                    result.position = dstSquare
                }
                return result
            })
            .filter(piece => piece.position !== dstSquare || piece.color !== takenPiece.color)
        )
    }

    const handleMovePiece = (dst: string) => {
        const src = focusedSquare;
        if (src === '') {
            return
        }
        // find the piece at the focused square:
        console.log(wisdomChess.WebCoord)
        console.log(src);
        console.log(dst);
        const srcCoord = wisdomChess.WebCoord.prototype.fromTextCoord(src)
        const dstCoord = wisdomChess.WebCoord.prototype.fromTextCoord(dst)
        const movedSuccess = game.makeMove(srcCoord, dstCoord)
        if (movedSuccess) {
            setPieces(oldPieces => getPieces(game))
        }
        //
        // const piecesCopy = pieces.map(piece => { return { ... piece }; })
        // piecesCopy[index].position = dst
        // setPieces(piecesCopy)
        // setFocusedSquare('');

        // const worker = ((window as unknown) as WisdomWindow).worker
        // console.log(worker);
        // worker.postMessage('moved ' + src + " -> " + dst)
    }

    return (
        <section className="board">
            {squares.map((position: Position) => {
                return (
                    <Square
                        key={position.index}
                        isOddRow={position.isOddRow}
                        position={position.position}
                        onClick={handleMovePiece}
                    />
                );
            })}
            {pieces.map((piece: Piece) => (
                <div className={`piece ${piece.position} ${piece.position === focusedSquare ? 'focused' : ''}`}
                     key={piece.id}
                     onClick={() => handlePieceClick(piece.position)}>
                    <img alt="piece" src={piece.icon} />
                </div>
            ))}
        </section>
    )
}

export default Board