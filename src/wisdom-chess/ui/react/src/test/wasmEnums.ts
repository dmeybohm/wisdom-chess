import { wasmEnumValues } from './wasm-enum-values'
import type { WisdomChess } from '../lib/WisdomChess'

// The enum constants of the WASM module: `White`, `Pawn`, `Playing` and so on.
export type WasmEnums = {
    [K in keyof WisdomChess as WisdomChess[K] extends number ? K : never]: WisdomChess[K]
}

wasmEnumValues satisfies Record<keyof WasmEnums, number>

// The module's enum constants with their real values, for test doubles.
export const wasmEnums = wasmEnumValues as unknown as WasmEnums
