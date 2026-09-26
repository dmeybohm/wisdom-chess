export type Position = {
    readonly position: string
    readonly index: number
    readonly isOddRow: boolean
}

const letters = [ 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' ];
const numbers = [ 8, 7, 6, 5, 4, 3, 2, 1 ];

export const initialSquares: readonly Position[] = numbers.flatMap((num, row) =>
    letters.map((letter, col) => ({
        position: letter + num,
        index: row * 8 + col,
        isOddRow: row % 2 > 0,
    }))
);
