import WhitePawn from "./assets/Chess_plt45.svg";
import WhiteBishop from "./assets/Chess_blt45.svg";
import WhiteKnight from "./assets/Chess_nlt45.svg";
import WhiteQueen from "./assets/Chess_qlt45.svg";
import WhiteRook from "./assets/Chess_rlt45.svg";
import WhiteKing from "./assets/Chess_klt45.svg";
import BlackPawn from "./assets/Chess_pdt45.svg";
import BlackBishop from "./assets/Chess_bdt45.svg";
import BlackKnight from "./assets/Chess_ndt45.svg";
import BlackQueen from "./assets/Chess_qdt45.svg";
import BlackRook from "./assets/Chess_rdt45.svg";
import BlackKing from "./assets/Chess_kdt45.svg";

export interface Piece {
    type: string
    position: string
}

export const initialPieces : Piece[] = [
    {
        type: WhitePawn,
        position: "a2",
    },
    {
        type: WhitePawn,
        position: "b2",
    },
    {
        type: WhitePawn,
        position: "c2",
    },
    {
        type: WhitePawn,
        position: "d2",
    },
    {
        type: WhitePawn,
        position: "e2",
    },
    {
        type: WhitePawn,
        position: "f2",
    },
    {
        type: WhitePawn,
        position: "g2",
    },
    {
        type: WhitePawn,
        position: "h2",
    },
    {
        type: BlackPawn,
        position: "a7",
    },
    {
        type: BlackPawn,
        position: "b7",
    },
    {
        type: BlackPawn,
        position: "c7",
    },
    {
        type: BlackPawn,
        position: "d7",
    },
    {
        type: BlackPawn,
        position: "e7",
    },
    {
        type: BlackPawn,
        position: "f7",
    },
    {
        type: BlackPawn,
        position: "g7",
    },
    {
        type: BlackPawn,
        position: "h7",
    },
    {
        type: BlackRook,
        position: "a8",
    },
    {
        type: BlackKnight,
        position: "b8",
    },
    {
        type: BlackBishop,
        position: "c8",
    },
    {
        type: BlackQueen,
        position: "d8",
    },
    {
        type: BlackKing,
        position: "e8",
    },
    {
        type: BlackBishop,
        position: "f8",
    },
    {
        type: BlackKnight,
        position: "g8",
    },
    {
        type: BlackRook,
        position: "h8",
    },
    {
        type: WhiteRook,
        position: "a1",
    },
    {
        type: WhiteKnight,
        position: "b1",
    },
    {
        type: WhiteBishop,
        position: "c1",
    },
    {
        type: WhiteQueen,
        position: "d1",
    },
    {
        type: WhiteKing,
        position: "e1",
    },
    {
        type: WhiteBishop,
        position: "f1",
    },
    {
        type: WhiteKnight,
        position: "g1",
    },
    {
        type: WhiteRook,
        position: "h1",
    },
]