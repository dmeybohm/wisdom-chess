import { initialSquares } from './Squares'
import { Color, Piece } from "./Pieces";

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
    startReact: (window: ReactWindow) => void
    receiveWorkerMessage: ReceiveWorkerMessageCallback
    setReceiveWorkerMessageCallback: (callback: ReceiveWorkerMessageCallback) => void
}

export type WebGameSettings = {
    whitePlayer: number
    blackPlayer: number
    thinkingTime: number
    searchDepth: number
    difficulty: number  // 0=Easy, 1=Medium, 2=Hard
}

export type GameState = {
    pieces: Piece[]
    squares: typeof initialSquares
    focusedSquare: string
    pawnPromotionDialogSquare: string
    lastDroppedSquare: string
    gameStatus: GameStatus
    moveStatus: string
    gameOverStatus: string
    settings: WebGameSettings
    hasHumanPlayer: boolean
}

// These are already described in the IDL:
export type GameModel = {
    startNewGame(): Game
    getCurrentGame(): Game
    getCurrentGameSettings(): GameSettings
    setCurrentGameSettings(newSettings: WorkerGameSettings): void
    getFirstHumanPlayerColor: PieceColor
    getSecondHumanPlayerColor: PieceColor
    notifyHumanMove(move: WebMove): void;
    notifyComputerMove(): void;
    sendPause(): void;
    sendUnpause(): void;
}

interface GameSettingsConstructor {
    new(): GameSettings
    new(
        whitePlayer: WebPlayer,
        blackPlayer: WebPlayer,
        thinkingTime: number,
        searchDepth: number
    ): GameSettings
}

export type WisdomChess = {
    White: PieceColor
    Black: PieceColor
    NoColor: PieceColor

    Human: WebPlayer
    ChessEngine: WebPlayer

    Queen: PieceType
    Rook: PieceType
    Bishop: PieceType
    Knight: PieceType
    NoPiece: PieceType

    Easy: WebDifficulty
    Medium: WebDifficulty
    Hard: WebDifficulty

    GameSettings: GameSettingsConstructor

    NotReached: DrawProposed
    Proposed: DrawProposed
    Accepted: DrawProposed
    Declined: DrawProposed

    // Web move constructor:
    WebMove: any
    WebCoord: any

    Playing: GameStatus
    Checkmate: GameStatus
    Stalemate: GameStatus
    ThreefoldRepetitionReached: GameStatus
    ThreefoldRepetitionAccepted: GameStatus
    FivefoldRepetitionDraw: GameStatus
    FiftyMovesWithoutProgressReached: GameStatus
    FiftyMovesWithoutProgressAccepted: GameStatus
    SeventyFiveMovesWithoutProgressDraw: GameStatus
    InsufficientMaterialDraw: GameStatus

    ThreefoldRepetition: DrawByRepetitionType
    FiftyMovesWithoutProgress: DrawByRepetitionType

    // Destroy a C++ object explicitly:
    destroy(obj: any): void
}

export type Game = any
export type WebPlayer = any
export type WebDifficulty = any

export type PieceColor = any
export type PieceType = any
export type DrawProposed = any
export type DrawByRepetitionType = any

export type GameStatus = any

export type WebMove = {
    asString(): string
}

interface ColoredPiece {
    color: number
    piece: number
}

export type GameSettings = {
    whitePlayer: WebPlayer
    blackPlayer: WebPlayer
    thinkingTime: number
    searchDepth: number
    difficulty: number  // 0=Easy, 1=Medium, 2=Hard
}

export type WorkerGameSettings = any

export interface WisdomWindow extends ReactWindow {
    wisdomChessWeb: unknown
    wisdomChessGameModel: GameModel
    wisdomChessCurrentGame: Game
}

export function getGameModel(): GameModel {
    const wisdomWindow = ((window as unknown) as WisdomWindow)
    return wisdomWindow.wisdomChessGameModel
}

export function getCurrentGame (): Game {
    const wisdomWindow = ((window as unknown) as WisdomWindow)
    if (!wisdomWindow.wisdomChessCurrentGame) {
        const gameModel = getGameModel()
        wisdomWindow.wisdomChessCurrentGame =  gameModel.startNewGame()
    }
    return wisdomWindow.wisdomChessCurrentGame;
}

export function startNewGame(): Game {
    const wisdomChess = WisdomChess()
    const wisdomWindow = ((window as unknown) as WisdomWindow)
    if (wisdomWindow.wisdomChessCurrentGame) {
        wisdomChess.destroy(wisdomWindow.wisdomChessCurrentGame)
        delete wisdomWindow.wisdomChessCurrentGame
    }
    return getCurrentGame()
}

export function getCurrentGameSettings(): GameSettings {
    const gameModel = getGameModel()
    return gameModel.getCurrentGameSettings()
}

export function WisdomChess(): WisdomChess {
    return ((window as unknown) as WisdomWindow).wisdomChessWeb as WisdomChess
}

function mapPieceToIcon(piece: ColoredPiece): string {
    const color = fromNumberToColor(piece.color)
    if (color === 'white') {
        switch (piece.piece) {
            case 1: return WhitePawn
            case 2: return WhiteKnight
            case 3: return WhiteBishop
            case 4: return WhiteRook
            case 5: return WhiteQueen
            case 6: return WhiteKing
            default: throw new Error("invalid piece type")
        }
    } else {
        switch (piece.piece) {
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
    switch (color) {
        case 1: return 'white'
        case 2: return 'black'
        default: throw new Error("Invalid color")
    }
}

export function fromColorToNumber(color: string): PieceColor {
    const wisdomChess = WisdomChess()
    switch (color) {
        case 'white': return wisdomChess.White
        case 'black': return wisdomChess.Black
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