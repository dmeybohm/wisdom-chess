import WhiteQueen from "./assets/Chess_qlt45.svg";
import WhiteRook from "./assets/Chess_rlt45.svg";
import WhiteBishop from "./assets/Chess_blt45.svg";
import WhiteKnight from "./assets/Chess_nlt45.svg";

import BlackQueen from "./assets/Chess_qdt45.svg";
import BlackRook from "./assets/Chess_rdt45.svg";
import BlackBishop from "./assets/Chess_bdt45.svg";
import BlackKnight from "./assets/Chess_ndt45.svg";

import { WisdomChess } from "./lib/WisdomChess";

type PawnPromotionDialogProps = {
    square: string
    direction: number
    color: number
}

export default function PawnPromotionDialog(props: PawnPromotionDialogProps) {
    const row: number = parseInt(props.square.charAt(1), 10)
    const reversed = row === 1 ? 'reversed' : '';
    const wisdomChess = WisdomChess()
    return (
        <div className={`piece pawn-promotion-dialog ${props.square} ${reversed}`}>
            <div>
                <img alt="piece" src={props.color === wisdomChess.White ? WhiteQueen : BlackQueen} />
            </div>
            <div>
                <img alt="piece" src={props.color === wisdomChess.White ? WhiteRook : BlackRook} />
            </div>
            <div>
                <img alt="piece" src={props.color === wisdomChess.White ? WhiteBishop : BlackBishop} />
            </div>
            <div>
                <img alt="piece" src={props.color === wisdomChess.White ? WhiteKnight : BlackKnight} />
            </div>
        </div>
    );
}
