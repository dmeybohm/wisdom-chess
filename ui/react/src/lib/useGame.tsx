import { Game, getPieces, PieceType, WebMove } from "./WisdomChess";
import { useState } from "react";
import { initialSquares } from "./Squares";
import { Piece } from "./Pieces";
import { wisdomChess } from "../App";

export const initialGameState = {
    pieces: [] as Piece[],
    squares: initialSquares,
    focusedSquare: '',
    pawnPromotionDialogSquare: '',
}

type GameState = typeof initialGameState;

export function useGame(initialGameState: GameState, game: Game) {
    const [gameState, setGameState] = useState<GameState>({
        pieces: getPieces(game),
        squares: initialSquares,
        focusedSquare: '',
        pawnPromotionDialogSquare: '',
    })

    //
    // Wrap action functions to update the state with their return values.
    //
    const actions = {
        humanMovePiece: (dst: string) =>
            setGameState(movePiece(gameState, game, dst)),
        pieceClick: (dst: string) =>
            setGameState(pieceClick(gameState, game, dst)),
        computerMovePiece: (move: WebMove) =>
            setGameState(computerMovePiece(gameState, game, move)),
        promotePiece: (pieceType: PieceType) =>
            setGameState(promotePiece(gameState, game, pieceType))
    }

    return [
        gameState,
        actions
    ] as const
}

function findPieceAtPosition(pieces: Piece[], position: string): Piece|undefined {
    return pieces.find(piece => piece.position === position)
}

function movePiece(prevState: GameState, game: Game, dst: string): GameState {
    let newState = { ... prevState }

    newState.pawnPromotionDialogSquare = ''
    const src = newState.focusedSquare;
    if (src === '') {
        return newState
    }
    const srcCoord = wisdomChess.WebCoord.prototype.fromTextCoord(src)
    const dstCoord = wisdomChess.WebCoord.prototype.fromTextCoord(dst)

    const move = game.createMoveFromCoordinatesAndPromotedPiece(
        srcCoord,
        dstCoord,
        wisdomChess.Queen
    );
    if (!move || !game.isLegalMove(move)) {
        newState.focusedSquare = ''
        return newState
    }
    if (game.needsPawnPromotion(srcCoord, dstCoord)) {
        newState.pawnPromotionDialogSquare = dst
        return newState
    }

    newState.focusedSquare = ''
    game.makeMove(move)
    newState.pieces = getPieces(game)
    return newState
}

//
// Either:
// - change the focus to the current piece, or
// - take the piece if the color of the piece is different
//
function pieceClick(prevState: GameState, game: Game, dst: string): GameState {
    let newState = { ... prevState }

    // begin focus if no focus already:
    if (newState.focusedSquare === '') {
        newState.focusedSquare = dst
        newState.pawnPromotionDialogSquare = ''
        return newState
    }

    // toggle focus if already focused:
    if (newState.focusedSquare === dst) {
        newState.focusedSquare = ''
        newState.pawnPromotionDialogSquare = ''
        return newState
    }

    // Change focus if piece is the same color:
    const pieces = getPieces(game)
    const dstPiece = findPieceAtPosition(pieces, dst)
    const srcPiece = findPieceAtPosition(pieces, newState.focusedSquare)
    if (!dstPiece || !srcPiece || srcPiece.color === dstPiece.color) {
        newState.focusedSquare = dst
        newState.pawnPromotionDialogSquare = ''
        return newState
    }

    // take the piece with the move-piece action, and then remove the focus:
    newState = movePiece(newState, game, dst)
    newState.pieces = getPieces(game)
    if (newState.pawnPromotionDialogSquare === '')
        newState.focusedSquare = ''
    return newState
}

function computerMovePiece(prevState: GameState, game: Game, move: WebMove): GameState {
    game.makeMove(move)
    return {
        ... prevState,
        pieces: getPieces(game)
    }
}

function promotePiece(prevState: GameState, game: Game, pieceType: PieceType): GameState {
    const newState = { ... prevState }
    const src = newState.focusedSquare
    const dst = newState.pawnPromotionDialogSquare

    newState.pawnPromotionDialogSquare = ''
    newState.focusedSquare = ''

    const srcCoord = wisdomChess.WebCoord.prototype.fromTextCoord(src)
    const dstCoord = wisdomChess.WebCoord.prototype.fromTextCoord(dst)

    const move = game.createMoveFromCoordinatesAndPromotedPiece(
        srcCoord,
        dstCoord,
        pieceType,
    );
    if (!move || !game.isLegalMove(move)) {
        return newState
    }
    game.makeMove(move)
    newState.pieces = getPieces(game)
    return newState
}
