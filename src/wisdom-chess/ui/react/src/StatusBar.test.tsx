import { describe, it, expect, beforeEach } from 'vitest'
import { render } from '@testing-library/react'
import StatusBar from './StatusBar'
import { getWisdomWindow, type WisdomChess } from './lib/WisdomChess'
import { wasmEnums } from './test/wasmEnums'

describe('StatusBar', () => {
    beforeEach(() => {
        getWisdomWindow().wisdomChessWeb = wasmEnums as unknown as WisdomChess
    })

    const renderStatusBar = (props: Partial<Parameters<typeof StatusBar>[0]> = {}) =>
        render(
            <StatusBar
                currentTurn={wasmEnums.White}
                inCheck={false}
                moveStatus=""
                gameOverStatus=""
                {...props}
            />
        )

    it('shows whose turn it is with the color in bold', () => {
        const { container } = renderStatusBar({ currentTurn: wasmEnums.Black })

        expect(container.querySelector('strong')?.textContent).toBe('Black')
        expect(container).toHaveTextContent('Black to move')
    })

    it('renders every bold part of the game over status', () => {
        const { container } = renderStatusBar({
            gameOverStatus: '<strong>Stalemate</strong> - No legal moves for <strong>White</strong>',
        })

        const bold = Array.from(container.querySelectorAll('strong')).map(e => e.textContent)
        expect(bold).toEqual(['Stalemate', 'White'])
        expect(container).toHaveTextContent('Stalemate - No legal moves for White')
        expect(container.textContent).not.toContain('<strong>')
    })

    it('adds check to the move status', () => {
        const { container } = renderStatusBar({ inCheck: true, moveStatus: 'Illegal move' })

        expect(container).toHaveTextContent('Illegal move - Check!')
    })

    it('does not report check once the game is over', () => {
        const { container } = renderStatusBar({
            inCheck: true,
            gameOverStatus: '<strong>Checkmate</strong> - Black wins the game.',
        })

        expect(container.textContent).not.toContain('Check!')
    })
})
