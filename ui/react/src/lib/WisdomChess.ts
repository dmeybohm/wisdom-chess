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

// These are already described in the iDL: TODO generated typescript from the IDL instead
export type Game = any
export type WisdomChess = any
export type PieceColor = any

interface ColoredPiece {
    color: number
    piece: number
}

interface WisdomWindow {
    wisdomChessWeb: unknown
}

export function getCurrentGame () {
    const wisdomChess = WisdomChess()
    if (!wisdomChess.currentGame) {
        wisdomChess.currentGame = new wisdomChess.WebGame(wisdomChess.Human, wisdomChess.Human)
        wisdomChess.currentGame.initializeWorker()
    }
    return wisdomChess.currentGame
}

export function WisdomChess(): WisdomChess {
    return ((window as unknown) as WisdomWindow).wisdomChessWeb as WisdomChess
}

function mapPieceToIcon(piece: ColoredPiece): string {
    const color = fromNumberToColor(piece.color)
    if (color === 'white') {
        switch (piece.piece)
        {
            case 1: return WhitePawn
            case 2: return WhiteKnight
            case 3: return WhiteBishop
            case 4: return WhiteRook
            case 5: return WhiteQueen
            case 6: return WhiteKing
            default: throw new Error("invalid piece type")
        }
    } else {
        switch (piece.piece)
        {
            case 1: return BlackPawn
            case 2: return BlackKnight
            case 3: return BlackBishop
            case 4: return BlackRook
            case 5: return BlackQueen
            case 6: return BlackKing
            default: throw new Error("invalid piece type")
        }
    }
}

function fromNumberToColor(color: number): Color {
    switch (color)
    {
        case 1: return 'white'
        case 2: return 'black'
        default: throw new Error("Invalid color")
    }
}

function fromRowAndColToStringCoord(row: number, col: number): string {
    let row_char = 8 - row;
    let col_code = 'a'.charCodeAt(0);
    col_code += col
    const col_char = String.fromCharCode(col_code)
    return col_char + row_char;
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

export function pieceColorToString(pieceColor: PieceColor) {
    const wisdom = WisdomChess()
    switch (pieceColor)
    {
        case wisdom.White:
            return "White";
        case wisdom.Black:
            return "Black";
        case wisdom.NoColor:
            return "No Color";
        default:
            return "Unknown color";
    }
}