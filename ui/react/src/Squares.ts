export class Position {
    #position: string
    #index: number

    constructor(position: string, index: number) {
        this.#position = position
        this.#index = index
    }

    get position() {
        return this.#position;
    }

    get index() {
        return this.#index;
    }

    get isOddRow(): boolean {
        let row = Math.floor(this.#index / 8);
        return Boolean(row % 2 > 0)
    }
}

const initialSquares : Array<Position> = [];

const letters = [ 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' ];
const numbers = [ 8, 7, 6, 5, 4, 3, 2, 1 ];

let index = 0

for (let num of numbers) {
    for (let letter of letters) {
        initialSquares.push(new Position(letter + num, index));
        index++
    }
}

export { initialSquares as initialSquares };
