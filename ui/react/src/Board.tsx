import React, {useContext} from 'react'
import Square from "./Square";
import "./Board.css";
import {Piece} from "./Pieces";
import "./Positions.css"
import PawnPromotionDialog from "./PawnPromotionDialog";
import {ChessGameContext} from "./lib/ChessGameContext";
import {Position} from "./Squares";

const Board = () => {
    const context = useContext(ChessGameContext)
    if (!context) {
        return <></>;
    }
    return (
        <section className="board">
            {context.squares.map((position: Position) => {
                return (
                    <Square
                        key={position.index}
                        isOddRow={position.isOddRow}
                        position={position.position}
                        onClick={context.actions.handleMovePiece}
                    />
                );
            })}
            {context.pieces.map((piece: Piece) => (
                <div className={`piece ${piece.position} ${piece.position === context.focusedSquare ? 'focused' : ''}`}
                     key={piece.id}
                     onClick={() => context.actions.handlePieceClick(piece.position)}>
                    <img alt="piece" src={piece.icon} />
                </div>
            ))}
            {context.pawnPromotionDialogSquare &&
                <PawnPromotionDialog square={context.pawnPromotionDialogSquare} />}
        </section>
    )
}

export default Board