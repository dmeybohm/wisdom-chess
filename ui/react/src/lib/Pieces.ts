export type Color = 'white' | 'black'

export interface Piece {
    id: number
    icon: string
    color: Color
    position: string
}