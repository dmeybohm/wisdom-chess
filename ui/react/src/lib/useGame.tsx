import { Game, getCurrentGame, getPieces, PieceType, resetCurrentGame, WebMove } from "./WisdomChess";
import { initialSquares } from "./Squares";
import { Piece } from "./Pieces";
import { wisdomChess } from "../App";
import { create } from "zustand";

export const initialGameState = {
    pieces: [] as Piece[],
    squares: initialSquares,
    focusedSquare: '',
    pawnPromotionDialogSquare: '',
}

interface GameState {
    pieces: Piece[]
    squares: typeof initialSquares,
    focusedSquare: string
    pawnPromotionDialogSquare: string

    actions: {
        init: () => void
        humanMovePiece: (dst: string) => void
        computerMovePiece: (move: WebMove) => void
        pieceClick: (dst: string) => void
        promotePiece: (pieceType: PieceType) => void
        startNewGame: () => void;
    }
}

export const useGame = create<GameState>()((set, get) => ({
    ... initialGameState,

    actions: {
        init: () => set(state => {
            return { pieces: getPieces(getCurrentGame()) }
        }),
        startNewGame: () => set(state => {
            return  {
                ... initialGameState,
                pieces: getPieces(resetCurrentGame())
            }
        }),
        computerMovePiece: (move: WebMove) => set((prevState) => {
            const game = getCurrentGame()
            game.makeMove(move);
            return {
                pieces: getPieces(game)
            }
        }),
        pieceClick: (dst: string) => set((prevState) => {
            let newState : Partial<GameState> = {}
            const game = getCurrentGame()

            // begin focus if no focus already:
            if (prevState.focusedSquare === '') {
                newState.focusedSquare = dst
                newState.pawnPromotionDialogSquare = ''
                return newState
            }

            // toggle focus if already focused:
            if (prevState.focusedSquare === dst) {
                newState.focusedSquare = ''
                newState.pawnPromotionDialogSquare = ''
                return newState
            }

            // Change focus if piece is the same color:
            const pieces = getPieces(game)
            const dstPiece = findPieceAtPosition(pieces, dst)
            const srcPiece = findPieceAtPosition(pieces, prevState.focusedSquare)
            if (!dstPiece || !srcPiece || srcPiece.color === dstPiece.color) {
                newState.focusedSquare = dst
                newState.pawnPromotionDialogSquare = ''
                return newState
            }

            // take the piece with the move-piece action, and then remove the focus:
            get().actions.humanMovePiece(dst)
            const updatedState = get()
            newState.pieces = getPieces(game)
            if (updatedState.pawnPromotionDialogSquare === '')
                newState.focusedSquare = ''
            return newState
        }),
        promotePiece: (pieceType: PieceType) => set((prevState) => {
            const newState : Partial<GameState> = {}
            const game = getCurrentGame()
            const src = prevState.focusedSquare
            const dst = prevState.pawnPromotionDialogSquare

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
        }),
        humanMovePiece: (dst: string) => set((prevState): Partial<GameState> => {
            const game = getCurrentGame()
            const newState : Partial<GameState> = {
                pawnPromotionDialogSquare: '',
            };
            const src = prevState.focusedSquare;
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
        })
    }
}))

function findPieceAtPosition(pieces: Piece[], position: string): Piece|undefined {
    return pieces.find(piece => piece.position === position)
}