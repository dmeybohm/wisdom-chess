import WhiteQueen from "./assets/Chess_qlt45.svg";
import WhiteRook from "./assets/Chess_rlt45.svg";
import WhiteBishop from "./assets/Chess_blt45.svg";
import WhiteKnight from "./assets/Chess_nlt45.svg";

import BlackQueen from "./assets/Chess_qdt45.svg";
import BlackRook from "./assets/Chess_rdt45.svg";
import BlackBishop from "./assets/Chess_bdt45.svg";
import BlackKnight from "./assets/Chess_ndt45.svg";

import { PieceColor, PieceType, WisdomChess } from "./lib/WisdomChess";
import { useState } from "react";

type PawnPromotionDialogProps = {
    square: string
    direction: number
    color: number
    selectedPiece: (pieceType: PieceType) => void
}

type PromotablePiece = {
    type: PieceColor,
    icon: any[]
}

export default function PawnPromotionDialog(props: PawnPromotionDialogProps) {
    const wisdomChess = WisdomChess()
    const [selectedPiece, setSelectedPiece] = useState<PieceColor>(wisdomChess.NoPiece)

    const pieces = [
        { type: wisdomChess.Queen, icon: [ WhiteQueen, BlackQueen] },
        { type: wisdomChess.Rook, icon: [ WhiteRook, BlackRook] },
        { type: wisdomChess.Bishop, icon: [ WhiteBishop, BlackBishop ] },
        { type: wisdomChess.Knight, icon: [ WhiteKnight, BlackKnight ] },
    ]

    const row: number = parseInt(props.square.charAt(1), 10)
    const reversed = row === 1 ? 'reversed' : '';

    function handleSelectPiece(piece: PromotablePiece) {
        if (piece.type === selectedPiece) {
            setSelectedPiece(wisdomChess.NoPiece)
            props.selectedPiece(selectedPiece)
        } else {
            setSelectedPiece(piece.type)
        }
    }

    return (
        <div className={`piece pawn-promotion-dialog ${props.square} ${reversed}`}>
            {pieces.map(piece => (
                <div
                    key={piece.type}
                    className={`pawn-promotion-dialog__piece ${selectedPiece === piece.type && 'selected'}`}
                    onClick={() => handleSelectPiece(piece)}
                >
                    <img
                        alt="piece"
                        src={props.color === wisdomChess.White ? piece.icon[0] : piece.icon[1]}
                    />
                </div>
            ))}
        </div>
    );
}
