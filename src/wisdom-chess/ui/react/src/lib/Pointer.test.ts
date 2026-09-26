import { describe, it, expect, afterEach, vi } from 'vitest'
import { hasFinePointer } from './Pointer'

describe('hasFinePointer', () => {
    afterEach(() => {
        vi.unstubAllGlobals()
    })

    it('is true when the primary pointer is a mouse or trackpad', () => {
        const matchMedia = vi.fn((query: string) => ({ matches: query === '(pointer: fine)' }))
        vi.stubGlobal('matchMedia', matchMedia)

        expect(hasFinePointer()).toBe(true)
        expect(matchMedia).toHaveBeenCalledWith('(pointer: fine)')
    })

    it('is false when the primary pointer is a finger', () => {
        vi.stubGlobal('matchMedia', vi.fn(() => ({ matches: false })))

        expect(hasFinePointer()).toBe(false)
    })

    it('assumes a mouse where the browser cannot say', () => {
        vi.stubGlobal('matchMedia', undefined)

        expect(hasFinePointer()).toBe(true)
    })
})
