import { Piece } from "./Pieces";
import type WisdomChessModule from './wisdom-chess-module'

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

export type ChessEngineEventType = 'computerMoved' | 'computerDrawStatusUpdated'

export type ReceiveWorkerMessageCallback =
    (type: ChessEngineEventType, gameId: number, message: string) => void;

export interface ReactWindow {
    startReact: () => void
    receiveWorkerMessage: ReceiveWorkerMessageCallback
    setReceiveWorkerMessageCallback: (callback: ReceiveWorkerMessageCallback) => void
}

export type WebGameSettings = {
    whitePlayer: WebPlayer
    blackPlayer: WebPlayer
    thinkingTime: number
    searchDepth: number
    debugLogging: boolean
}

// The module and its classes and enums, as described by the IDL:
export type WisdomChess = typeof WisdomChessModule
export type Game = WisdomChessModule.WebGame
export type GameModel = WisdomChessModule.GameModel
export type GameSettings = WisdomChessModule.GameSettings
export type ColoredPiece = WisdomChessModule.WebColoredPiece

export type WebPlayer = WisdomChessModule.wisdom_WebPlayer
export type PieceColor = WisdomChessModule.wisdom_WebColor
export type PieceType = WisdomChessModule.wisdom_WebPiece
export type GameStatus = WisdomChessModule.wisdom_WebGameStatus
export type DrawProposed = WisdomChessModule.wisdom_WebDrawStatus
export type DrawByRepetitionType = WisdomChessModule.wisdom_WebDrawByRepetitionType

// A C++ object that JavaScript owns and has to destroy.
export type WasmObject = Game | GameModel | GameSettings

// Returned by Game.makeHumanMove() when the move is not legal.
export const ILLEGAL_MOVE = -1

export interface WisdomWindow extends ReactWindow {
    wisdomChessWeb: WisdomChess
    wisdomChessGameModel: GameModel
    wisdomChessCurrentGame?: Game
}

// The page's globals, set up by the WebAssembly loader and main.tsx.
export function getWisdomWindow(): WisdomWindow {
    return (window as unknown) as WisdomWindow
}

export function getGameModel(): GameModel {
    return getWisdomWindow().wisdomChessGameModel
}

export function getCurrentGame (): Game {
    const wisdomWindow = getWisdomWindow()
    if (!wisdomWindow.wisdomChessCurrentGame) {
        const gameModel = getGameModel()
        wisdomWindow.wisdomChessCurrentGame =  gameModel.startNewGame()
    }
    return wisdomWindow.wisdomChessCurrentGame;
}

export function startNewGame(): Game {
    const wisdomChess = WisdomChess()
    const wisdomWindow = getWisdomWindow()
    if (wisdomWindow.wisdomChessCurrentGame) {
        wisdomChess.destroy(wisdomWindow.wisdomChessCurrentGame)
        delete wisdomWindow.wisdomChessCurrentGame
    }
    return getCurrentGame()
}

// Copies the settings out of the C++ object and frees it.
export function getCurrentGameSettings(): WebGameSettings {
    const wasmSettings = getGameModel().getCurrentGameSettings()
    try {
        return {
            whitePlayer: wasmSettings.whitePlayer,
            blackPlayer: wasmSettings.blackPlayer,
            thinkingTime: wasmSettings.thinkingTime,
            searchDepth: wasmSettings.searchDepth,
            debugLogging: Boolean(wasmSettings.debugLogging),
        }
    } finally {
        WisdomChess().destroy(wasmSettings)
    }
}

// Objects returned across the WebIDL boundary are owned by the caller.
// Runs the callback and then frees every object it was given.
export function withWasmObjects<T>(objects: WasmObject[], callback: () => T): T {
    try {
        return callback()
    } finally {
        const wisdomChess = WisdomChess()
        for (const object of objects) {
            wisdomChess.destroy(object)
        }
    }
}

export function WisdomChess(): WisdomChess {
    return getWisdomWindow().wisdomChessWeb
}

function mapPieceToIcon(piece: ColoredPiece): string {
    const wisdomChess = WisdomChess()
    const isWhite = piece.color === wisdomChess.White
    switch (piece.piece) {
        case wisdomChess.Pawn: return isWhite ? WhitePawn : BlackPawn
        case wisdomChess.Knight: return isWhite ? WhiteKnight : BlackKnight
        case wisdomChess.Bishop: return isWhite ? WhiteBishop : BlackBishop
        case wisdomChess.Rook: return isWhite ? WhiteRook : BlackRook
        case wisdomChess.Queen: return isWhite ? WhiteQueen : BlackQueen
        case wisdomChess.King: return isWhite ? WhiteKing : BlackKing
        default: throw new Error("invalid piece type")
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
           color: piece.color,
           position: fromRowAndColToStringCoord(piece.row, piece.col)
       }
       result.push(newPiece)
    }

    return result
}

export function pieceColorToString(pieceColor: PieceColor) {
    const wisdom = WisdomChess()
    switch (pieceColor) {
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