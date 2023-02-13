import WhiteQueen from "./assets/Chess_qlt45.svg";
import WhiteRook from "./assets/Chess_rlt45.svg";
import WhiteBishop from "./assets/Chess_blt45.svg";
import WhiteKnight from "./assets/Chess_nlt45.svg";

export default function PawnPromotionDialog(props: { square: string, direction: number }) {
    const row: number = parseInt(props.square.charAt(1), 10)
    const reversed = row === 1 ? 'reversed' : '';

    return (
        <div className={`piece pawn-promotion-dialog ${props.square} ${reversed}`}>
            <div>
                <img alt="piece" src={WhiteQueen} />
            </div>
            <div>
                <img alt="piece" src={WhiteRook} />
            </div>
            <div>
                <img alt="piece" src={WhiteBishop} />
            </div>
            <div>
                <img alt="piece" src={WhiteKnight} />
            </div>
        </div>
    );
}
