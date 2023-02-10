import {Color, Piece} from "../Pieces";

import WhitePawn from "../assets/Chess_plt45.svg";
import WhiteBishop from "../assets/Chess_blt45.svg";
import WhiteKnight from "../assets/Chess_nlt45.svg";
import WhiteQueen from "../assets/Chess_qlt45.svg";
import WhiteRook from "../assets/Chess_rlt45.svg";
import WhiteKing from "../assets/Chess_klt45.svg";
import BlackPawn from "../assets/Chess_pdt45.svg";
import BlackBishop from "../assets/Chess_bdt45.svg";
import BlackKnight from "../assets/Chess_ndt45.svg";
import BlackQueen from "../assets/Chess_qdt45.svg";
import BlackRook from "../assets/Chess_rdt45.svg";
import BlackKing from "../assets/Chess_kdt45.svg";

interface Game {

    getPieceList (): ColoredPieceList
}

interface ColoredPieceList {
    length: number;
    pieceAt: (index: number) => ColoredPiece
}

interface ColoredPiece {
    id: number;
    color: number;
    piece: number;
    row: number;
    col: number;
}

interface WisdomWindow {
    game: Game
}

export function getGame (): Game {
    return ((window as unknown) as WisdomWindow).game as Game
}

function mapPieceToIcon(piece: ColoredPiece): string {
    return BlackKnight
}

function fromNumberToColor(color: number): Color {
    return "white"
}

function fromRowAndColToStringCoord(row: number, col: number): string {
    return "a8"
}

export function getPieces(game: Game): Piece[] {
    const result : Piece[] = []

    // Convert the pieces to appropriate format:
    const pieceList = game.getPieceList()

    for (let i = 0; i < pieceList.length; i++) {
       const piece = pieceList.pieceAt(i);
       const newPiece : Piece = {
           id: piece.id,
           icon: mapPieceToIcon(piece),
           color: fromNumberToColor(piece.color),
           position: fromRowAndColToStringCoord(piece.row, piece.col)
       }
       result.push(newPiece)
    }

    return result
}