// Generated from ui/wasm/wisdom-chess.idl. Do not edit.
// Regenerate with: npm run generate:wasm-types
export const wasmEnumValues = {
    // wisdom_WebPlayer
    Human: 0,
    ChessEngine: 1,
    // wisdom_WebColor
    NoColor: 0,
    White: 1,
    Black: 2,
    // wisdom_WebPiece
    NoPiece: 0,
    Pawn: 1,
    Knight: 2,
    Bishop: 3,
    Rook: 4,
    Queen: 5,
    King: 6,
    // wisdom_WebGameStatus
    Playing: 0,
    Checkmate: 1,
    Stalemate: 2,
    ThreefoldRepetitionReached: 3,
    ThreefoldRepetitionAccepted: 4,
    FivefoldRepetitionDraw: 5,
    FiftyMovesWithoutProgressReached: 6,
    FiftyMovesWithoutProgressAccepted: 7,
    SeventyFiveMovesWithoutProgressDraw: 8,
    InsufficientMaterialDraw: 9,
    // wisdom_WebDrawStatus
    NotReached: 0,
    Proposed: 1,
    Accepted: 2,
    Declined: 3,
    // wisdom_WebDrawByRepetitionType
    ThreefoldRepetition: 0,
    FiftyMovesWithoutProgress: 1,
} as const
