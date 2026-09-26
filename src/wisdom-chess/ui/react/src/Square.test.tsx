import { describe, it, expect, beforeEach, vi } from 'vitest'
import { fireEvent, render, screen, waitFor } from '@testing-library/react'
import { DndProvider } from 'react-dnd'
import { HTML5Backend } from 'react-dnd-html5-backend'
import { PieceOverlay } from './Square'
import type { Game, WisdomChess } from './lib/WisdomChess'
import { getWisdomWindow } from './lib/WisdomChess'
import { wasmEnums } from './test/wasmEnums'

// jsdom has no DataTransfer; this is what the HTML5 backend touches.
const dataTransfer = () => ({
    types: [],
    setData: vi.fn(),
    getData: vi.fn(() => ''),
    setDragImage: vi.fn(),
})

// jsdom has no PointerEvent either, so the type rides on a plain event.
const pointerDown = (target: Element, pointerType: string) => {
    const event = new Event('pointerdown', { bubbles: true })
    Object.defineProperty(event, 'pointerType', { value: pointerType })
    fireEvent(target, event)
}

const renderWhitePawn = () => render(
    <DndProvider backend={HTML5Backend}>
        <PieceOverlay
            piece={{ id: 1, icon: 'pawn.svg', color: wasmEnums.White, position: 'e2' }}
            focusedSquare=""
            droppedSquare=""
            currentTurn={wasmEnums.White}
            onPieceClick={() => {}}
            onDropPiece={() => {}}
        />
    </DndProvider>
)

describe('PieceOverlay', () => {
    beforeEach(() => {
        const wisdomWindow = getWisdomWindow()
        wisdomWindow.wisdomChessWeb = { ...wasmEnums } as unknown as WisdomChess
        wisdomWindow.wisdomChessCurrentGame = {
            getPlayerOfColor: vi.fn(() => wasmEnums.Human),
        } as unknown as Game
    })

    it('drags with a mouse', async () => {
        renderWhitePawn()
        const image = screen.getByAltText('piece')

        pointerDown(image, 'mouse')
        const started = fireEvent.dragStart(image, { dataTransfer: dataTransfer() })

        expect(started).toBe(true)
        await waitFor(() => expect(screen.queryByAltText('piece')).not.toBe(image))
    })

    it('does not drag with a finger', async () => {
        renderWhitePawn()
        const image = screen.getByAltText('piece')

        pointerDown(image, 'touch')
        const started = fireEvent.dragStart(image, { dataTransfer: dataTransfer() })

        expect(started).toBe(false)
        await new Promise(resolve => setTimeout(resolve, 0))
        expect(screen.getByAltText('piece')).toBe(image)
    })

    it('decides by the press that starts each drag', async () => {
        renderWhitePawn()
        const image = screen.getByAltText('piece')

        pointerDown(image, 'touch')
        expect(fireEvent.dragStart(image, { dataTransfer: dataTransfer() })).toBe(false)

        pointerDown(image, 'mouse')
        expect(fireEvent.dragStart(image, { dataTransfer: dataTransfer() })).toBe(true)
    })
})
