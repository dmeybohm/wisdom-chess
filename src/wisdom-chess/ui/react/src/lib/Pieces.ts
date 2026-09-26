import type { PieceColor } from './WisdomChess'

export interface Piece {
    id: number
    icon: string
    color: PieceColor
    position: string
}