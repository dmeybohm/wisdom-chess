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

export type Color = 'white' | 'black'

export interface Piece {
    id: number
    icon: string
    color: Color
    position: string
}

export const initialPieces : Piece[] = [
    {
        id: 1,
        icon: WhitePawn,
        color: 'white',
        position: "a2",
    },
    {
        id: 2,
        icon: WhitePawn,
        color: 'white',
        position: "b2",
    },
    {
        id: 3,
        icon: WhitePawn,
        color: 'white',
        position: "c2",
    },
    {
        id: 4,
        icon: WhitePawn,
        color: 'white',
        position: "d2",
    },
    {
        id: 5,
        icon: WhitePawn,
        color: 'white',
        position: "e2",
    },
    {
        id: 6,
        icon: WhitePawn,
        color: 'white',
        position: "f2",
    },
    {
        id: 7,
        icon: WhitePawn,
        color: 'white',
        position: "g2",
    },
    {
        id: 8,
        icon: WhitePawn,
        color: 'white',
        position: "h2",
    },
    {
        id: 9,
        icon: BlackPawn,
        color: 'black',
        position: "a7",
    },
    {
        id: 10,
        icon: BlackPawn,
        color: 'black',
        position: "b7",
    },
    {
        id: 11,
        icon: BlackPawn,
        color: 'black',
        position: "c7",
    },
    {
        id: 12,
        icon: BlackPawn,
        color: 'black',
        position: "d7",
    },
    {
        id: 13,
        icon: BlackPawn,
        color: 'black',
        position: "e7",
    },
    {
        id: 14,
        icon: BlackPawn,
        color: 'black',
        position: "f7",
    },
    {
        id: 15,
        icon: BlackPawn,
        color: 'black',
        position: "g7",
    },
    {
        id: 16,
        icon: BlackPawn,
        color: 'black',
        position: "h7",
    },
    {
        id: 17,
        icon: BlackRook,
        color: 'black',
        position: "a8",
    },
    {
        id: 18,
        icon: BlackKnight,
        color: 'black',
        position: "b8",
    },
    {
        id: 19,
        icon: BlackBishop,
        color: 'black',
        position: "c8",
    },
    {
        id: 20,
        icon: BlackQueen,
        color: 'black',
        position: "d8",
    },
    {
        id: 21,
        icon: BlackKing,
        color: 'black',
        position: "e8",
    },
    {
        id: 22,
        icon: BlackBishop,
        color: 'black',
        position: "f8",
    },
    {
        id: 23,
        icon: BlackKnight,
        color: 'black',
        position: "g8",
    },
    {
        id: 24,
        icon: BlackRook,
        color: 'black',
        position: "h8",
    },
    {
        id: 25,
        icon: WhiteRook,
        color: 'white',
        position: "a1",
    },
    {
        id: 26,
        icon: WhiteKnight,
        color: 'white',
        position: "b1",
    },
    {
        id: 27,
        icon: WhiteBishop,
        color: 'white',
        position: "c1",
    },
    {
        id: 28,
        icon: WhiteQueen,
        color: 'white',
        position: "d1",
    },
    {
        id: 29,
        icon: WhiteKing,
        color: 'white',
        position: "e1",
    },
    {
        id: 30,
        icon: WhiteBishop,
        color: 'white',
        position: "f1",
    },
    {
        id: 31,
        icon: WhiteKnight,
        color: 'white',
        position: "g1",
    },
    {
        id: 32,
        icon: WhiteRook,
        color: 'white',
        position: "h1",
    },
]